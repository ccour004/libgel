/*MIT License

Copyright (c) 2017 Colin Courtney

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once

#include <vector>
#include <poly2tri/poly2tri.h>

#include <ft2build.h>
#include <iostream>
#include <fstream>
#include FT_FREETYPE_H
#include FT_STROKER_H

int leftCell(int x,int range,int width,int height){
    if(x - range < 0 || x % width - range < 0) return - 1;
    return x - range;
}

int rightCell(int x,int range,int width,int height){
    if(x + range >= width * height * range || x % width + range >= width) return - 1;
    return x + range;   
}

int upCell(int x,int range,int width,int height){
    if(x - width < 0) return -1;
    return x - width;
}

int downCell(int x,int range,int width,int height){
    if(x + width >= width * height * range) return -1;
    return x + width;
}

int upLeftCell(int x,int range,int width,int height){
    if(x - width < 0) return -1;
    if(x - range < 0 || x % width - range < 0) return - 1;
    return x - width - range;
}
int upRightCell(int x,int range,int width,int height){
    if(x - width < 0) return -1;
    if(x + range >= width * height * range || x % width + range >= width) return - 1;
    return x - width + range;
}
int downLeftCell(int x,int range,int width,int height){
    if(x + width >= width * height * range) return -1;
    if(x - range < 0 || x % width - range < 0) return - 1;
    return x + width - range;
}
int downRightCell(int x,int range,int width,int height){
    if(x + width >= width * height * range) return -1;
    if(x + range >= width * height * range || x % width + range >= width) return - 1;
    return x + width + range;
}

std::vector<int> neighborCells(int x,int range,int width,int height){
    std::vector<int> indices;
    indices.push_back(rightCell(x,range,width,height));
    indices.push_back(upCell(x,range,width,height));
    indices.push_back(leftCell(x,range,width,height));
    indices.push_back(downCell(x,range,width,height));
    indices.push_back(upRightCell(x,range,width,height));
    indices.push_back(upLeftCell(x,range,width,height));
    indices.push_back(downLeftCell(x,range,width,height));
    indices.push_back(downRightCell(x,range,width,height));
    return indices;
}

bool isOutline(int x,int width,int rows,unsigned char* buffer){
    for(int neighbor:neighborCells(x,1,width,rows)){
        if(neighbor == -1 || buffer[neighbor] == 0) return true;
    }
    return false;
}

std::vector<glm::vec2> findOutlineVertices(int width,int height,unsigned char* buffer,std::vector<int>& remaining){
    std::vector<glm::vec2> vertices;
    int start = remaining[0/*remaining.size()-1*/];
    vertices.push_back(glm::vec2(start % width,start / width));
    //SDL_Log("START VALUE: %i",start);
    int me = start;
    remaining.erase(remaining.begin());
    //SDL_Log("VERTICES LEFT: %i",remaining.size());

    bool done = false;
    while(!done){
        std::vector<int> neighbors = neighborCells(me,1,width,height);
        bool foundCandidate = false;
        for(int i = 0;i < neighbors.size();i++){
            //SDL_Log("NEIGHBOR: %i",neighbors[i]);
            auto found = std::find(remaining.begin(),remaining.end(),neighbors[i]);
            if(found != remaining.end()){
                foundCandidate = true;
                //SDL_Log("NEXT VERTEX: %i",neighbors[i]);
                vertices.push_back(glm::vec2(neighbors[i] % width,neighbors[i] / width));
                me = neighbors[i];
                remaining.erase(found);
                i = neighbors.size();
                //SDL_Log("VERTICES LEFT: %i",remaining.size());
            }
        }
        if(!foundCandidate) done = true;
    }
    return vertices;
}

float freetype_test(wchar_t ch,FT_Library& library,std::string filename,
        std::vector<GLfloat>& line_vertices,std::vector<GLuint>& line_indices,std::vector<std::vector<GLfloat>>& holes,
        float scale,bool loop_remove){
    // Initialize FreeType.
    float advance = 0.0f;

    // Open up a font file.
    std::ifstream fontFile(filename.c_str(), std::ios::binary);
    if (fontFile)
    {
        // Read the entire file to a memory buffer.
        fontFile.seekg(0, std::ios::end);
        std::fstream::pos_type fontFileSize = fontFile.tellg();
        fontFile.seekg(0);
        unsigned char *fontBuffer = new unsigned char[fontFileSize];
        fontFile.read((char *)fontBuffer, fontFileSize);

        // Create a face for loading/filling glyph info.
        FT_Face face;
        FT_New_Memory_Face(library, fontBuffer, fontFileSize, 0, &face);

        int size = /*100*/1000;
        // Set the size to use.
        if (FT_Set_Char_Size(face, size << 6, size << 6, 90, 90) == 0)
        {
            // Load the glyph we are looking for (need an outline for this to work).
            FT_UInt gindex = FT_Get_Char_Index(face, ch);
            if (FT_Load_Glyph(face, gindex, FT_LOAD_NO_BITMAP) == 0 && face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                FT_Glyph glyph,border_glyph;           
                if (FT_Get_Glyph(face->glyph, &glyph) == 0 && glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    //Get glyph details, convert to outline.
                    advance = (float)glyph->advance.x * scale;
                    SDL_Log("Horiz adv: %f",advance);

                    //Render outline to bitmap.
                    FT_Glyph_To_Bitmap(&glyph,FT_RENDER_MODE_NORMAL,0,true);                    
                    FT_Bitmap *bitmap = &reinterpret_cast<FT_BitmapGlyph>(glyph)->bitmap;
                    SDL_Log("BITMAP INFO - pitch: %i,rows: %i,width: %i, pixel_mode: %i,num_grays = %i",
                        bitmap->pitch,bitmap->width,bitmap->rows,bitmap->pixel_mode,bitmap->num_grays);
                    std::vector<unsigned char> buffer;std::vector<int> remaining;
                    for(int i = 0;i < bitmap->width * bitmap->rows;i++){
                        //SDL_Log("PIXEL: %i",bitmap->buffer[i]);
                        if(bitmap->buffer[i] > 0 && isOutline(i,bitmap->width,bitmap->rows,bitmap->buffer))
                            {buffer.push_back(0x00);buffer.push_back(0x00);buffer.push_back(0x00);
                             remaining.push_back(i);
                            }
                        else {buffer.push_back(0xFF);buffer.push_back(0xFF);buffer.push_back(0xFF);}
                    }

                    /**TEMP**/
                    std::ofstream outfile;
                    std::vector<char> name; name.push_back((char)ch);
                    outfile.open("f.bin",std::ios::out | std::ios::binary | std::ios::trunc);
                    outfile.write((const char*)buffer.data(),buffer.size());
                    outfile.close();
                    /**TEMP**/

                    //Find outline vertices.
                    std::vector<glm::vec2> outline_vertices = findOutlineVertices(bitmap->width,bitmap->rows,bitmap->buffer,remaining);
                    //std::reverse(outline_vertices.begin(),outline_vertices.end());
                    std::vector<std::vector<glm::vec2>> temp_holes;
                    while(remaining.size() > 0) temp_holes.push_back(findOutlineVertices(bitmap->width,bitmap->rows,bitmap->buffer,remaining));
                                    
                    SDL_Log("Outline vertices done! Found %i vertices and %i holes.",outline_vertices.size(),temp_holes.size());
                    for(int i = 0;i < outline_vertices.size();i++){
                        line_vertices.push_back(outline_vertices[i].x * scale);
                        line_vertices.push_back(outline_vertices[i].y * scale);
                        line_vertices.push_back(-1.0f);
                        line_indices.push_back(line_indices.size());
                    }
                    for(std::vector<glm::vec2> hole:temp_holes){
                        SDL_Log("HOLE SIZE: %i",hole.size());
                        std::vector<GLfloat> hole_vertices;
                        /**TEMP**/
                        if(hole.size() >= 10){
                        /**TEMP**/
                        for(int i = 0;i < hole.size();i++){
                            hole_vertices.push_back(hole[i].x * scale);
                            hole_vertices.push_back(hole[i].y * scale);
                            hole_vertices.push_back(-1.0f);
                        }
                        holes.push_back(hole_vertices);
                        /**TEMP**/
                         }
                        /**TEMP**/
                    }
                }
            }
        }
        delete [] fontBuffer;       
    }
    return advance;
}

void triangulate(const std::vector<GLfloat>& incoming,const std::vector<std::vector<GLfloat>>& holes,
    std::vector<GLfloat>& outgoing,std::vector<GLuint>& outgoing_indices){
    //Convert points.
    std::vector<p2t::Point*> polyline; 
    SDL_Log("POLYLINE SIZE: %li",incoming.size()/3);
    if(incoming.size() == 0) return;

    //Add outline as polyline.
    std::vector<float> x,y;
    for(int i = 0;i < incoming.size();i +=3){
        //SDL_Log("POLYLINE POINT: (%f,%f)",incoming[i],incoming[i+1]);
        if(std::find(x.begin(),x.end(),incoming[i]) != x.end() &&
        std::find(y.begin(),y.end(),incoming[i+1]) != y.end()) {
            //SDL_Log("<<POINT EXISTS!>>");//return;
            x.push_back(incoming[i]);
            y.push_back(incoming[i+1]);
            polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));
        }else{
            x.push_back(incoming[i]);
            y.push_back(incoming[i+1]);
            polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));
        }
    }
    p2t::CDT* cdt = new p2t::CDT(polyline);

    //Add holes.
    for(std::vector<GLfloat> hole:holes){
        std::vector<p2t::Point*> holePoints;
        for(int i = 0;i < hole.size();i +=3){
            holePoints.push_back(new p2t::Point(hole[i],hole[i+1]));
        }
        cdt->AddHole(holePoints);       
    }

    //Triangulate.
    cdt->Triangulate();
    std::vector<p2t::Triangle*> triangles = cdt->GetTriangles();

    //Convert triangles to something we can return.
    for(int i = 0;i < triangles.size();i++){
        p2t::Triangle& t = *triangles[i];
        p2t::Point& a = *t.GetPoint(0);
        p2t::Point& b = *t.GetPoint(1);
        p2t::Point& c = *t.GetPoint(2);
        outgoing.push_back(a.x);outgoing.push_back(a.y);outgoing.push_back(-1.0f);
        outgoing.push_back(b.x);outgoing.push_back(b.y);outgoing.push_back(-1.0f);
        outgoing.push_back(c.x);outgoing.push_back(c.y);outgoing.push_back(-1.0f);
        //SDL_Log("GLYPH TRIANGLE: (%f,%f),(%f,%f),(%f,%f)",a.x,a.y,b.x,b.y,c.x,c.y);
        for(int j = 0;j < 3;j++)outgoing_indices.push_back(outgoing_indices.size());
    }

    //Clean up.
    delete cdt;
    polyline.clear();
}
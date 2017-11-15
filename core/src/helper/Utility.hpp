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
    if((x - range < 0) || (((x % width) - range) < 0)) return - 1;
    return x - range;
}

int rightCell(int x,int range,int width,int height){
    if((x + range >= width * height * range) || ((x % width) + range >= width)) return - 1;
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
    int left = x - range,right = (x % width) - range;
    if(x - width < 0) return -1;
    if(left < 0 || right < 0) return - 1;
    return x - width - range;
}
int upRightCell(int x,int range,int width,int height){
    if(x - width < 0) return -1;
    if(((x + range) >= (width * height * range)) || (((x % width) + range) >= width)) return - 1;
    return x - width + range;
}
int downLeftCell(int x,int range,int width,int height){
    if(x + width >= width * height * range) return -1;
    if((x - range < 0) || (((x % width) - range) < 0)) return - 1;
    return x + width - range;
}
int downRightCell(int x,int range,int width,int height){
    if(x + width >= width * height * range) return -1;
    if((x + range >= width * height * range) || (((x % width) + range) >= width)) return - 1;
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

struct GLYPH_LINE{
    GLYPH_LINE(glm::vec2 start,glm::vec2 end):start(start),end(end),slope(end-start){}
    glm::vec2 start,end,slope;
};
glm::vec2 line_intersect_params(GLYPH_LINE l1,GLYPH_LINE l2){
    glm::vec2 a = l1.start,b = l1.slope,c = l2.start,d = l2.slope;
    return glm::vec2((d.x * (a.y-c.y) +d.y * (c.x-a.x))/(b.x*d.y-b.y*d.x),
        (b.x * (c.y-a.y) +b.y * (a.x-c.x))/(d.x*b.y-d.y*b.x));  
}
bool pointInsidePolygon(glm::vec2 pt,const std::vector<glm::vec2>& polygon){
    GLYPH_LINE inf = GLYPH_LINE(pt,glm::vec2(pt.x + 9999,pt.y + 0.01f));
    int numIntersections = 0;
    for(int i = 0;i < polygon.size();i++){
        GLYPH_LINE edge = GLYPH_LINE(polygon[i],polygon[i+1 == polygon.size()?0:i+1]);
        glm::vec2 params = line_intersect_params(inf,edge);
        if(params.x >= 0.0f && params.x <= 1.0f && params.y >= 0.0f && params.y <= 1.0f)
            numIntersections++;
    }
    return (numIntersections % 2 == 0) ? false : true;
    //return doIntersect(pt,polygon);
}
bool polygonInsidePolygon(const std::vector<glm::vec2>& inner,const std::vector<glm::vec2>& outer){
    for(glm::vec2 pt:inner) if(pointInsidePolygon(pt,outer)) return true;
    return false;
}

struct OUTLINE{
    std::vector<glm::vec2> points;
    std::vector<std::vector<glm::vec2>> holes;
};
std::vector<glm::vec2> findOutlineVertices(int width,int height,unsigned char* buffer,std::vector<int>& remaining){
    std::vector<glm::vec2> vertices;
    int start = remaining[0/* remaining.size()-1*/];
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

void getVerticesAndIndices(std::vector<glm::vec2> shapes,float scale,std::vector<GLfloat>& vertices,std::vector<GLuint>& indices){
    for(glm::vec2 shape:shapes){
        vertices.push_back(shape.x * scale);
        vertices.push_back(shape.y * scale);
        vertices.push_back(-1.0f);
        indices.push_back(indices.size());
    }
}

std::vector<glm::vec2> getScaled(std::vector<glm::vec2> shapes,float scale){
    std::vector<glm::vec2> out;
    for(glm::vec2 shape:shapes){
        out.push_back(glm::vec2(shape.x * scale,shape.y * scale)); 
    }
    return out;
}

glm::vec2 freetype_test(wchar_t ch,FT_Library& library,std::string filename,std::vector<OUTLINE>& outlines){
    // Initialize FreeType.
    glm::vec2 advance = glm::vec2(0.0f,0.0f);

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

        int size = /*1000*/2000;
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
                    //advance = (float)glyph->advance.x * scale;
                    advance.x = (float)glyph->advance.x;
                    advance.y = (float)glyph->advance.y;
                    //SDL_Log("Horiz adv: %f",advance);

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
                    //Run: convert -depth 8 -size [width]x[height]+0 rgb:f.bin out.png
                    /**TEMP**/

                    //Find all shapes.
                    std::vector<std::vector<glm::vec2>> shapes;
                    while(remaining.size() > 0) shapes.push_back(findOutlineVertices(bitmap->width,bitmap->rows,bitmap->buffer,remaining));
                    SDL_Log("Found %li shapes.",shapes.size());

                    //Sort shapes into outlines or holes.
                    std::map<int,std::vector<std::vector<glm::vec2>>> holeMap;
                    std::map<int,int> outlineMap;
                    int holeCounter = 0;
                    for(int i = 0;i < shapes.size();i++){
                        OUTLINE outline;
                        SDL_Log("SHAPE %i has %li points.",i,shapes[i].size());

                        /**TEMP???**/
                        //Is this large enough to be considered a shape?
                        if(shapes[i].size() < 10) continue;
                        /**TEMP???**/

                        //Is this a hole?
                        bool hole = false;
                        for(int j = 0;j < shapes.size();j++)
                            if(!hole && i != j && polygonInsidePolygon(shapes[i],shapes[j])){
                                hole = true;
                                holeCounter++;
                                std::vector<std::vector<glm::vec2>> holes;
                                auto search = holeMap.find(j);
                                if(search != holeMap.end())
                                    holes = holeMap[j];
                                holes.push_back(shapes[i]);
                                holeMap[j] = holes;
                                j = shapes.size();
                            }

                        //Sort outlines vs. holes.
                        if(!hole){ 
                            outline.points = shapes[i];
                            outlines.push_back(outline);
                            outlineMap[i] = outlines.size()-1;
                        }
                    }
                    SDL_Log("%li are outlines, %i are holes.",outlines.size(),holeCounter);
                    
                    //Assign any holes to owning outlines.
                    for(int i = 0;i < shapes.size();i++){
                        auto search = holeMap.find(i);
                        if(search != holeMap.end()){
                            SDL_Log("SHAPE %i has a hole! Owning outline is at: %i position.",i,outlineMap[i]);
                            for(std::vector<glm::vec2> hole:search->second)
                            outlines[outlineMap[i]].holes.push_back(hole);
                        }
                    }
                }
            }
        }
        delete [] fontBuffer;       
    }
    return advance;
}

void triangulate(const std::vector<GLfloat>& incoming,const std::vector<std::vector<glm::vec2>>& holes,
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
    for(std::vector<glm::vec2> hole:holes){
        std::vector<p2t::Point*> holePoints;
        for(glm::vec2 pt:hole){
            holePoints.push_back(new p2t::Point(pt.x,pt.y));
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
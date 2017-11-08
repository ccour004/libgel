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
#include <glm/gtc/epsilon.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#define CUBIC_BEZIER_BEST 100
#define CUBIC_BEZIER_FAST 10

glm::vec2 cubic_bezier(glm::vec2 p0,glm::vec2 p1,glm::vec2 p2,glm::vec2 p3,float t){
    float poly = 1.0f - t,poly2 = pow(poly,2.0f),poly3 = pow(poly,3.0f),t2 = pow(t,2.0f),t3 = pow(t,3.0f);
    float a = poly3,b = 3.0f * poly2 * t,c = 3.0f * poly * t2,d = t3;
    glm::vec2 part1 = (a * p0) + (b * p1),part2 = (c * p2),part3 = (d * p3);
    return part1 + part2 + part3;
}

glm::vec2 quadratic_bezier(glm::vec2 p0,glm::vec2 p1,glm::vec2 p2,float t){
    float poly = 1.0f - t,poly2 = pow(poly,2.0f),t2 = pow(t,2.0f);
    glm::vec2 part1 = p0 * poly2,part2 = 2 * poly * t * p1,part3 = t2 * p2;
    return part1 + part2 + part3;
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

glm::vec2 pt;
std::vector<GLYPH_LINE> lines;

int move_to (const FT_Vector* to,void* user){
    pt = glm::vec2(to->x,to->y);
    //SDL_Log("MOVE TO: %ld,%ld",to->x,to->y);
    return 0;
}

int line_to(const FT_Vector* to,void* user)
{
    lines.push_back(GLYPH_LINE(glm::vec2(pt.x,pt.y),
        glm::vec2(to->x ,to->y)));
    pt = glm::vec2(to->x,to->y);
   //SDL_Log("LINE TO: %ld,%ld",to->x,to->y);
    return 0;
}

int conic_to(const FT_Vector* control,const FT_Vector* to,void* user){
    glm::vec2 p0 = glm::vec2(pt.x,pt.y),p1 = glm::vec2(control->x,control->y),p2 = glm::vec2(to->x ,to->y),lastPt = p0;
    float numPoints = CUBIC_BEZIER_BEST,t_increment = 1.0f / numPoints;
    for(float t = 0.0f;t <= 1.0f;t += t_increment){
        glm::vec2 pt = quadratic_bezier(p0,p1,p2,t);
        lines.push_back(GLYPH_LINE(lastPt,pt));
        lastPt = pt;
    }
    pt = glm::vec2(to->x,to->y);
    //SDL_Log("CONIC TO: %ld,%ld, with CONTROL1: %ld,%ld",to->x,to->y,control->x,control->y);
    return 0;
}

int cubic_to(const FT_Vector* control1,const FT_Vector* control2,const FT_Vector* to,void* user){
    glm::vec2 p0 = glm::vec2(pt.x,pt.y),p1 = glm::vec2(control1->x,control1->y),
        p2 = glm::vec2(control2->x,control2->y),p3 = glm::vec2(to->x ,to->y),lastPt = p0;
    float numPoints = CUBIC_BEZIER_BEST,t_increment = 1.0f / numPoints;
    for(float t = 0.0f;t < 1.0f;t += t_increment){
        glm::vec2 pt = cubic_bezier(p0,p1,p2,p3,t);
        lines.push_back(GLYPH_LINE(lastPt,pt));
        lastPt = pt;
    }
    pt = glm::vec2(to->x,to->y);
    //SDL_Log("CUBIC TO: %ld,%ld, with CONTROL1: %ld,%ld and CONTROL 2: %ld,%ld",to->x,to->y,control1->x,control1->y,control2->x,control2->y);
    return 0;
}

/*std::vector<std::vector<GLYPH_LINE>>*/void removeLoopsAndGetHoles(std::vector<GLYPH_LINE>& lines,bool fill_right){
    std::vector<std::vector<GLYPH_LINE>> holes;
    for(int i = 0;i < lines.size();i++)
    for(int j = 0;j < i;j++){
        glm::vec2 params = line_intersect_params(lines[i],lines[j]);
        if(params.x > 0.0f && params.x < 1.0f && params.y > 0.0f && params.y < 1.0f){
            glm::vec2 a = lines[i].start + params.x * lines[i].slope,b = lines[j].end,c = lines[i].start,
                d = b - a,e = c - a;
            //CW winding result for fill_right, CCW otherwise, will indicate the presence of a loop.
            float winding_result = glm::cross(glm::vec3(d.x,d.y,0),glm::vec3(e.x,e.y,0)).z;
            if((winding_result > 0.0f && !fill_right) || (winding_result < 0.0f && fill_right)){
                lines[i].end = a;
                lines[j].start = a;
                lines.erase(lines.begin()+j+1,lines.begin()+i);
            }else{ //If it isn't a loop, then it's a hole.
                //TODO: Remove hole, but add its points to the holes vector for triangulation later.
            }
        }
    }
    //return holes;
}

std::vector<GLYPH_LINE> getScanlines(std::vector<GLYPH_LINE>& lines,bool fill_right){
    std::vector<GLYPH_LINE> scanlines;
    float increment = 0.1f/*0.5f*/,scanline_length = 999.0f;

    for(int i = 0;i < lines.size();i++)
    for(float t = 0.0f;t <= 1.0f;t += increment){
        glm::vec2 vec = lines[i].end - lines[i].start,
            ortho = fill_right ? glm::vec2(vec.y,-vec.x) : glm::vec2(-vec.y,vec.x);
        float smallestT = FLT_MAX;
        GLYPH_LINE ortho_line = GLYPH_LINE(lines[i].start + lines[i].slope * t,lines[i].start + lines[i].slope * t + ortho * scanline_length);
        for(int j = 0;j < lines.size();j++)
        if(i != j){
            glm::vec2 params = line_intersect_params(ortho_line,lines[j]);
            if(params.x >= 0.0f && params.x <= 1.0f && params.x < smallestT && params.y >= 0.0f && params.y <= 1.0f)
                smallestT = params.x;
        }
        if(smallestT < FLT_MAX) {
            GLYPH_LINE new_scanline = GLYPH_LINE(ortho_line.start,ortho_line.start + ortho_line.slope * smallestT);
            //Clip any potential stray lines.
            float subT = smallestT;
            for(int k = 0;k < lines.size();k++){
                glm::vec2 params = line_intersect_params(new_scanline,lines[k]);
                if(params.x >= 0.0f && params.x <= 1.0f && params.x < subT && params.y >= 0.0f && params.y <= 1.0f)
                subT = params.x;
            }
            scanlines.push_back(GLYPH_LINE(ortho_line.start,ortho_line.start + ortho_line.slope * subT));
        }
    }
    return scanlines;
}

float freetype_test(wchar_t ch,FT_Library& library,std::string filename,std::vector<GLfloat>& vertices,std::vector<GLuint>& indices,float scale,bool loop_remove){
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

        int size = 100;
        // Set the size to use.
        if (FT_Set_Char_Size(face, size << 6, size << 6, 90, 90) == 0)
        {
            // Load the glyph we are looking for (need an outline for this to work).
            FT_UInt gindex = FT_Get_Char_Index(face, ch);
            if (FT_Load_Glyph(face, gindex, FT_LOAD_NO_BITMAP) == 0 && face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
            {
                FT_Glyph glyph;           
                if (FT_Get_Glyph(face->glyph, &glyph) == 0 && glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                {
                    advance = (float)glyph->advance.x * scale;
                    SDL_Log("Horiz adv: %f",advance);
                    FT_Outline *o = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;

                    //Decompose outline.
                    FT_Outline_Funcs* outline_funcs = new FT_Outline_Funcs;
                    outline_funcs->move_to = move_to;
                    outline_funcs->line_to = line_to;
                    outline_funcs->conic_to = conic_to;
                    outline_funcs->cubic_to = cubic_to;
                    outline_funcs->shift = 0;outline_funcs->delta = 0;
                    FT_Outline_Decompose(o,outline_funcs,NULL);
                    delete outline_funcs;

                    //Orientation for filling.
                    FT_Orientation orientation = FT_Outline_Get_Orientation(o);
                    bool fill_right = true;
                    switch(orientation){
                        case FT_ORIENTATION_FILL_RIGHT:SDL_Log("FILL RIGHT!");break;
                        case FT_ORIENTATION_FILL_LEFT:SDL_Log("FILL LEFT!");fill_right = false; break;
                        case FT_ORIENTATION_NONE:SDL_Log("FILL UNKNOWN!");break;
                    }

                    //Scanlines
                    //removeLoopsAndGetHoles(lines,fill_right);
                    lines.erase(lines.begin());
                    SDL_Log("NUM LINES: %li",lines.size());
                    int counter = 0;
                    for(GLYPH_LINE line:getScanlines(lines,fill_right)/*lines*/){
                        vertices.push_back(line.start.x * scale);
                        vertices.push_back(line.start.y * -scale);
                        vertices.push_back(-1.0f);
                        indices.push_back(indices.size());
                        vertices.push_back(line.end.x  * scale);
                        vertices.push_back(line.end.y * -scale);
                        vertices.push_back(-1.0f);
                        indices.push_back(indices.size());
                        //SDL_Log("PUSH LINE: (%f,%f) to (%f,%f)",line.start.x * scale,line.start.y * -scale,
                        //    line.end.x * scale,line.end.y * -scale);
                        counter++;
                    }
                    SDL_Log("NUM SCAN LINES: %i",counter);
                    lines.clear();
                }
            }
        }
        delete [] fontBuffer;       
    }
    return advance;
}

void triangulate(const std::vector<GLfloat>& incoming,std::vector<GLfloat>& outgoing,std::vector<GLuint>& outgoing_indices){
    //Convert points.
    std::vector<p2t::Point*> polyline; 
    SDL_Log("POLYLINE SIZE: %li",incoming.size());
    if(incoming.size() == 0) return;

    std::vector<float> x,y;
    for(int i = 0;i < incoming.size();i +=3){
        //SDL_Log("POLYLINE POINT: (%f,%f)",incoming[i],incoming[i+1]);
        if(std::find(x.begin(),x.end(),incoming[i]) != x.end() &&
        std::find(y.begin(),y.end(),incoming[i+1]) != y.end()) {
            SDL_Log("<<POINT EXISTS!>>");//return;
            //x.push_back(incoming[i]);
            //y.push_back(incoming[i+1]);
            //polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));
        }else{
            x.push_back(incoming[i]);
            y.push_back(incoming[i+1]);
            polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));
        }
    }

    //Triangulate.
    p2t::CDT* cdt = new p2t::CDT(polyline);
    cdt->Triangulate();
    std::vector<p2t::Triangle*> triangles = cdt->GetTriangles();

    //TODO: convert triangles to something we can return
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
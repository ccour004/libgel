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

#define CUBIC_BEZIER_BEST 100
#define CUBIC_BEZIER_FAST 10

glm::vec2 cubic_bezier(glm::vec2 p0,glm::vec2 p1,glm::vec2 p2,glm::vec2 p3,float t){
    float poly = 1.0f - t,poly2 = pow(poly,2.0f),poly3 = pow(poly,3.0f),t2 = pow(t,2.0f),t3 = pow(t,3.0f);
    float a = poly3,b = 3.0f * poly2 * t,c = 3.0f * poly * t2,d = t3;
    glm::vec2 part1 = (a * p0) + (b * p1),part2 = (c * p2),part3 = (d * p3);
    return part1 + part2 + part3;
}
void cubic_bezier_range(glm::vec2 p0,glm::vec2 p1,glm::vec2 p2,glm::vec2 p3,std::vector<GLfloat>& vertices,std::vector<GLuint>& indices,float numPoints){
    float t_increment = 1.0f / numPoints;
    for(float t = 0.0f;t < 1.0f/**/-t_increment/**/;t += t_increment){
        glm::vec2 pt = cubic_bezier(p0,p1,p2,p3,t);
        vertices.push_back(pt.x);vertices.push_back(pt.y);vertices.push_back(-1.0f);
        indices.push_back(indices.size());
    }  
}
struct GLYPH_SHAPE{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;
    float px,py;
    void point(float px,float py){this->px = px;this->py = py;}
    void bezier(float c1x,float c1y,float c2x,float c2y,float endx,float endy){
        cubic_bezier_range(glm::vec2(px,py),glm::vec2(c1x,c1y),
            glm::vec2(c2x,c2y),glm::vec2(endx,endy),vertices,indices,CUBIC_BEZIER_BEST);
        px = endx; py = endy;
    }
    void clear(){vertices.clear();indices.clear();}
};
std::vector<GLYPH_SHAPE> parse_glyph(std::string data){
    std::stringstream sstream;
    sstream.str(data);
    std::string temp[6];
    std::vector<GLYPH_SHAPE> shapes;
    GLYPH_SHAPE shape;
    while(sstream){
        sstream >> temp[0];
        //Close line.
        if(temp[0][0] == 'z'){
            shapes.push_back(shape);
            shape.clear();
        }
        //Move To - starting point.
        if(temp[0][0] == 'M' || temp[0][1] == 'M'){
            sstream >> temp[1];
            shape.point(std::stof(temp[0].substr(temp[0].find("M")+1)),-std::stof(temp[1]));
        }
        //Cubic Bezier curve.
        else if(temp[0][0] == 'C'){
            sstream >> temp[1] >> temp[2] >> temp[3] >> temp[4] >> temp[5];
            shape.bezier(std::stof(temp[0].substr(temp[0].find("C")+1)),-std::stof(temp[1]),std::stof(temp[2]),
                -std::stof(temp[3]),std::stof(temp[4]),-std::stof(temp[5]));
        }
    }
    return shapes;
 }
void svgGlyphToTriangles(std::vector<GLfloat>& vertices,std::vector<GLuint>& indices){
    //TODO: take in a glyph, pull out points, bezier curves, etc.,triangulate the resulting polyline, and return triangles/indices
}

void triangulate(const std::vector<GLfloat>& incoming,std::vector<GLfloat>& outgoing,std::vector<GLuint>& outgoing_indices){
    //Convert points.
    std::vector<p2t::Point*> polyline; 
    SDL_Log("POLYLINE SIZE: %i",incoming.size());
    if(incoming.size() == 0) return;

    for(int i = 0;i < incoming.size();i +=3){
        SDL_Log("POLYLINE POINT: (%f,%f)",incoming[i],incoming[i+1]);
        polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));
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
        SDL_Log("GLYPH TRIANGLE: (%f,%f),(%f,%f),(%f,%f)",a.x,a.y,b.x,b.y,c.x,c.y);
        for(int j = 0;j < 3;j++)outgoing_indices.push_back(outgoing_indices.size());
    }

    //Clean up.
    delete cdt;
    polyline.clear();
}
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

class Utility{
public:
    static void svgGlyphToTriangles(){
        //TODO: take in a glyph, pull out points, bezier curves, etc.,triangulate the resulting polyline, and return triangies/indices
    }
    static glm::mat4 bezierMat;
    static glm::vec2 bezierCurve(const glm::mat4& points,float t){
        return glm::vec4(1,t,pow(t,2),pow(t,3)) * bezierMat * points;
    }
    static void bezierRange(std::vector<glm::vec2>& incoming,const glm::mat4& points,float numPoints){
        for(float i = 0;i < 1.0f;i += 1.0f/numPoints) incoming.push_back(bezierCurve(points,i));
    }
    /*static glm::vec2 bezierCurve(glm::vec2 p1,glm::vec2 p2,glm::vec2 p3,glm::vec2 p4,float t){
        return glm::vec2(
            pow(1.0f-t,3.0f*p1.x)+pow(3.0f * t * (1.0f - t),2 * p2.x)+pow(3.0f * t,2.0f * (1.0f - t) * p3.x)+pow(t,3 * p4.x),
            pow(1.0f-t,3.0f*p1.y)+pow(3.0f * t * (1.0f - t),2 * p2.y)+pow(3.0f * t,2.0f * (1.0f - t) * p3.y)+pow(t,3 * p4.y),
            //(1-t)^3(P1x) + 3t(1-t)^2(P2x) + 3t^2(1-t)(P3x) + t^3(P4x),
            //(1-t)^3(P1y) + 3t(1-t)^2(P2y) + 3t^2(1-t)(P3y) + t^3(P4y)
        
            //[1 t t^2 t^3] [1 0  0  0] [p1.x p1.y] = [px py]
            //              -3 3  0  0   p2.x p2.y
            //               3 -6 3  0   p3.x p3.y
            //              -1 3 -3  1   p4.x p4.y
        );
    }*/
    static void triangulate(const std::vector<GLfloat>& incoming,std::vector<GLfloat>& outgoing,std::vector<GLuint>& outgoing_indices){
        //Convert points.
        std::vector<p2t::Point*> polyline;
        for(int i = 0;i < incoming.size();i +=2) polyline.push_back(new p2t::Point(incoming[i],incoming[i+1]));

        //Triangulate.
        p2t::CDT* cdt = new p2t::CDT(polyline);
        cdt->Triangulate();
        vector<p2t::Triangle*> triangles = cdt->GetTriangles();

        //TODO: convert triangles to something we can return
        int counter = 0;
        for(int i = 0;i < triangles.size();i++){
            p2t::Triangle& t = *triangles[i];
            p2t::Point& a = *t.GetPoint(0);
            p2t::Point& b = *t.GetPoint(1);
            p2t::Point& c = *t.GetPoint(2);
            outgoing.push_back(a.x);outgoing.push_back(a.y);
            outgoing.push_back(b.x);outgoing.push_back(b.y);
            outgoing.push_back(c.x);outgoing.push_back(c.y);
            for(int j = 0;j < 3;j++)outgoing_indices.push_back(counter++);
        }

        //Clean up.
        delete cdt;
        p2t::FreeClear(polyline);
        //TODO: free triangles?
    }
}; glm::mat4 Utility::bezierMat = glm::mat4(1,0,0,0,-3,3,0,0,3,-6,3,0,-1,3,-3,1);
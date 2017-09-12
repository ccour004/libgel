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

namespace gel
{
    class ShapeBuilder
    {
    public:
        ShapeBuilder();

        static void buildSphere(std::vector<GLfloat>& vertexValues,std::vector<GLuint>& indices,float width, float height,float depth, int divisionsU,int divisionsV){
            buildSphere(vertexValues,indices,width,height,depth,divisionsU,divisionsV,0,360,0,180);
        }

        static void buildBox(std::vector<GLfloat>& vertices,std::vector<GLuint>& indices,float width, float height, float depth){
            //POS
            vertices = std::vector<GLfloat>{
                // front
                -0.5f * width, -0.5f * height,  0.5f * depth,
                0.5f * width, -0.5f * height,  0.5f * depth,
                0.5f * width,  0.5f * height,  0.5f * depth,
                -0.5f * width,  0.5f * height,  0.5f * depth,
                // back
                -0.5f * width, -0.5f * height, -0.5f * depth,
                0.5f * width, -0.5f * height, -0.5f * depth,
                0.5f * width,  0.5f * height, -0.5f * depth,
                -0.5f * width,  0.5f * height, -0.5f * depth
            };

            indices = std::vector<GLuint>{
                // front
                0, 1, 2,
                2, 3, 0,
                // top
                1, 5, 6,
                6, 2, 1,
                // back
                7, 6, 5,
                5, 4, 7,
                // bottom
                4, 0, 3,
                3, 7, 4,
                // left
                4, 5, 1,
                1, 0, 4,
                // right
                3, 2, 6,
                6, 7, 3
            };
        }

        static void buildSphere(std::vector<GLfloat>& vertexValues,std::vector<GLuint>& indices,float width, float height, float depth,
                            int divisionsU, int divisionsV, float angleUFrom, float angleUTo, float angleVFrom, float angleVTo) {
            float degreesToRadians = M_PI / 180.0f;
            float hw = width * 0.5f;
            float hh = height * 0.5f;
            float hd = depth * 0.5f;
            float auo = degreesToRadians * angleUFrom;
            float stepU = (degreesToRadians * (angleUTo - angleUFrom)) / divisionsU;
            float avo = degreesToRadians * angleVFrom;
            float stepV = (degreesToRadians * (angleVTo - angleVFrom)) / divisionsV;
            float us = 1.0f / divisionsU;
            float vs = 1.0f / divisionsV;
            float u = 0.0f;
            float v = 0.0f;
            float angleU = 0.0f;
            float angleV = 0.0f;

            int s = divisionsU + 3;
            std::vector<GLuint> tmpIndices(s);
            int tempOffset = 0;
            
            std::vector<glm::vec3> vertices;

            for (int iv = 0; iv <= divisionsV; iv++) {
                angleV = avo + stepV * iv;
                v = vs * iv;
                float t = sin(angleV);
                float h = cos(angleV) * hh;
                for (int iu = 0; iu <= divisionsU; iu++) {
                    angleU = auo + stepU * iu;
                    u = 1.0f - us * iu;
                    vertices.push_back(glm::vec3(cos(angleU) * hw * t, h, sin(angleU) * hd * t));
                    vertices.push_back(glm::vec3(u,v,0));
                    tmpIndices[tempOffset] = vertices.size() - 1;
                    int o = tempOffset + s;
                    if ((iv > 0) && (iu > 0))
                    {
                        indices.push_back(tmpIndices[tempOffset]);                 
                        indices.push_back(tmpIndices[(o - 1) % s]);                
                        indices.push_back(tmpIndices[(o - (divisionsU + 2)) % s]); 
                        indices.push_back(tmpIndices[(o - (divisionsU + 2)) % s]); 
                        indices.push_back(tmpIndices[(o - (divisionsU + 1)) % s]); 
                        indices.push_back(tmpIndices[tempOffset]);                 
                    }
                    tempOffset = (tempOffset + 1) % s;
                }
            }
            
            for(/*glm::vec3 vertex:vertices*/int i = 0;i < vertices.size();i+=2){
                /*vertexValues.push_back(vertex.x);
                vertexValues.push_back(vertex.y);
                vertexValues.push_back(vertex.z);*/
                vertexValues.push_back(vertices[i].x);
                vertexValues.push_back(vertices[i].y);
                vertexValues.push_back(vertices[i].z);
                vertexValues.push_back(vertices[i+1].x);
                vertexValues.push_back(vertices[i+1].y);
            }
        }
        ~ShapeBuilder();
    };
}

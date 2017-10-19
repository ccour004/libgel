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

#include "system/AssetSystem.hpp"
#include <Application.hpp>

class UIBuilder{
    static bool isBuilt;
    static gel::Asset<gel::VertexReference> uiVertex;
public:
    static void addText(std::string text,glm::vec2 pos,glm::vec2 dimen,gel::Asset<gel::FontReference>& font,gel::Asset<gel::ShaderProgram>& shader,AssetSystem& assets){
        //Build the backing vertex object if not previously built.
        if(!isBuilt)
        {
            //float dimen[0] = 150.0f,dimen[1] = 40.0f;
            std::vector<GLfloat> uiVerts = std::vector<GLfloat>{
                -0.5f * dimen[0], -0.5f * dimen[1],  -1.0f, 0,0,
                 0.5f * dimen[0], -0.5f * dimen[1],  -1.0f, 1,0,
                 0.5f * dimen[0],  0.5f * dimen[1],  -1.0f, 1,1,
                -0.5f * dimen[0],  0.5f * dimen[1],  -1.0f, 0,1};
            std::vector<GLuint> uiIndices = std::vector<GLuint>{0, 1, 2, 2, 3, 0};
            uiVertex = assets.load<gel::VertexReference,gel::Vertex>(
                std::vector<gel::VertexSpec>{gel::POSITION,gel::TEXTURE_0},uiVerts,uiIndices).assign(shader);
            isBuilt = true;
        }

        //Build the actual text object.
        assets.load<gel::Mesh>().assign(glm::vec2(pos[0]+RenderSystem::cam.width/2.0f,pos[1]+RenderSystem::cam.height/2.0f)).assign(glm::vec4(1.0f,0.0f,0.0f,0.5f))
            .assign(assets.load<gel::TextureReference,gel::Texture>(text).assign(font)).assign(uiVertex).assign(shader);
    }
};
bool UIBuilder::isBuilt = false;
gel::Asset<gel::VertexReference> UIBuilder::uiVertex;
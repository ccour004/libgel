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
#include <string>
#include <sstream>

class ShaderBuilder{
    const std::string MAIN_START = "void main(){",
          SHADER_START = "#version 300 es\n",SHADER_END = "}";
    bool hasPosition = true,hasColor = true;
public:
    std::string buildVertexShader(){
        std::stringstream shader;
        shader << SHADER_START;

        //Member variables.
        if(hasPosition) shader << "in vec3 a_position;\n";
        if(hasColor) shader << "in vec4 a_color;\n" << "out vec4 v_color;\n";

        //Main.
        shader << MAIN_START;
        if(hasColor) shader << "v_color = a_color;\n";
        if(hasPosition) shader << "gl_Position = u_projView * vec4(a_position,1);\n";

        shader << SHADER_END;
        return shader.str();
    }
};
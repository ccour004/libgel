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
#include <map>
#include <string>
#include <sstream>
#include <fstream>

namespace gel
{
    class Shader
    {
        GLuint program;
        struct ShaderVariable{
            GLint location;
            GLenum type;
            int size;
        };
        std::string status;
    public:
        std::map<std::string,ShaderVariable> uniforms,attributes;
        Shader(){}
        Shader(std::string sharedFilename,std::string prepend):Shader(sharedFilename+".vert",sharedFilename+".frag",prepend){}
        Shader(std::string vertFilename,std::string fragFilename,std::string prepend){
            //Convert files to source text.
            std::ostringstream vStream,fStream;
            std::string vs = fileToString(vertFilename),fs = fileToString(fragFilename);
            vStream << prepend << vs; fStream << prepend << fs;

            //Compile and link the vertex and fragment shaders.
            linkProgram(compileShaders(std::vector<GLenum>{GL_VERTEX_SHADER,GL_FRAGMENT_SHADER},
                std::vector<const char*>{vStream.str().c_str(),fStream.str().c_str()}));
            
            //Populate the internal list of uniforms and attributes.
            populateUniforms();
            populateAttributes();
        }
        ~Shader(){SDL_Log("SHADER destruct!");/*glDeleteProgram(program);*/}
        
        void begin(){/*SDL_Log("PROGRAM: %i",program);*/glUseProgram(program);}
        void end(){glUseProgram(0);}
        
        std::string fileToString(std::string filename){
            SDL_RWops* io = SDL_RWFromFile(filename.c_str(),"rb");
            if(io != NULL){
                char name[256];
                if(SDL_RWread(io,name,sizeof(name),1) > 0)
                    SDL_Log("SOURCE: %s",std::string(name).substr(0,std::string(name).rfind("}")+1).c_str());
                SDL_RWclose(io);
                return std::string(name).substr(0,std::string(name).rfind("}")+1);
            }
            return "";
        }
        
        std::string getStatus(){return status;}
        
        std::vector<GLuint> compileShaders(std::vector<GLenum> types,std::vector<const char*> sources){
            std::vector<GLuint> shaders;
            for(int i = 0;i < types.size();i++){
                const GLuint shader = glCreateShader(types[i]);
                if(shader) shaders.push_back(shader);
                else{
                    SDL_Log("Could not create shader!");
                    return shaders;
                }
                
                const GLint len = (GLint)strlen(sources[i]);
                GLint compiled = GL_FALSE;
                glShaderSource(shader,1,&sources[i],&len);
                glCompileShader(shader);
                glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
                
                if(!compiled){
                    int bufSize = 256;
                    GLchar log[bufSize];
                    glGetShaderInfoLog(shader,bufSize,0,log);
                    SDL_Log("COMPILATION ERROR: %s|||%s",log,sources[i]);
                }
            }
            return shaders;
        }
        
        void linkProgram(std::vector<GLuint> shaders){
            program = glCreateProgram();
            //SDL_Log("PROGRAM IS: %i",program);
            for(GLuint shader:shaders) glAttachShader(program,shader);
            glLinkProgram(program);
        }
        
        GLint checkExists(const std::string& name,const std::map<std::string,ShaderVariable>& varList){
            if(varList.size() > 0){
                auto search = varList.find(name);
                if(search != varList.end()){
                    //SDL_Log("SEARCH FOR %s gives: %i",name.c_str(),search->second.location);
                    return search->second.location;
                }
            }
            //SDL_Log("SEARCH FOR %s gives: -1",name.c_str());
            return -1;
        }
        
        void populateUniforms(){
            int bufSize = 256;
            GLchar names[bufSize];
            GLint count;
            glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
            for (int i = 0; i < count; i++) {
                ShaderVariable uniform;
                glGetActiveUniform(program, i,bufSize,0, &uniform.size,&uniform.type,names);
                uniform.location = glGetUniformLocation(program, names);
                SDL_Log("FOUND Uniform #%d Type: %u Name: %s\n", i, uniform.type,names);
                uniforms[std::string(names)] = uniform;
            }       
        }
        
        void populateAttributes(){
            int bufSize = 256;
            GLchar names[bufSize];
            GLint count;
            glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
            for (int i = 0; i < count; i++) {
                ShaderVariable attribute;
                glGetActiveAttrib(program, i,bufSize,0, &attribute.size,&attribute.type,names);
                attribute.location = glGetAttribLocation(program, names);
                SDL_Log("FOUND Attribute #%d Type: %u Name: %s\n", i, attribute.type, names);
                attributes[std::string(names)] = attribute;
            }       
        }
        
        GLint getAttribute(const std::string& name){
            return checkExists(name,attributes);
        }
        
        GLint getUniform(const std::string& name){
            return checkExists(name,uniforms);
        }
        
        bool enableVertexAttribPointer(const std::string& name){
            GLint location = checkExists(name,attributes);
            if(location == -1) return false;
            glEnableVertexAttribArray(location);
            return true;
        }
        
        bool disableVertexAttribPointer(const std::string& name){
            GLint location = checkExists(name,attributes);
            if(location == -1) return false;
            glDisableVertexAttribArray(location);
            return true;
        }
        
        bool setVertexAttribPointer(const std::string& name,GLint size, GLenum type,GLboolean normalized,GLsizei stride,const void* offset){
            GLint location = checkExists(name,attributes);
            if(location == -1) return false;
            glVertexAttribPointer(location,size,type,normalized,stride,offset);
            return true;
        }
        
        bool setUniform(const std::string& name,const std::vector<GLfloat>& values){
        GLint location = checkExists(name,uniforms);
        if(location == -1) return false;
        switch(values.size()){
            case 1: glUniform1f(location,values[0]);return true;
            case 2: glUniform2f(location,values[0],values[1]);return true;
            case 3: glUniform3f(location,values[0],values[1],values[2]);return true;
            case 4: glUniform4f(location,values[0],values[1],values[2],values[3]);return true;
        }
        return false;
        }
        
        bool setAttribute(const std::string& name,const std::vector<GLfloat>& values){
        GLint location = checkExists(name,attributes);
        if(location == -1) return false;
        switch(values.size()){
            case 1: glVertexAttrib1f(location,values[0]);return true;
            case 2: glVertexAttrib2f(location,values[0],values[1]);return true;
            case 3: glVertexAttrib3f(location,values[0],values[1],values[2]);return true;
            case 4: glVertexAttrib4f(location,values[0],values[1],values[2],values[3]);return true;
        }
        return false;
        }
        
        bool setUniform(const std::string& name,const std::vector<GLint>& values){
        GLint location = checkExists(name,uniforms);
        if(location == -1) return false;
        switch(values.size()){
            case 1: glUniform1i(location,values[0]);return true;
            case 2: glUniform2i(location,values[0],values[1]);return true;
            case 3: glUniform3i(location,values[0],values[1],values[2]);return true;
            case 4: glUniform4i(location,values[0],values[1],values[2],values[3]);return true;
        }
        return false;
        }
        
        bool setUniform(const std::string& name,const std::vector<GLuint>& values){
        GLint location = checkExists(name,uniforms);
        if(location == -1) return false;
        switch(values.size()){
            case 1: glUniform1ui(location,values[0]);return true;
            case 2: glUniform2ui(location,values[0],values[1]);return true;
            case 3: glUniform3ui(location,values[0],values[1],values[2]);return true;
            case 4: glUniform4ui(location,values[0],values[1],values[2],values[3]);return true;
        }
        return false;
        }
        
        bool setUniform(const std::string& name,const glm::mat2& value,GLboolean transpose){
        GLint location = checkExists(name,uniforms);
        if(location == -1) return false;
        glUniformMatrix2fv(location,1,transpose,(const float*)glm::value_ptr(value));
        return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat3& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix3fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat4& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix4fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat2x3& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix2x3fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat3x2& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix3x2fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat2x4& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix2x4fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat4x2& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix4x2fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat3x4& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix3x4fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
        
        bool setUniform(const std::string& name,const glm::mat4x3& value,GLboolean transpose){
            GLint location = checkExists(name,uniforms);
            if(location == -1) return false;
            glUniformMatrix4x3fv(location,1,transpose,(const float*)glm::value_ptr(value));
            return true;
        }
    };
}


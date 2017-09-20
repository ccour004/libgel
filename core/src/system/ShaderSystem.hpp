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

#include <entityx/entityx.h>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

namespace gel{
    struct ShaderHandle{
        entityx::Entity ent;
        ShaderHandle(entityx::Entity& ent):ent(ent){}
    };

    struct ShaderVariable{
        GLint location;
        GLenum type;
        int size;
    };

    struct ShaderSource{
        std::string fileToString(std::string filename){
            SDL_RWops* io = SDL_RWFromFile(filename.c_str(),"r");
            if(io != NULL){
                char name[256];
                if(SDL_RWread(io,name,sizeof(name),1) > 0)
                    SDL_Log("SOURCE: %s",std::string(name).substr(0,std::string(name).rfind("}")+1).c_str());
                SDL_RWclose(io);
                return std::string(name).substr(0,std::string(name).rfind("}")+1);
            }
            return "";
        }
        std::string sanitize(std::string input){
            std::string str = input;
            bool openBracket = false;
            for(int i = 0;i < str.length();i++){
                if(str[i] == '{' && openBracket) str.erase(str.begin()+i);
                else if(str[i] == '}' && !openBracket) str.erase(str.begin()+i);
                
                if(str[i] == '}') openBracket = false;
                else if(str[i] == '{') openBracket = true;
            }
            SDL_Log("SANITIZED: %s",str.c_str());
            return str;
        }
    public:
        GLenum type;
        std::string contents;
        ShaderSource(){}
        ShaderSource(std::string filename,GLenum type,std::string prepend){
            this->type = type;
            std::ostringstream stream;
            stream << prepend << fileToString(filename);
            contents = sanitize(stream.str());
        }
    };

    struct ShaderSpec{
        std::string name;
        std::vector<ShaderSource> sources;
        ShaderSpec(){}
        ShaderSpec(std::string name,std::vector<ShaderSource> sources):name(name),sources(sources){}
    };

    struct ShaderProgram{
        std::string name;
        GLuint program;
        std::map<std::string,ShaderVariable> uniforms,attributes;

        ShaderProgram(){}
        ShaderProgram(std::string name,GLuint program):name(name),program(program){}

        void begin(){glUseProgram(program);}
        void end(){glUseProgram(0);}

        GLint checkExists(const std::string& name,const std::map<std::string,gel::ShaderVariable>& varList){
            if(varList.size() > 0){
                auto search = varList.find(name);
                if(search != varList.end()){
                    return search->second.location;
                }
            }
            return -1;
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
    class ShaderSystem: public entityx::System<ShaderSystem>,public entityx::Receiver<ShaderSystem>{ 
    public:
        static std::map<std::string,entityx::ComponentHandle<gel::ShaderProgram>> programs;
        void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
            //Compile and link a shader.
            entities.each<gel::ShaderSpec>([](entityx::Entity entity,gel::ShaderSpec& spec) {
                gel::ShaderProgram program;
                program.program = glCreateProgram();

                //Compile
                for(gel::ShaderSource source:spec.sources){
                    const GLuint shader = glCreateShader(source.type);
                    if(!shader) SDL_Log("Could not create shader!");
                    else{     
                        //SDL_Log("CODE: %s",source.contents.c_str()); 
                        const char* contents_c_str = source.contents.c_str();
                        const GLint len = (GLint)strlen(contents_c_str);
                        GLint compiled = GL_FALSE;
                        glShaderSource(shader,1,&contents_c_str,&len);
                        glCompileShader(shader);
                        glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);  
                        if(!compiled){
                            int bufSize = 256;
                            GLchar log[bufSize];
                            glGetShaderInfoLog(shader,bufSize,0,log);
                            SDL_Log("COMPILATION ERROR: %s ||| %s",log,contents_c_str);
                        }else glAttachShader(program.program,shader);
                        //delete contents_c_str;
                    }
                }

                //Link             
                glLinkProgram(program.program);

                //Populate all variables.
                std::vector<GLenum> variableTypes = std::vector<GLenum>{GL_ACTIVE_UNIFORMS,GL_ACTIVE_ATTRIBUTES};
                for(GLenum type:variableTypes){
                    int bufSize = 256;
                    GLchar names[bufSize];
                    GLint count;
                    glGetProgramiv(program.program, type, &count);
                    for (int i = 0; i < count; i++) {
                        gel::ShaderVariable variable;
                        if(type == GL_ACTIVE_UNIFORMS){
                            glGetActiveUniform(program.program, i,bufSize,0, &variable.size,&variable.type,names);
                            variable.location = glGetUniformLocation(program.program, names);
                            SDL_Log("FOUND Uniform #%d Type: %u Name: %s\n", i, variable.type,names);
                            program.uniforms[std::string(names)] = variable;
                        }else if(type == GL_ACTIVE_ATTRIBUTES){
                            glGetActiveAttrib(program.program, i,bufSize,0, &variable.size,&variable.type,names);
                            variable.location = glGetAttribLocation(program.program, names);
                            SDL_Log("FOUND Attribute #%d Type: %u Name: %s\n", i, variable.type,names);
                            program.attributes[std::string(names)] = variable;                            
                        }
                    }
                } 
                
                //Finish up.
                entity.assign<gel::ShaderProgram>(program);
                entity.component<gel::ShaderSpec>().remove();
            });
        }
        void configure(entityx::EventManager& events) override{
            events.subscribe<entityx::ComponentRemovedEvent<gel::ShaderProgram>>(*this);
        }
        void receive(const entityx::ComponentRemovedEvent<gel::ShaderProgram>& event){
            SDL_Log("Removing shader...");
            glDeleteProgram((*event.component.get()).program);
        }
    };
    std::map<std::string,entityx::ComponentHandle<gel::ShaderProgram>> ShaderSystem::programs;
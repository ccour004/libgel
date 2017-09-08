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
#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

#include "Common.hpp"
#include <vector>

namespace gel
{
    struct VertexDescriptor{
        VertexDescriptor(GLenum type,GLint size,GLint location){
            this->type = type;
            this->size = size;
            this->location = location;
        }
        GLenum type;
        GLint size,location;
    };

    struct VertexBufferObject{
    private:
        GLuint vbo_handle;
        std::vector<VertexDescriptor> descriptors;
        std::vector<GLfloat> data;
    public:   
        VertexBufferObject(){}
        VertexBufferObject& operator=(const VertexBufferObject& other){
            this->descriptors = other.descriptors;
            this->data = other.data;
            return *this;
        }
        VertexBufferObject(const VertexBufferObject& other){
            this->descriptors = other.descriptors;
            this->data = other.data;       
        }
        VertexBufferObject(const std::vector<VertexDescriptor>& descriptors,const std::vector<GLfloat>& data){
            this->descriptors = descriptors;
            this->data = data;
            glGenBuffers(descriptors.size(),&vbo_handle);
            
            /*TEMP*/
            int total_size = 0;
            for(int i = 0;i < descriptors.size();i++) total_size += sizeof(descriptors[i].type) * descriptors[i].size;
            /*TEMP*/
            
            int offset = 0;
            glBindBuffer(GL_ARRAY_BUFFER,vbo_handle);
            glBufferData(GL_ARRAY_BUFFER,data.size() * sizeof(GLfloat),data.data(),GL_STATIC_DRAW);

            for(int i = 0;i < descriptors.size();i++){
                glEnableVertexAttribArray(descriptors[i].location);
                glVertexAttribPointer(descriptors[i].location,
                    descriptors[i].size,descriptors[i].type,GL_FALSE,total_size/*0*/,BUFFER_OFFSET(offset));
                offset += sizeof(descriptors[i].type) * descriptors[i].size;
            }
        }
        ~VertexBufferObject(){SDL_Log("VBO DESTRUCT!");/*glDeleteBuffers(descriptors.size(),&vbo_handle);*/}
    };

    struct IndexBufferObject{
    GLuint ibo_handle;
    std::vector<GLuint> data;
    
    IndexBufferObject(){}
    IndexBufferObject& operator=(const IndexBufferObject& other){
        this->data = other.data;
        return *this;
    }
    IndexBufferObject(const IndexBufferObject& other){
        this->data = other.data;       
    }
    IndexBufferObject(const std::vector<GLuint>& data){
        this->data = data;
        glGenBuffers(1,&ibo_handle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(GLuint), data.data(), GL_STATIC_DRAW);
    }
    ~IndexBufferObject(){SDL_Log("IBO DESTRUCT!");/*glDeleteBuffers(1,&ibo_handle);*/}
    };

    class Vertices
    {
        GLuint vao;
        VertexBufferObject vbo;
        IndexBufferObject ibo;
    public:
        Vertices& operator=(const Vertices& other){
            copy(other);
            return *this;
        }
        Vertices(const Vertices& other){copy(other);}
        Vertices(){};
        Vertices(const std::vector<VertexDescriptor>& descriptors,std::vector<GLfloat> vertices,
                    std::vector<GLuint> indices){        
            //Generate vao and populate it.
            glGenVertexArrays(1,&vao);  SDL_Log("VAO: %i",vao);
            glBindVertexArray(vao); 

            //Generate the vbos and an ibo.
            vbo = VertexBufferObject(descriptors,vertices);
            ibo = IndexBufferObject(indices);
    
            //Reset state.
            //glBindVertexArray(0);
        }
        
        void copy(const Vertices& other){
            this->vao = other.vao;
            this->vbo = other.vbo;
            this->ibo = other.ibo;
        }
        
        void draw(GLenum mode){
        glBindVertexArray(vao);
        //SDL_Log("VAO: %i",vao);
        glDrawElements(mode,ibo.data.size(),GL_UNSIGNED_INT,0);
        //Reset state.
        //glBindVertexArray(0);
        }
        
        ~Vertices(){
            SDL_Log("VAO DESTRUCT!");
            //glDeleteVertexArrays(1,&vao);
        }
    };
}

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
#include <entityx/entityx.h>

namespace gel
{
    struct VertexHandle{
        entityx::Entity ent;
       VertexHandle(entityx::Entity& ent):ent(ent){}
    };

    struct VertexSpec{
        GLenum type;
        GLint size;
        std::string name;
        VertexSpec(GLenum type,GLint size,std::string name):type(type),size(size),name(name){}
    };

    struct Vertex{
        std::vector<VertexSpec> descriptors;
        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;
        Vertex(std::vector<VertexSpec> descriptors,std::vector<GLfloat> vertices,std::vector<GLuint> indices):
            descriptors(descriptors),vertices(vertices),indices(indices){}       
    };

    struct VertexReference
    {
        GLuint vao,vbo,ibo;
        int ibo_size,vbo_size;
    };
}

class VertexSystem: public entityx::System<VertexSystem>,public entityx::Receiver<VertexSystem>{ 
public:
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        entities.each<gel::ShaderHandle,gel::Vertex>([](entityx::Entity entity,gel::ShaderHandle& shaderHandle,gel::Vertex& vertex) {
            gel::VertexReference ref;
            entityx::ComponentHandle<gel::ShaderProgram> shader = shaderHandle.ent.component<gel::ShaderProgram>();

            if(shader){
                ref.ibo_size = vertex.indices.size();
                ref.vbo_size = vertex.descriptors.size();

                //Generate VAO (Vertex Array Object).
                glGenVertexArrays(1,&ref.vao);  //SDL_Log("VAO: %i",ref.vao);
                glBindVertexArray(ref.vao); 

                //Load VBO (Vertex Buffer Object).
                glGenBuffers(vertex.descriptors.size(),&ref.vbo);
                
                /*TEMP*/
                int total_size = 0;
                for(int i = 0;i < vertex.descriptors.size();i++) total_size += sizeof(vertex.descriptors[i].type) * vertex.descriptors[i].size;
                /*TEMP*/
                
                int offset = 0;
                glBindBuffer(GL_ARRAY_BUFFER,ref.vbo);
                glBufferData(GL_ARRAY_BUFFER,vertex.vertices.size() * sizeof(GLfloat),vertex.vertices.data(),GL_STATIC_DRAW);

                for(int i = 0;i < vertex.descriptors.size();i++){
                    glEnableVertexAttribArray(shader->getAttribute(vertex.descriptors[i].name));
                    SDL_Log("FOR ATTRIBUTE %s value is %i",vertex.descriptors[i].name.c_str(),shader->getAttribute(vertex.descriptors[i].name));
                    glVertexAttribPointer(shader->getAttribute(vertex.descriptors[i].name),
                        vertex.descriptors[i].size,vertex.descriptors[i].type,GL_FALSE,total_size/*0*/,BUFFER_OFFSET(offset));
                    offset += sizeof(vertex.descriptors[i].type) * vertex.descriptors[i].size;
                }

                //Load IBO (Index Buffer Object).
                glGenBuffers(1,&ref.ibo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ref.ibo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex.indices.size() * sizeof(GLuint), vertex.indices.data(), GL_STATIC_DRAW);

                //Reset state.
                //glBindVertexArray(0);

                entity.component<gel::Vertex>().remove();
                entity.assign<gel::VertexReference>(ref);
            }
        });
    }
    void configure(entityx::EventManager& events) override{
        events.subscribe<entityx::ComponentRemovedEvent<gel::VertexReference>>(*this);
    }
    void receive(const entityx::ComponentRemovedEvent<gel::VertexReference>& event){
        SDL_Log("Removing vertex...");
        glDeleteBuffers((*event.component.get()).vbo_size,&(*event.component.get()).vbo);
        glDeleteBuffers(1,&(*event.component.get()).ibo);
        glDeleteVertexArrays(1,&(*event.component.get()).vao);
    }
};
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
#include "../Common.hpp"
#include "../Camera.hpp"
#include "ShaderSystem.hpp"
#include "TextureSystem.hpp"
#include "PhysicsSystem.hpp"
#include "VertexSystem.hpp"
#include <glm/gtx/rotate_vector.hpp>

namespace gel{
    struct Mesh{
        std::string name = "<NULL>";
        glm::mat4 transform = glm::mat4();
        Mesh(std::string name,glm::mat4 transform):name(name),transform(transform){}
        //TODO: fill this out!!
    };
}

class RenderSystem: public entityx::System<RenderSystem>{
public:
    static gel::Camera cam;
    static int x;
    static float rotateAmount,rotateSpeed;
    static bool drag;
    static glm::vec3 eye;
    static btTransform trans;
    static GLuint lastVAO;

    static int rotCounter;

    RenderSystem(){
        cam = gel::Camera(glm::radians(90.0f),640.0f,480.0f,0.1f,1000.0f);
        cam.lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,1,0));
    }
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

        glViewport(0,0,cam.width,cam.height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        eye = glm::rotateY(eye,(float)(dt * rotateAmount * rotateSpeed));
        cam.lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,1,0));

        //3D models
        entities.each<gel::Asset<gel::ShaderProgram>,gel::Asset<gel::VertexReference>,glm::vec4,glm::mat4>([](entityx::Entity entity,
            gel::Asset<gel::ShaderProgram>& shaderHandle,gel::Asset<gel::VertexReference>& vertexHandle,glm::vec4& color,glm::mat4& model) {
            entityx::ComponentHandle<RigidBody> body = entity.component<RigidBody>();
            entityx::ComponentHandle<gel::ShaderProgram> shader = shaderHandle.component<gel::ShaderProgram>();
            entityx::ComponentHandle<gel::VertexReference> vertex = vertexHandle.component<gel::VertexReference>();
            entityx::ComponentHandle<gel::TextureReference> texture;
            if(entity.component<gel::Asset<gel::TextureReference>>()){
                gel::Asset<gel::TextureReference> ref = entity.component<gel::Asset<gel::TextureReference>>()->get();
                if(ref.component<gel::TextureReference>())
                    texture = ref.component<gel::TextureReference>();
                }

            if(body && body->mass != 0.0f){
                body->motionState->getWorldTransform(trans);
                model = glm::mat4();
                for(int i = 0;i < 4;i++)
                for(int j = 0;j < 4;j++)
                    model[i][j] = float(trans.getBasis()[i][j]);
                model[3][3] = 1.0f;
            }
                
            if(entity.component<gel::Mesh>()){
                glm::mat4 reference = glm::rotate(-1.570797f,glm::vec3(1,0,0));
                std::string strfind = entity.component<gel::Mesh>()->name;
                glm::mat4 local_transform = entity.component<gel::Mesh>()->transform;
                entityx::ComponentHandle<glm::mat4> transform = entity.component<glm::mat4>();
                if(strfind.find("blade") != std::string::npos){
                    glm::mat4 rotate = glm::rotate(0.01f * (rotCounter++),glm::vec3(0,0,1));
                    *transform = reference * local_transform * rotate;
                }else{
                    model = model * reference * local_transform;
                }
                // SDL_Log("MESH NAME: %s",mesh.component<gel::Mesh>()->name.c_str());
            }

            if(shader){
                shader->begin();
                shader->setAttribute("a_color",std::vector<GLfloat>{color.r,color.g,color.b,color.a});
                shader->setUniform("u_projView",cam.getCombined() * model,false);
            }
            
            if(texture) {/*SDL_Log("TEXTURE: %i",texture->tex);*/glBindTexture(GL_TEXTURE_2D,texture->tex);}
            if(vertex){
                if(lastVAO != vertex->vao){
                    lastVAO = vertex->vao;
                    glBindVertexArray(vertex->vao);
                }
                glDrawElements(GL_TRIANGLES,vertex->ibo_size,GL_UNSIGNED_INT,0);
            }
            if(shader) shader->end();
         });

         //2D elements
         entities.each<gel::Asset<gel::ShaderProgram>,/*gel::Asset<gel::TextureReference>,*/gel::Asset<gel::VertexReference>,glm::vec2,glm::vec4>([](entityx::Entity entity,
            gel::Asset<gel::ShaderProgram>& shaderHandle/*,gel::Asset<gel::TextureReference>& texHandle*/,gel::Asset<gel::VertexReference>& vertexHandle,glm::vec2& pos,glm::vec4& color) {
            entityx::ComponentHandle<gel::ShaderProgram> shader = shaderHandle.component<gel::ShaderProgram>();
            entityx::ComponentHandle<gel::VertexReference> vertex = vertexHandle.component<gel::VertexReference>();
            entityx::ComponentHandle<float> timeRef = entity.component<float>();
            //entityx::ComponentHandle<gel::TextureReference> texture = texHandle.component<gel::TextureReference>(); 

            if(shader){
                shader->begin();
                shader->setAttribute("a_color",std::vector<GLfloat>{color.r,color.g,color.b,color.a});
                if(timeRef) shader->setAttribute("time",std::vector<GLfloat>{*timeRef});
                shader->setUniform("u_projView",/*cam.getOrtho() * glm::translate(glm::mat4(1.0f),glm::vec3(pos.x,pos.y,0.0f))*/glm::ortho(0.0f,1.0f,1.0f,0.0f,0.1f,100.0f) * glm::translate(glm::vec3(0,0,-1)),false);
            }

            //if(texture) glBindTexture(GL_TEXTURE_2D,texture->tex);
            if(vertex){
                if(lastVAO != vertex->vao){
                    lastVAO = vertex->vao;
                    glBindVertexArray(vertex->vao);
                }
                //glDrawElements(GL_TRIANGLES,vertex->ibo_size,GL_UNSIGNED_INT,0);
                glDrawArrays(GL_TRIANGLES,0,3);
            }
            if(shader) shader->end();
         });

         //Polylines
         entities.each<gel::Asset<gel::ShaderProgram>,gel::Asset<gel::VertexReference>,int,glm::vec2,glm::vec4>([](entityx::Entity entity,
            gel::Asset<gel::ShaderProgram>& shaderHandle,gel::Asset<gel::VertexReference>& vertexHandle,int numPoints,glm::vec2& pos,glm::vec4& color) {
            entityx::ComponentHandle<gel::ShaderProgram> shader = shaderHandle.component<gel::ShaderProgram>();
            entityx::ComponentHandle<gel::VertexReference> vertex = vertexHandle.component<gel::VertexReference>();

            if(shader){
                shader->begin();
                shader->setAttribute("a_color",std::vector<GLfloat>{color.r,color.g,color.b,color.a});
                shader->setUniform("u_projView",cam.getOrtho() * glm::translate(glm::mat4(1.0f),glm::vec3(pos.x,pos.y,0.0f)),false);
            }

            if(vertex){
                if(lastVAO != vertex->vao){
                    lastVAO = vertex->vao;
                    glBindVertexArray(vertex->vao);
                }
                glDrawElements(/*GL_LINES*/GL_LINE_LOOP,numPoints,GL_UNSIGNED_INT,0);
            }
            if(shader) shader->end();
         });         

         //SDL_Log("RENDER TIME: %f",(float)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now).count());
    }
};

gel::Camera RenderSystem::cam;
int RenderSystem::x = 30;
float RenderSystem::rotateAmount = 0.0f;
float RenderSystem::rotateSpeed = 0.05f * 0.001f;
bool RenderSystem::drag = false;
btTransform RenderSystem::trans;
glm::vec3 RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(glm::vec3(50,10,50));
GLuint RenderSystem::lastVAO = -1;
int RenderSystem::rotCounter = 0;
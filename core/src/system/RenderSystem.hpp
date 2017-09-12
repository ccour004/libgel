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
#include "../Vertices.hpp"
#include "../Shader.hpp"
#include "ResourceSystem.hpp"
#include "PhysicsSystem.hpp"
#include <glm/gtx/rotate_vector.hpp>

class RenderSystem: public entityx::System<RenderSystem>{
public:
    static gel::Camera cam;
    static int x;
    static float rotateAmount,rotateSpeed;
    static bool drag;
    static glm::vec3 eye;

    RenderSystem(){
        cam = gel::Camera(glm::radians(90.0f),640.0f/480.0f,0.1f,100.0f);
        cam.lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,1,0));
    }
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        eye = glm::rotateY(eye,(float)(dt * rotateAmount * rotateSpeed));
        cam.lookAt(eye,glm::vec3(0,0,0),glm::vec3(0,1,0));
        entities.each<gel::Vertices,gel::Shader,glm::vec3,glm::vec4>
            ([](entityx::Entity entity, gel::Vertices& vertices,gel::Shader& shader,glm::vec3& pos,glm::vec4& color) {
            shader.begin();
            glm::mat4 model = glm::translate(glm::mat4(1.0f),pos);

            entityx::ComponentHandle<RigidBody> body = entity.component<RigidBody>();
            entityx::ComponentHandle<entityx::Entity> textureHandle = entity.component<entityx::Entity>();

            if(body){
                btTransform trans;
                if (body->body && body->body->getMotionState()){
                    body->body->getMotionState()->getWorldTransform(trans);
               // else trans = body->body->getWorldTransform();
                model = glm::translate(glm::mat4(1.0f),glm::vec3(float(trans.getOrigin().getX()),
                    float(trans.getOrigin().getY()), float(trans.getOrigin().getZ())));}
            }

            shader.setAttribute("a_color",std::vector<GLfloat>{color.r,color.g,color.b,color.a});
            shader.setUniform("u_projView",cam.getCombined() * model,false);
            if(textureHandle) {
                entityx::ComponentHandle<gel::Resource> resource = (*textureHandle.get()).component<gel::Resource>();
                glBindTexture(GL_TEXTURE_2D,((gel::Texture*)resource->type)->tex);
            }
            vertices.draw(GL_TRIANGLES);
            shader.end();
         });
    }
};

gel::Camera RenderSystem::cam;
int RenderSystem::x = 30;
float RenderSystem::rotateAmount = 0.0f;
float RenderSystem::rotateSpeed = 0.05f;
bool RenderSystem::drag = false;
glm::vec3 RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(glm::vec3(50,10,50));
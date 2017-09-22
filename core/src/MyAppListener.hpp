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

#include <DefaultAppListener.hpp>

#include "Camera.hpp"
#include "Input.hpp"
#include "ShapeBuilder.hpp"

class MyRawInputProcessor: public gel::RawInputProcessor{
public:
    bool controllerAxisEvent(const SDL_ControllerAxisEvent& event){
        SDL_Log("[SDL_ControllerAxisEvent] axis: %i,value: %i",event.axis,event.value);
        return true;
    }
    
    bool controllerButtonEvent(const SDL_ControllerButtonEvent& event){
        switch(event.type){
            case SDL_CONTROLLERBUTTONDOWN: SDL_Log("[SDL_CONTROLLERBUTTONDOWN]");break;
            case SDL_CONTROLLERBUTTONUP: SDL_Log("[SDL_CONTROLLERBUTTONUP]");break;
        }
        return true;
    }
    
    bool controllerDeviceEvent(const SDL_ControllerDeviceEvent& event){
        switch(event.type){
            case SDL_CONTROLLERDEVICEADDED: SDL_Log("[SDL_CONTROLLERDEVICEADDED]");break;
            case SDL_CONTROLLERDEVICEREMOVED: SDL_Log("[SDL_CONTROLLERDEVICEREMOVED]");break;
            case SDL_CONTROLLERDEVICEREMAPPED: SDL_Log("[SDL_CONTROLLERDEVICEREMAPPED]");break;
        }
        return true;
    }

    bool touchFingerEvent(const SDL_TouchFingerEvent& event){
        switch(event.type){
            case SDL_FINGERDOWN: SDL_Log("[SDL_FINGERDOWN]");
                RenderSystem::drag = true;
                break;
            case SDL_FINGERUP: SDL_Log("[SDL_FINGERUP]");
                RenderSystem::drag = false;
                RenderSystem::rotateAmount = 0.0f;
                break;
            case SDL_FINGERMOTION: SDL_Log("[SDL_FINGERMOTION]: %f,%f",event.dx,event.dy);
                if(RenderSystem::drag) RenderSystem::rotateAmount += event.dx;
                break;
        }
        return true;
    }
    bool keyboardEvent(const SDL_KeyboardEvent& event){
        SDL_Log("[SDL_KEYBOARDEVENT]");
        return true;
    }

    bool mouseMotionEvent(const SDL_MouseMotionEvent& event){
        SDL_Log("[SDL_MOUSEMOTIONEVENT]: %i,%i",event.xrel,event.yrel);
        if(RenderSystem::drag) RenderSystem::rotateAmount += event.xrel;
        return true;
    }
    
    bool mouseButtonEvent(const SDL_MouseButtonEvent& event){
        switch(event.type){
            case SDL_MOUSEBUTTONDOWN: SDL_Log("[SDL_MOUSEBUTTONDOWN]");
                RenderSystem::drag = true;
                break; 
            case SDL_MOUSEBUTTONUP: SDL_Log("[SDL_MOUSEBUTTONUP]");
                RenderSystem::drag = false;
                RenderSystem::rotateAmount = 0.0f;
                break;
        }
        return true;
    }
    
    bool mouseWheelEvent(const SDL_MouseWheelEvent& event){
        SDL_Log("[SDL_MOUSEWHEELEVENT]: %i",event.y);
        RenderSystem::x -= event.y;
        RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(RenderSystem::eye);
        return true;
    }
    
    bool multiGestureEvent(const SDL_MultiGestureEvent& event){
        SDL_Log("[SDL_MULTIGESTUREEVENT]: %f",event.dDist);
        RenderSystem::x -= event.dDist;
        RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(RenderSystem::eye);
        return true;
    }  
};

class MyAppListener: public gel::DefaultAppListener{
public:
 bool create(){
    DefaultAppListener::create();

    //Setup gl settings.
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor( 0.66f, 0.66f, 0.66f, 1.f );
            
    //Setup input.
    setRawInputProcessor(std::make_shared<MyRawInputProcessor>());
        
    //Setup shader program.
    std::string prepend = "#version 300 es\n";
    gel::Asset<gel::ShaderProgram> defaultShader = newAsset<gel::ShaderProgram>(newEntity()).assign(gel::ShaderSpec("default",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/default.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/default.frag",GL_FRAGMENT_SHADER,prepend)
    })),
    alShader = newAsset<gel::ShaderProgram>(newEntity()).assign(gel::ShaderSpec("alt",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/alt.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/alt.frag",GL_FRAGMENT_SHADER,prepend)
    })),
    texShader = newAsset<gel::ShaderProgram>(newEntity()).assign(gel::ShaderSpec("tex",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/texTest.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/texTest.frag",GL_FRAGMENT_SHADER,prepend)
    }));
    
    //Create sphere.
    std::vector<GLfloat> vertices = std::vector<GLfloat>();
    std::vector<GLuint> indices = std::vector<GLuint>();
    PhysicsSystem::shapes.push_back(new btSphereShape(1.0f));
    gel::ShapeBuilder::buildSphere(vertices,indices,2,2,2,20,20);
    gel::Asset<gel::VertexReference> groundVertex, sphereVertex = newAsset<gel::VertexReference>(newEntity()).assign(texShader)
        .assign(gel::Vertex(
            std::vector<gel::VertexSpec>{
                gel::VertexSpec(GL_FLOAT,3,"a_position"),
                gel::VertexSpec(GL_FLOAT,2,"a_texCoord0")
            },vertices,indices));
    
    //Create ground.
    std::vector<float> obstructions[] = {
        std::vector<float>{5.0f,-8.0f,29.0f,
            50.0f,3.0f,2.0f},
        std::vector<float>{29.0f,-8.0f, 5.0f,
            2.0f, 3.0f, 50.0f},
        std::vector<float>{-19.0f,-8.0f, 5.0f,
            2.0f, 3.0f, 50.0f},
        std::vector<float>{5.0f,-8.0f, -19.0f,
            50.0f, 3.0f, 2.0f}, 
        std::vector<float>{5.0f, -10.0f, 5.0f,
            50.0f, 2.0f, 50.0f}                             
    };

    gel::Asset<gel::TextureReference> obsTexture = newAsset<gel::TextureReference>(newEntity()).assign(gel::Texture("abcdef",true));
    for(std::vector<float> obstruction:obstructions){
        //Build vertex.
        std::vector<GLfloat> tempvertices = std::vector<GLfloat>();
        std::vector<GLuint> tempindices = std::vector<GLuint>();
        gel::ShapeBuilder::buildBox(tempvertices,tempindices,obstruction[3],obstruction[4],obstruction[5]);
        groundVertex = newAsset<gel::VertexReference>(newEntity()).assign(texShader)
            .assign(gel::Vertex(
                std::vector<gel::VertexSpec>{
                    gel::VertexSpec(GL_FLOAT,3,"a_position"),
                    gel::VertexSpec(GL_FLOAT,2,"a_texCoord0")
                },tempvertices,tempindices));
        
        //Build ground entity.
        PhysicsSystem::shapes.push_back(new btBoxShape(btVector3(obstruction[3] / 2.0f,obstruction[4] / 2.0f,obstruction[5] / 2.0f)));
        newAsset<gel::Mesh>(newEntity()).assign(texShader).assign(groundVertex).assign(obsTexture)
            .assign(glm::mat4()).assign(glm::vec4(1.0f,1.0f,1.0f,1.0f)).assign(glm::vec3(obstruction[0],obstruction[1],obstruction[2]))
            .assign(RigidBody(std::string("test"),0.0f,btVector3(obstruction[0],obstruction[1],obstruction[2]),
                PhysicsSystem::shapes[PhysicsSystem::shapes.size()-1]));
    }
    
    std::stringstream sstream(fileToString("assets/cube-drop.txt"));
    int count = 0;
    float x,y,z;
    sstream>>count;
    gel::Asset<gel::TextureReference> sphereTexture = newAsset<gel::TextureReference>(newEntity()).assign(gel::Texture("assets/test.jpg",false));

    while(!sstream.eof()){
        sstream>>x>>y>>z;
        newAsset<gel::Mesh>(newEntity()).assign(texShader).assign(sphereVertex).assign(sphereTexture)
            .assign(glm::mat4()).assign(glm::vec3(x,y,z))
            .assign(glm::vec4(((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,1.0f))
            .assign(RigidBody(std::string("test"),10.0f,btVector3(x,y,z),PhysicsSystem::shapes[0]));
    }
    return true;
 }

 void resize(int width, int height){
    RenderSystem::cam.setAspect((float)width,(float)height);
 }

 void pause(){}
 void resume(){}
};
#pragma once

#include <entityx/entityx.h>
#include <Application.hpp>
#include <chrono>
#include <memory>

#include "Camera.hpp"
#include "Vertices.hpp"
#include "Shader.hpp"
#include "Input.hpp"
#include "ShapeBuilder.hpp"

#include "system/RenderSystem.hpp"
#include "system/PhysicsSystem.hpp"

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

class MyAppListener: public gel::ApplicationListener,entityx::EntityX{
    bool isCreated = false;
    static bool isPaused;
    int counter = 0;
    
    //Camera
    float near = 0.1f,far = 100.0f,fov = 67.0f;
    float width = 640,height = 480;
    
    //Resources
    std::vector<gel::Vertices> vertData;
    std::vector<gel::Shader> shaders;
    std::chrono::time_point<std::chrono::system_clock> lastTime;
public:
 bool create(){
    //Entityx configuration.
    systems.add<PhysicsSystem>();
    systems.add<RenderSystem>();
    systems.configure();
    
    //Setup gl settings.
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor( 0.66f, 0.66f, 0.66f, 1.f );
            
    setRawInputProcessor(std::make_shared<MyRawInputProcessor>());
        
    //Setup shader program.
    shaders.push_back(gel::Shader("assets/default","#version 300 es\n"));
    shaders.push_back(gel::Shader("assets/alt","#version 300 es\n"));
    shaders.push_back(gel::Shader("assets/texTest","#version 300 es\n"));
    
    std::vector<GLfloat> vertices = std::vector<GLfloat>();
    std::vector<GLuint> indices = std::vector<GLuint>();
    gel::ShapeBuilder::buildSphere(vertices,indices,2,2,2,20,20);
    
    vertData.push_back(gel::Vertices(
        std::vector<gel::VertexDescriptor>{gel::VertexDescriptor(GL_FLOAT,3,
        shaders[1].getAttribute("a_position"))},vertices,indices));
    
    //Create ground.
    entityx::Entity ground;
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

    for(std::vector<float> obstruction:obstructions){
        ground = entities.create();
        std::vector<GLfloat> tempvertices = std::vector<GLfloat>();
        std::vector<GLuint> tempindices = std::vector<GLuint>();
        gel::ShapeBuilder::buildBox(tempvertices,tempindices,obstruction[3],obstruction[4],obstruction[5]);
        ground.assign<gel::Vertices>(gel::Vertices(std::vector<gel::VertexDescriptor>{gel::VertexDescriptor(GL_FLOAT,3,
            shaders[1].getAttribute("a_position"))},tempvertices,tempindices));
        ground.assign<gel::Shader>(shaders[1]);
        ground.assign<glm::vec3>(glm::vec3(obstruction[0],obstruction[1],obstruction[2]));
        ground.assign<glm::vec4>(glm::vec4(((float) rand()) / (float) RAND_MAX/*1.0f*/,((float) rand()) / (float) RAND_MAX/*1.0f*/,((float) rand()) / (float) RAND_MAX/*1.0f*/,1.0f));
        ground.assign<RigidBody>(RigidBody(std::string("test"),0.0f,btVector3(obstruction[0],obstruction[1],obstruction[2]),
            new btBoxShape(btVector3(obstruction[3] / 2.0f,obstruction[4] / 2.0f,obstruction[5] / 2.0f))));
    }
    
    std::stringstream sstream(fileToString("assets/cube-drop.txt"));
    int count = 0;
    float x,y,z;
    entityx::Entity temp;
    sstream>>count;
    btSphereShape* sphere = new btSphereShape(1.0f);

    while(!sstream.eof()){
        sstream>>x>>y>>z;
        temp = entities.create();
        temp.assign<gel::Vertices>(vertData[0]);
        temp.assign<gel::Shader>(shaders[1]);
        temp.assign<glm::vec3>(glm::vec3(x,y,z));
        temp.assign<glm::vec4>(glm::vec4(((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,1.0f));
        temp.assign<RigidBody>(RigidBody(std::string("test"),10.0f,btVector3(x,y,z),sphere));
    }
    return true;
 }

 void render(){
    glViewport(0,0,width,height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::duration<double> delta = now - lastTime;lastTime = now;
    entityx::TimeDelta dt = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count() * 0.001f;

    systems.update<PhysicsSystem>(dt);
    systems.update<RenderSystem>(dt);
 }

 void resize(int width, int height){
    this->width = (float)width;this->height = (float)height;
    RenderSystem::cam.setAspect(this->width/this->height);
 }

 void pause(){}
 void resume(){}
 void dispose(){}
};

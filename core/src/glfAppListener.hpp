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

entityx::TimeDelta dt;
#include <DefaultAppListener.hpp>

#include "Camera.hpp"
#include "Input.hpp"
#include "helper/UIBuilder.hpp"
#include "helper/ShapeBuilder.hpp"

#include <rapidxml.hpp>

#include "helper/Utility.hpp"

std::vector<gel::Asset<gel::Mesh>> meshes;
std::map<SDL_Keycode,bool> keyMap;

#include "interfaces/GL_Interface.hpp"

float zoom = 45.0f;
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
            case SDL_FINGERDOWN: //SDL_Log("[SDL_FINGERDOWN]");
                RenderSystem::drag = true;
                break;
            case SDL_FINGERUP: //SDL_Log("[SDL_FINGERUP]");
                RenderSystem::drag = false;
                RenderSystem::rotateAmount = 0.0f;
                break;
            case SDL_FINGERMOTION: //SDL_Log("[SDL_FINGERMOTION]: %f,%f",event.dx,event.dy);
                if(RenderSystem::drag) RenderSystem::rotateAmount += event.dx;
                break;
        }
        return true;
    }
    bool keyboardEvent(const SDL_KeyboardEvent& event){
        //SDL_Log("[SDL_KEYBOARDEVENT]");
        if(event.type == SDL_KEYDOWN) keyMap[event.keysym.sym] = true;
        else keyMap[event.keysym.sym] = false;
        return true;
    }

    bool mouseMotionEvent(const SDL_MouseMotionEvent& event){
        //SDL_Log("[SDL_MOUSEMOTIONEVENT]: %i,%i",event.xrel,event.yrel);
        if(RenderSystem::drag) RenderSystem::rotateAmount += event.xrel;
        return true;
    }
    
    bool mouseButtonEvent(const SDL_MouseButtonEvent& event){
        switch(event.type){
            case SDL_MOUSEBUTTONDOWN: //SDL_Log("[SDL_MOUSEBUTTONDOWN]: %i,%i",event.x,event.y);
                RenderSystem::drag = true;
                break; 
            case SDL_MOUSEBUTTONUP: //SDL_Log("[SDL_MOUSEBUTTONUP]");
                RenderSystem::drag = false;
                RenderSystem::rotateAmount = 0.0f;
                break;
        }
        return true;
    }
    
    bool mouseWheelEvent(const SDL_MouseWheelEvent& event){
        //SDL_Log("[SDL_MOUSEWHEELEVENT]: %i",event.y);
        zoom = glm::clamp(zoom - event.y,30.0f,120.0f/*90.0f*/);
        return true;
    }
    
    bool multiGestureEvent(const SDL_MultiGestureEvent& event){
       // SDL_Log("[SDL_MULTIGESTUREEVENT]: %f",event.dDist);
        RenderSystem::x -= event.dDist;
        RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(RenderSystem::eye);
        return true;
    }  
};

class glfAppListener: public gel::DefaultAppListener{
public:
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    gel::Asset<gel::ShaderProgram> altShader;
    //glTF TEST
    gel::model model;
    std::map<std::string,unsigned int> locations;
    //glTF TEST
 bool create(){
    DefaultAppListener::create();

    //Setup gl settings.
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(2.0f);
    glClearColor( 0.66f, 0.66f, 0.66f, 1.f );
            
    //Setup input.
    setRawInputProcessor(std::make_shared<MyRawInputProcessor>());

    //Fonts
    gel::Asset<gel::FontReference> sans_reg_16 = assets.load<gel::FontReference,gel::Font>("assets/font/OpenSans-Regular.ttf",16);

    //Textures
    gel::Asset<gel::TextureReference> sphereTexture = assets.load<gel::TextureReference,gel::Texture>("assets/test.jpg");
        
    //Setup shader program.
    std::string prepend = "#version 300 es\n";
    altShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("alt",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/alt.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/alt.frag",GL_FRAGMENT_SHADER,prepend)
    });

    //glTF TEST
    fillWithJSON(model,"assets/BoxAnimated.gltf");
    locations["POSITION"] = 0;
    loadModel(model,"assets/",locations);
    //glTF TEST
    return true;
}

void resize(int width, int height){
    RenderSystem::cam.setAspect((float)width,(float)height);
}

void render(){
    DefaultAppListener::render();
    //glTF TEST
    glViewport(0,0,640,480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderModel(model,*altShader.component<gel::ShaderProgram>(),zoom);
    //glTF TEST
}

 void pause(){}
 void resume(){}
};
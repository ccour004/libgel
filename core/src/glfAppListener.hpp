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

#include <Application.hpp>
#include <deque>
#include "interfaces/GL_Interface.hpp"

enum CMDVal {quit,meshcmd,modelcmd,cameracmd};
std::map<std::string,CMDVal> commandMap = std::map<std::string,CMDVal>{
    {"quit",quit},{"mesh",meshcmd},{"model",modelcmd},{"camera",cameracmd}
};

std::string myCommand(std::string cmd,gel::model& model,gel::camera& camera){
    std::stringstream ss(cmd);
    while(!ss.eof()){
        std::string output;
        ss >> output;
        SDL_Log("OUTPUT: %s",output.c_str());
        switch(commandMap[output]){
            case quit: return "ACK: quit";
            case cameracmd:
                ss>>output;
                float index1,index2,index3;
                if(output == "setTranslate"){
                    ss>>index1>>index2>>index3;
                    camera.setTranslate(glm::vec3(index1,index2,index3));
                }
                return ">set camera";
            case modelcmd:
                ss>>output;
                if(output == "load"){
                    model.clear();
                    ss>>output;
                    fillWithJSON(model,"assets/"+output+".gltf");
                    loadModel(model,"assets/");
                }
                return ">load model";
            case meshcmd:
                ss>>output;
                int index;
                if(output == "setvisible"){
                    ss>>index;
                    model.meshes[index].isVisible = true;
                }else if(output == "setinvisible"){
                    ss>>index;
                    model.meshes[index].isVisible = false;
                }
            return ">set mesh visibility";
        }
    }
    return "<UNKNOWN COMMAND>";
}

class MyRawInputProcessor: public gel::RawInputProcessor{
    gel::camera* camera;
    gel::model* model;
    std::function<std::string(std::string,gel::model&,gel::camera&)> cmdptr;
    bool drag = false,shift = false;
    std::map<SDL_Keycode,bool> keyMap;
    std::vector<SDL_Keycode> keyBuffer;
    std::deque<std::string> commandBuffer;
    float speed = 0.01f,rotateX = 0.0f,rotateY = 0.0f;
public:
    MyRawInputProcessor(gel::camera* camera,gel::model* model,std::function<std::string(std::string,gel::model&,gel::camera&)> cmdptr):camera(camera),model(model),cmdptr(cmdptr){}
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
                drag = true;
                break;
            case SDL_FINGERUP: //SDL_Log("[SDL_FINGERUP]");
                drag = false;
                break;
            case SDL_FINGERMOTION: //SDL_Log("[SDL_FINGERMOTION]: %f,%f",event.dx,event.dy);
                if(drag){
                    rotateX += event.dx * speed;
                    rotateY += event.dy * speed;
                    SDL_Log("XROT: %f,YROT: %f",rotateX,rotateY);
                    rotateY = glm::clamp(rotateY,-1.5f,1.5f);
                    camera->setRotate(rotateX,rotateY,0);
                }
                break;
        }
        return true;
    }
    bool keyboardEvent(const SDL_KeyboardEvent& event){
        //SDL_Log("[SDL_KEYBOARDEVENT]");
        if(event.type == SDL_KEYDOWN) {
            keyMap[event.keysym.sym] = true;
            std::string output = "";
            if(event.keysym.sym == 8 && keyBuffer.size() > 0) keyBuffer.pop_back();
            else if (event.keysym.sym != 8 && event.keysym.sym != SDLK_RSHIFT && event.keysym.sym != SDLK_LSHIFT) 
                keyBuffer.push_back(keyMap[SDLK_RSHIFT] || keyMap[SDLK_LSHIFT]?toupper(event.keysym.sym):event.keysym.sym);
            for(SDL_Keycode keycode:keyBuffer) output += keycode;
            #ifdef _WIN32 
            system("cls"); 
            #else 
            system("clear"); 
            #endif
            if(event.keysym.sym == 13){
                keyBuffer.clear();
                if(commandBuffer.size() > 256) commandBuffer.pop_front();
                commandBuffer.push_back(output);
                std::string latest = cmdptr(output,*model,*camera);
                commandBuffer.push_back(latest);
                if(latest == "ACK: quit")quit();
                output = "";
            }
            for(std::string cmd:commandBuffer)SDL_Log("%s",cmd.c_str());
            SDL_Log("CMD: <%s>",output.c_str());
        }
        else keyMap[event.keysym.sym] = false;
        return true;
    }

    bool mouseMotionEvent(const SDL_MouseMotionEvent& event){
        //SDL_Log("[SDL_MOUSEMOTIONEVENT]: %i,%i",event.xrel,event.yrel);
        if(drag){
            rotateX += event.xrel * speed;
            rotateY += event.yrel * speed;
            SDL_Log("XROT: %f,YROT: %f",rotateX,rotateY);
            rotateY = glm::clamp(rotateY,-1.5f,1.5f);
            camera->setRotate(rotateX,rotateY,0);
        }
        return true;
    }
    
    bool mouseButtonEvent(const SDL_MouseButtonEvent& event){
        switch(event.type){
            case SDL_MOUSEBUTTONDOWN: //SDL_Log("[SDL_MOUSEBUTTONDOWN]: %i,%i",event.x,event.y);
                drag = true;
                break; 
            case SDL_MOUSEBUTTONUP: //SDL_Log("[SDL_MOUSEBUTTONUP]");
                drag = false;
                break;
        }
        return true;
    }
    
    bool mouseWheelEvent(const SDL_MouseWheelEvent& event){
        //SDL_Log("[SDL_MOUSEWHEELEVENT]: %i",event.y);
        camera->setFov(glm::clamp(camera->getFov() - event.y,30.0f,120.0f/*90.0f*/));
        return true;
    }
    
    bool multiGestureEvent(const SDL_MultiGestureEvent& event){
       // SDL_Log("[SDL_MULTIGESTUREEVENT]: %f",event.dDist);
        camera->setFov(glm::clamp(camera->getFov() - event.dDist,30.0f,120.0f/*90.0f*/));
        return true;
    }  
};

class glfAppListener: public gel::ApplicationListener{
public:
    gel::model model;
    gel::camera camera;
    float curr = 0.0f;
 bool create(){
     /**TODO: Set sane defaults for technique.states in glfw code instead of declaring these here.**/
    //Setup gl settings.
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(2.0f);
    glClearColor(0.66f,0.66f,0.66f,1.f);
    /**TODO**/
            
    //Setup input and camera.
    setRawInputProcessor(std::make_shared<MyRawInputProcessor>(&camera,&model,myCommand));
    camera = gel::camera(gel::perspective(640.0f/480.0f,45.0f,0.1f,1000.0f));
    camera.setTranslate(glm::vec3(0,0,-500/*-10*/));
    fillWithJSON(model,"assets/scene.gltf"/*scene*/);
    loadModel(model,"assets/");
    return true;
}

void resize(int width, int height){
    camera.setAspectRatio(width,height);
}

void render(){
    /**TODO: glClear() should be abstracted behind a rendering interface**/
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /**TODO**/
    curr += 0.01f;
    renderModel(model,camera,curr);
}

 void dispose(){}
 void pause(){}
 void resume(){}
};
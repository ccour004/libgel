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

#include <SDL.h>
#include <SDL_ttf.h>
#include "Common.hpp"
#include "Input.hpp"

namespace gel
{
    class ApplicationListener{
        std::shared_ptr<RawInputProcessor> input;
    public:
        virtual bool create() = 0;
        virtual void resize(int width, int height) = 0;
        virtual void render() = 0;
        virtual void pause() = 0;
        virtual void resume() = 0;
        virtual void dispose() = 0;

        void setRawInputProcessor(std::shared_ptr<RawInputProcessor> input){this->input = input;};
        std::shared_ptr<RawInputProcessor> getRawInputProcessor(){return input;}
    };

    class Configuration{
    public:
        int depth = 16,stencil = 0;
        int gl_major_version = 3,gl_minor_version = 0;
        int multiSampleBuffer = 0,multiSampleSamples = 0;
        void setAntiAlias(){multiSampleBuffer = 1;multiSampleSamples = 2;}
        void setBetterAntiAlias(){multiSampleBuffer = 1;multiSampleSamples = 4;}
    };

    class DesktopConfiguration: public Configuration{
    public:
        int width = 640,height = 480;
        int x = -1,y = -1;
        bool fullscreen = false,resizable = true;
        std::string title = "Test Window";
        DesktopConfiguration(){}
    };

    class MobileConfiguration: public Configuration{
    public:
        MobileConfiguration(){}
    };

    class Application{
    private:
        std::shared_ptr<ApplicationListener> listener;
        SDL_Window* window;
        SDL_GLContext glContext;
        
        void dispose();
        bool setAttributes(std::shared_ptr<DesktopConfiguration> desktop,std::shared_ptr<MobileConfiguration> mobile){
            #ifdef DESKTOP
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,desktop->gl_major_version);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,desktop->gl_minor_version);  
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, desktop->depth);  
                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, desktop->stencil); 
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, desktop->multiSampleBuffer);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desktop->multiSampleSamples);
            #else
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,mobile->gl_major_version);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,mobile->gl_minor_version);  
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, mobile->depth);  
                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, mobile->stencil); 
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, mobile->multiSampleBuffer);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mobile->multiSampleSamples);
            #endif
        
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            
            //Build window and context.
            SDL_Log("++BUILD SDL WINDOW AND SDL/GL CONTEXT++");
            int params = SDL_WINDOW_OPENGL;
            #ifdef DESKTOP
                if(desktop->resizable) params |= SDL_WINDOW_RESIZABLE;
                if(desktop->fullscreen) params |= SDL_WINDOW_FULLSCREEN;
                window = SDL_CreateWindow(desktop->title.c_str(),
                    desktop->x > -1 ? desktop->x : SDL_WINDOWPOS_UNDEFINED,
                    desktop->y > -1 ? desktop->y : SDL_WINDOWPOS_UNDEFINED,
                    desktop->width,desktop->height,params);
            #else
                window = SDL_CreateWindow("",
                    SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,
                    640,480,params);        
            #endif
            if (window == NULL){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Could not create window: %s", SDL_GetError());
                return false;
            }
            glContext = SDL_GL_CreateContext(window);
            if(glContext == NULL){
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Could not create context: %s", SDL_GetError());
                return false;
            }   
            return true;     
        }
    public:
        Application(std::shared_ptr<ApplicationListener> listener):
            Application(std::make_shared<DesktopConfiguration>(),std::make_shared<MobileConfiguration>(),listener){};
        Application(std::shared_ptr<DesktopConfiguration> desktop,std::shared_ptr<ApplicationListener> listener):
            Application(desktop,std::make_shared<MobileConfiguration>(),listener){};
        Application(std::shared_ptr<MobileConfiguration> mobile,std::shared_ptr<ApplicationListener> listener):
            Application(std::make_shared<DesktopConfiguration>(),mobile,listener){};
        Application(std::shared_ptr<DesktopConfiguration> desktop,std::shared_ptr<MobileConfiguration> mobile,
            std::shared_ptr<ApplicationListener> listener){
                this->listener = listener;
                //Initialization flag
                SDL_Log("++INITIALIZE SDL++");
                SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);

                SDL_Log("++INITIALIZE SDL_TTF++");
                if(TTF_Init() == -1){
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Error initializing SDL_TTF! \n");
                    return;
                }
        
                SDL_Log("++SET OPENGL ATTRIBUTES++");
                setAttributes(desktop,mobile);
                
                #ifdef DESKTOP
                    GLenum glewError = glewInit(); 
                    if(glewError != GLEW_OK){
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Error initializing GLEW! %s\n",glewGetErrorString(glewError));
                        return;
                    }
                #endif
        
                //Use Vsync
                if(SDL_GL_SetSwapInterval(1) < 0){
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
                    return;
                }
        
                //Let listener know that we're created now.
                SDL_Log("++CREATE APPLISTENER++");
                if(!listener->create()){
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"AppListener create failed!");
                    return;	
                }
                listener->resize(640,480);
        
                //Main loop.
                SDL_Log("++START GAME LOOP++");
                SDL_Event e;
                bool isPaused = false,quit = false;
                while(!quit){
                    while(SDL_PollEvent(&e) != 0){
                        //System events first.
                        if(e.type == SDL_QUIT){quit = true;SDL_Log("!!!SDL_QUIT!!!"); continue;}
                        switch(e.type){
                            case SDL_APP_TERMINATING:SDL_Log("!!!SDL_APP_TERMINATING!!!");
                            case SDL_APP_LOWMEMORY:SDL_Log("!!!SDL_APP_LOWMEMORY!!!");
                                quit = true;
                                continue;
                            case SDL_WINDOWEVENT:
                                if (e.window.windowID == SDL_GetWindowID(window)) {
                                    switch (e.window.event) {
                                        case SDL_WINDOWEVENT_CLOSE:
                                            quit = true;SDL_Log("!!!SDL_WINDOWEVENT_CLOSE!!!");
                                            continue;
                                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                                            listener->resize(e.window.data1,e.window.data2);
                                            break;
                                        case SDL_WINDOWEVENT_MINIMIZED:
                                            listener->pause();
                                            isPaused = true;
                                            break;
                                        case SDL_WINDOWEVENT_RESTORED:
                                            listener->resume();
                                            isPaused = false;
                                            break;
                                    }
                                }
                                break;
                        }
                        
                        //Now touch events, if an input processor is set.
                        if(listener->getRawInputProcessor().get() != nullptr)
                        switch(e.type){
                            case SDL_CONTROLLERAXISMOTION:listener->getRawInputProcessor()->controllerAxisEvent((const SDL_ControllerAxisEvent&)e); break;
                            case SDL_CONTROLLERBUTTONDOWN:
                            case SDL_CONTROLLERBUTTONUP:
                                listener->getRawInputProcessor()->controllerButtonEvent((const SDL_ControllerButtonEvent&)e); break;
                            case SDL_CONTROLLERDEVICEADDED:
                            case SDL_CONTROLLERDEVICEREMOVED:
                            case SDL_CONTROLLERDEVICEREMAPPED:
                                listener->getRawInputProcessor()->controllerDeviceEvent((const SDL_ControllerDeviceEvent&)e); break;
                            case SDL_FINGERMOTION: 
                            case SDL_FINGERDOWN:
                            case SDL_FINGERUP:
                                listener->getRawInputProcessor()->touchFingerEvent((const SDL_TouchFingerEvent&)e); break;
                            case SDL_KEYDOWN: 
                            case SDL_KEYUP:
                                listener->getRawInputProcessor()->keyboardEvent((const SDL_KeyboardEvent&)e); break;
                            case SDL_MOUSEMOTION: listener->getRawInputProcessor()->mouseMotionEvent((const SDL_MouseMotionEvent&)e); break;
                            case SDL_MOUSEBUTTONDOWN:
                            case SDL_MOUSEBUTTONUP:
                                listener->getRawInputProcessor()->mouseButtonEvent((const SDL_MouseButtonEvent&)e); break;
                            case SDL_MOUSEWHEEL: listener->getRawInputProcessor()->mouseWheelEvent((const SDL_MouseWheelEvent&)e); break;
                            case SDL_MULTIGESTURE: listener->getRawInputProcessor()->multiGestureEvent((const SDL_MultiGestureEvent&)e); break;
                            case SDL_TEXTINPUT:
                                if(e.text.text[0] == 'q') quit = true;
                                break;
                        }
                    }
        
                    if(!isPaused){
                        listener->render();
                        SDL_GL_SwapWindow(window);
                    }
               }
                
               SDL_Log("--STOP GAME LOOP AND TEAR DOWN--");
               listener->dispose();
               SDL_GL_DeleteContext(glContext);
               SDL_DestroyWindow(window);
               SDL_Quit();
            }
    };
}
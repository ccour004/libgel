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
#include <entityx/deps/Dependencies.h>
#include <Application.hpp>
#include <chrono>

#include "system/AssetSystem.hpp"

#include <thread>

namespace gel{
    class DefaultAppListener: public gel::ApplicationListener{
        std::chrono::time_point<std::chrono::system_clock> now,lastTime;
        std::chrono::duration<double> delta;
    protected:
        AssetSystem assets;
    public:
        static bool quitFlag,threadFlag,isPaused,isCreated;
        static /*void*/int PhysicsThread(/*entityx::SystemManager**/void* /*systems_ptr*/systems_ptr_v){
            /**/entityx::SystemManager* systems_ptr = (entityx::SystemManager*)systems_ptr_v;/**/
            std::chrono::time_point<std::chrono::system_clock> thread_now,thread_lastTime;
            std::chrono::duration<double> thread_delta;

            while(!quitFlag){
                thread_now = std::chrono::system_clock::now();
                thread_delta = thread_now - thread_lastTime;thread_lastTime = thread_now;
                systems_ptr->update<PhysicsSystem>(std::chrono::duration_cast<std::chrono::milliseconds>(thread_delta).count());
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            return 0;
        }
        bool create(){
            //Entityx configuration.
            assets.getSystems().add<PhysicsSystem>();
            assets.getSystems().add<RenderSystem>();
            assets.getSystems().add<TextureSystem>();
            assets.getSystems().add<FontSystem>();
            assets.getSystems().add<ShaderSystem>();
            assets.getSystems().add<VertexSystem>();
            assets.getSystems().configure(); 
            return true;
        }

        void render(){   
            if(!threadFlag){
                //Thread configuration.
                //std::thread(PhysicsThread,&systems).detach();
                SDL_DetachThread(SDL_CreateThread(PhysicsThread, "PhysicsThread",&assets.getSystems()));
                threadFlag = true;
            }     
            now = std::chrono::system_clock::now();
            delta = now - lastTime;lastTime = now;
            dt = std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
            
            assets.getSystems().update<TextureSystem>(dt);
            assets.getSystems().update<FontSystem>(dt);
            assets.getSystems().update<VertexSystem>(dt);
            assets.getSystems().update<ShaderSystem>(dt);
            //assets.getSystems().update<PhysicsSystem>(dt);
            //assets.getSystems().update<RenderSystem>(dt);
        }

        void dispose(){
            quitFlag = true;
            assets.getEntities().reset();
        }

        entityx::Entity newEntity(){return assets.getEntities().create();}
    };  
    bool DefaultAppListener::quitFlag = false;
    bool DefaultAppListener::threadFlag = false;
    bool DefaultAppListener::isCreated = false;
}
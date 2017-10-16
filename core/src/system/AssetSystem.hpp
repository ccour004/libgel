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

#include "../Common.hpp"

#include "RenderSystem.hpp"
#include "PhysicsSystem.hpp"
#include "FontSystem.hpp"
#include "TextureSystem.hpp"
#include "ShaderSystem.hpp"
#include "VertexSystem.hpp"

namespace gel{
    struct Queued{};
    struct Loaded{};
}

class AssetSystem: public entityx::System<AssetSystem>,entityx::EntityX{
public:
    void configure(entityx::EventManager& events) override{
        systems.add<TextureSystem>();
        systems.add<FontSystem>();
        systems.add<ShaderSystem>();
        systems.add<VertexSystem>();
        systems.configure(); 
    }

    template<typename T> gel::Asset<T> load(){
        entityx::Entity ent = entities.create();
        ent.assign<gel::Queued>();
        return gel::Asset<T>(ent);
    }

    template<typename T,typename U,typename ... Args> gel::Asset<T> load(Args && ... args){
        entityx::Entity ent = entities.create();
        ent.assign<U>(args...);
        ent.assign<gel::Queued>();
        return gel::Asset<T>(ent);
    }

    entityx::SystemManager& getSystems(){return systems;}
    entityx::EntityManager& getEntities(){return entities;}

    /*template<class T> bool isQueued(gel::Asset<T> asset){
        return (bool)asset.component<gel::Queued>();
    }

    template<class T> bool isLoaded(gel::Asset<T> asset){
        return (asset.component<gel::Loaded>() == nullptr)?false:true;
    }*/

    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        /*entities.each<gel::Queued>([](entityx::Entity entity,gel::Queued& queued) {
            //TODO: work through the queue of load requests (maybe conditional loads/unloads, threads for parallel jobs later)
        });*/
        systems.update<TextureSystem>(dt);
        systems.update<FontSystem>(dt);
        systems.update<ShaderSystem>(dt);
        systems.update<VertexSystem>(dt);
    }
    ~AssetSystem(){
        SDL_Log("Removing asset system...");
    }
};
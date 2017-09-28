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

namespace gel{
    struct Font{
        std::string filename,name;
        int size;
        Font(std::string filename,int size):filename(filename),size(size){}     
    };
    struct FontReference{
        TTF_Font* font;
        std::string name;
        FontReference(std::string name):name(name){}
    };
}

class FontSystem: public entityx::System<FontSystem>,public entityx::Receiver<FontSystem>{
public:
    void configure(entityx::EventManager& events) override{
        events.subscribe<entityx::ComponentRemovedEvent<gel::FontReference>>(*this);
        if(TTF_Init() < 0)
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TTF_Init() failed!");
    }
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        entities.each<gel::Font>([](entityx::Entity entity,gel::Font& font) {
            gel::FontReference ref = gel::FontReference(font.filename);
            ref.font = TTF_OpenFont(font.filename.c_str(),font.size);
            if(!ref.font) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Error loading TTF font: %s \n",font.filename.c_str());
            else{
                SDL_Log("Loaded font: %s with size: %i",font.filename.c_str(),font.size);
                entity.component<gel::Font>().remove();
                entity.assign<gel::FontReference>(ref);
            }
        });
    }
    void receive(const entityx::ComponentRemovedEvent<gel::FontReference>& event){
        SDL_Log("Removing font...");
        TTF_CloseFont(event.component.get()->font);
    }
    ~FontSystem(){
        SDL_Log("Removing font system...");
        TTF_Quit();
    }
};
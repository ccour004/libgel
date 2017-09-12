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

namespace gel
{
    struct ResourceType{
        std::string filename;
        bool isLoaded = false;
        virtual void load() = 0;
    };

    struct Resource{
        ResourceType* type;
        Resource(ResourceType* type):type(type){}
        void load(){if(!type->isLoaded) type->load();}
    };

    struct Texture:public ResourceType{
        SDL_Surface* image;
        GLuint tex;
        Texture(std::string filename){this->filename = filename;}
        ~Texture(){delete image;}
        void load() override{
            image = IMG_Load(filename.c_str());
            if(!image)
                SDL_Log("IMG_Load: %s\n",IMG_GetError());
                
            glEnable (GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            int format = GL_RGB;
            switch(image->format->BytesPerPixel){
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
            }
            
            SDL_Log("FORMAT: %i (%i,%i,%i,%i)",image->format->BytesPerPixel,image->format->Rmask,image->format->Gmask,image->format->Bmask,image->format->Amask);
                
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            
            glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0, format, GL_UNSIGNED_BYTE, image->pixels);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
            isLoaded = true;
        }
    };
}

class ResourceSystem: public entityx::System<ResourceSystem>{
public:
    void update(entityx::EntityManager& entities,entityx::EventManager& events,entityx::TimeDelta dt) override{
        entities.each<gel::Resource>([](entityx::Entity entity, gel::Resource& resource) {
           resource.load();
        });
    }
};
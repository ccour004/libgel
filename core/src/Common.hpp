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

#ifdef DESKTOP
	#include <GL/glew.h> 
    #include <SDL_opengl.h> 
	#include <GL/glu.h>
#else
    #include <GLES3/gl3.h>
#endif

#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <memory>
#include <entityx/entityx.h>

char* file_read(const char* filename) {
    SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
    if (rw == NULL) return NULL;

    Sint64 res_size = SDL_RWsize(rw);
    char* res = (char*)malloc(res_size + 1);

    Sint64 nb_read_total = 0, nb_read = 1;
    char* buf = res;
    while (nb_read_total < res_size && nb_read != 0) {
            nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
            nb_read_total += nb_read;
            buf += nb_read;
    }
    SDL_RWclose(rw);
    if (nb_read_total != res_size) {
            free(res);
            return NULL;
    }

    res[nb_read_total] = '\0';
    return res;
}

std::string fileToString(const std::string& filename){
return std::string(file_read(filename.c_str()));
}

namespace gel{
    template<class T>
    class Asset{
        entityx::Entity parent;
    public:
        Asset(){}
        Asset(entityx::Entity parent):parent(parent){}
        entityx::Entity get(){return parent;}
        template<class U> entityx::ComponentHandle<U> component(){return parent.component<U>();}
        template<class U> gel::Asset<T> assign(U component){parent.assign<U>(component); return *this;}
    };
}

template<class T> gel::Asset<T> newAsset(entityx::Entity ent){return gel::Asset<T>(ent);}
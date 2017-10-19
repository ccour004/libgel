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
#include <math.h>

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

    //SVG/XML 
    //<glyph unicode="+" horiz-adv-x="24.833855799373055" d="M10.879777813539466 41.868767292006410 
    //C11.238503434311724 34.096378841939710 11.597229055084085 26.204415184948846 11.955954675856447 18.192876321033953 
    //C11.955954675856447 17.714575493337509 11.238503434311724 17.475425079489344 10.760202606615394 17.475425079489344 
    //C10.102538968532826 17.475425079489344 10.172828141380478 17.841151746598474 10.162326571994868 17.834150700261649 
    //C9.923176158146703 25.606539150328377 9.564450537374341 33.498502807319127 9.205724916601980 41.390466464309938 
    //C9.086149709677898 42.107917705854604 10.879777813539476 42.586218533551047 10.879777813539476 41.868767292006382 
    //zM0.955035638838901 30.030821806520180 
    //C8.607848881981454 30.269972220368334 16.141086918200017 30.987423461913000 23.674324954418466 32.183175531154049 
    //C24.630926609811354 32.302750738078132 24.511351402887271 31.346149082685304 23.674324954418466 31.106998668837139 
    //C23.544039097272162 31.074427204670133 18.886182440443804 29.402891525351766 0.715885224990643 28.835069737279174 
    //C-0.479866844250296 28.835069737279174 -0.001566016553966 29.911246599596087 0.955035638838922 30.030821806520169 z" />

    //horiz-adv-x = amount of x space after the shape is done
    //M = start point
    //C = Bezier curve: ([previous point.x],[previous point.y],p2.x,p2.y,p3.x,p3.y,p4.x,p4.y)
    //z = end path

    glm::vec2 bezierCurve(glm::vec2 p1,glm::vec2 p2,glm::vec2 p3,glm::vec2 p4,float t){
        return glm::vec2(
            pow(1.0f-t,3.0f*p1.x)+pow(3.0f * t * (1.0f - t),2 * p2.x)+pow(3.0f * t,2.0f * (1.0f - t) * p3.x)+pow(t,3 * p4.x),
            pow(1.0f-t,3.0f*p1.y)+pow(3.0f * t * (1.0f - t),2 * p2.y)+pow(3.0f * t,2.0f * (1.0f - t) * p3.y)+pow(t,3 * p4.y),
            //(1-t)^3(P1x) + 3t(1-t)^2(P2x) + 3t^2(1-t)(P3x) + t^3(P4x),
            //(1-t)^3(P1y) + 3t(1-t)^2(P2y) + 3t^2(1-t)(P3y) + t^3(P4y)
        
            //[1 t t^2 t^3] [1 0  0  0] [p1.x p1.y] = [px py]
            //              -3 3  0  0   p2.x p2.y
            //               3 -6 3  0   p3.x p3.y
            //              -1 3 -3  1   p4.x p4.y
        );
    }
};
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

#include <DefaultAppListener.hpp>

#include "Camera.hpp"
#include "Input.hpp"
#include "helper/UIBuilder.hpp"
#include "helper/ShapeBuilder.hpp"

#include <rapidxml.hpp>

#include "helper/Utility.hpp"

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
        RenderSystem::x -= event.y;
        RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(RenderSystem::eye);
        return true;
    }
    
    bool multiGestureEvent(const SDL_MultiGestureEvent& event){
       // SDL_Log("[SDL_MULTIGESTUREEVENT]: %f",event.dDist);
        RenderSystem::x -= event.dDist;
        RenderSystem::eye = ((float)glm::clamp(RenderSystem::x,1,72)) * glm::normalize(RenderSystem::eye);
        return true;
    }  
};

class MyAppListener: public gel::DefaultAppListener{
public:
 bool create(){
    DefaultAppListener::create();

    //Setup gl settings.
    glEnable(GL_DEPTH_TEST);
    glClearDepthf(1.0f);
    glDepthFunc(GL_LEQUAL);
    glLineWidth(2.0f);
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glClearColor( 0.66f, 0.66f, 0.66f, 1.f );
            
    //Setup input.
    setRawInputProcessor(std::make_shared<MyRawInputProcessor>());

    //Fonts
    gel::Asset<gel::FontReference> sans_reg_16 = assets.load<gel::FontReference,gel::Font>("assets/font/OpenSans-Regular.ttf",16);

    //Textures
    gel::Asset<gel::TextureReference> sphereTexture = assets.load<gel::TextureReference,gel::Texture>("assets/test.jpg");
        
    //Setup shader program.
    std::string prepend = "#version 300 es\n";
    gel::Asset<gel::ShaderProgram> defaultShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("default",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/default.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/default.frag",GL_FRAGMENT_SHADER,prepend)
    }),
    altShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("alt",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/alt.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/alt.frag",GL_FRAGMENT_SHADER,prepend)
    }),
    texShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("tex",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/texTest.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/texTest.frag",GL_FRAGMENT_SHADER,prepend)
    });

    //Freetype TEST
    glm::vec2 pos = glm::vec2(0/*-RenderSystem::cam.width/2.0f*/,0);
    float scale = 0.1f,advance_scale = 1e-6,advance = 0;
    glm::vec2 final_pos = glm::vec2(pos.x+RenderSystem::cam.width/2.0f,pos.y+RenderSystem::cam.height/2.0f+65.0f);
    std::string s = "lazy dog"/*"%"*/;
    gel::Asset<gel::VertexReference> glyphVertex;

    std::vector<wchar_t> letters(s.c_str(), s.c_str() + s.length());
    FT_Library library;
    int error = FT_Init_FreeType(&library);
    if(error)SDL_Log("FT Init Error!");

    for(wchar_t letter:letters)
    if(letter == ' ') final_pos.x += /*advance * advance_scale*/10.0f;
    else
    {
        //Get outlines and holes, and process into verts/indices.
        std::vector<OUTLINE> outlines;
        advance = freetype_test(letter,library,"assets/ah_natural.ttf"/*"assets/ipapotamus_4.ttf"*/,outlines).x;
        
        for(OUTLINE outline:outlines){
            //Process.
            std::vector<GLfloat> glyphVertices,outVertices;
            std::vector<GLuint> glyphIndices,outIndices;
            getVerticesAndIndices(outline.points,scale,glyphVertices,glyphIndices);
            std::vector<std::vector<glm::vec2>> holes;
            if(outline.holes.size() > 0) {
                SDL_Log("OUTLINE HAS HOLE(S)!");
                for(std::vector<glm::vec2> hole:outline.holes)
                    holes.push_back(getScaled(hole,scale));
            }

            //Polyline Test
            glyphVertex = assets.load<gel::VertexReference,gel::Vertex>(
                std::vector<gel::VertexSpec>{gel::POSITION},glyphVertices,glyphIndices).assign(altShader);  
            assets.load<gel::Mesh>().assign(final_pos)
                .assign(glm::vec4(0.0f,0.0f,1.0f,1.0f)).assign(glyphVertex).assign(altShader).assign((int)glyphVertices.size());

            //Triangle Test
            triangulate(glyphVertices,holes,outVertices,outIndices);
            glyphVertex = assets.load<gel::VertexReference,gel::Vertex>(
                std::vector<gel::VertexSpec>{gel::POSITION},outVertices,outIndices).assign(altShader);
            assets.load<gel::Mesh>().assign(final_pos)
                .assign(glm::vec4(0.0f,0.0f,0.0f,1.0f)).assign(glyphVertex).assign(altShader);
        }

        //SDL_Log("ADVANCE: %f",advance * 0.0001f);
        final_pos.x += /*advance * advance_scale*/10.0f;
    }
    FT_Done_FreeType(library);
    //Freetype TEST
    
    //Create sphere.
    std::vector<GLfloat> vertices = std::vector<GLfloat>();
    std::vector<GLuint> indices = std::vector<GLuint>();
    PhysicsSystem::shapes.push_back(new btSphereShape(1.0f));
    gel::ShapeBuilder::buildSphere(vertices,indices,2,2,2,20,20);
    gel::Asset<gel::VertexReference> groundVertex, sphereVertex = assets.load<gel::VertexReference,gel::Vertex>(
            std::vector<gel::VertexSpec>{gel::POSITION,gel::TEXTURE_0},vertices,indices).assign(texShader);
    
    //Create ground.
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
    
    //Load obstructions.
    for(std::vector<float> obstruction:obstructions){
        //Build vertex.
        std::vector<GLfloat> tempvertices = std::vector<GLfloat>();
        std::vector<GLuint> tempindices = std::vector<GLuint>();
        gel::ShapeBuilder::buildBox(tempvertices,tempindices,obstruction[3],obstruction[4],obstruction[5]);
        groundVertex = assets.load<gel::VertexReference,gel::Vertex>(std::vector<gel::VertexSpec>{gel::POSITION,gel::TEXTURE_0},tempvertices,tempindices)
                        .assign(texShader);
        
        //Build ground entity.
        PhysicsSystem::shapes.push_back(new btBoxShape(btVector3(obstruction[3] / 2.0f,obstruction[4] / 2.0f,obstruction[5] / 2.0f)));
        assets.load<gel::Mesh>()
            .assign(glm::mat4()).assign(glm::vec4(1.0f,1.0f,1.0f,1.0f)).assign(glm::vec3(obstruction[0],obstruction[1],obstruction[2]))
            .assign(RigidBody(std::string("test"),0.0f,btVector3(obstruction[0],obstruction[1],obstruction[2]),
                PhysicsSystem::shapes[PhysicsSystem::shapes.size()-1]))
            .assign(texShader).assign(groundVertex).assign(sphereTexture);
    }
    
    //Load spheres.
    std::stringstream sstream(fileToString("assets/cube-drop.txt"));
    int count = 0;
    float x,y,z;
    sstream>>count;

    while(!sstream.eof()){
        sstream>>x>>y>>z;
        assets.load<gel::Mesh>()
            .assign(glm::mat4()).assign(glm::vec3(x,y,z))
            .assign(glm::vec4(((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,1.0f))
            .assign(RigidBody(std::string("test"),10.0f,btVector3(x,y,z),PhysicsSystem::shapes[0]))
            .assign(texShader).assign(sphereVertex).assign(sphereTexture);
    }

    //Build UI elements.
    //UIBuilder::addText("Test UI Element",glm::vec2(0.0f,0.0f),glm::vec2(RenderSystem::cam.width*0.25f,RenderSystem::cam.height*0.15f),sans_reg_16,texShader,assets);
    return true;
 }

 void resize(int width, int height){
    RenderSystem::cam.setAspect((float)width,(float)height);
 }

 void pause(){}
 void resume(){}
};
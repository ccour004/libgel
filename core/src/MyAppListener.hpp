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
    gel::Asset<gel::Mesh> zMesh;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    gel::Asset<gel::ShaderProgram> defaultShader,altShader;
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
    defaultShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("default",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/default.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/default.frag",GL_FRAGMENT_SHADER,prepend)
    });
    altShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("alt",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/alt.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/alt.frag",GL_FRAGMENT_SHADER,prepend)
    });
    gel::Asset<gel::ShaderProgram>  textShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("text",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/text.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/text.frag",GL_FRAGMENT_SHADER,prepend)
    }),
    texShader = assets.load<gel::ShaderProgram,gel::ShaderSpec>("tex",std::vector<gel::ShaderSource>{
        gel::ShaderSource("assets/texTest.vert",GL_VERTEX_SHADER,prepend),
        gel::ShaderSource("assets/texTest.frag",GL_FRAGMENT_SHADER,prepend)
    });

    //glTF TEST
    fillWithJSON(model);
    locations["POSITION"] = 0;
    loadModel(model,locations);
    //glTF TEST

    //Freetype TEST
  //  glm::vec2 pos = glm::vec2(0/*-RenderSystem::cam.width/2.0f*/,0);
  //  float scale = 0.1f,advance = 0;
 //   glm::vec2 final_pos = glm::vec2(/*pos.x+RenderSystem::cam.width/2.0f*/0,pos.y+RenderSystem::cam.height/2.0f+65.0f);
 //   std::string s = "lazy dog."/*"%"*/;
  //  gel::Asset<gel::VertexReference> glyphVertex;

 //   std::vector<wchar_t> letters(s.c_str(), s.c_str() + s.length());
 //   FT_Library library;
 //   int error = FT_Init_FreeType(&library);
  //  if(error)SDL_Log("FT Init Error!");

 //   for(wchar_t letter:letters)
 //   if(letter == ' ') final_pos.x += advance;
 //   else
 //   {
        //Get outlines and holes, and process into verts/indices.
  //      std::vector<OUTLINE> outlines;
 //       glm::vec4 params =  freetype_test(letter,library,"assets/ah_natural.ttf"/*"assets/ipapotamus_4.ttf"*/,outlines);
 //       advance = params.x / 65536.0 * scale;
        
  //      for(OUTLINE outline:outlines){
            //Process.
  //          std::vector<GLfloat> glyphVertices,outVertices;
  //          std::vector<GLuint> glyphIndices,outIndices;
  //          getVerticesAndIndices(outline.points,scale,glyphVertices,glyphIndices);
   //         std::vector<std::vector<glm::vec2>> holes;
  //          if(outline.holes.size() > 0) {
  //              SDL_Log("OUTLINE HAS HOLE(S)!");
  //              for(std::vector<glm::vec2> hole:outline.holes)
  //                  holes.push_back(getScaled(hole,scale));
  //              }

            //Polyline Test
            /*glyphVertex = assets.load<gel::VertexReference,gel::Vertex>(
                std::vector<gel::VertexSpec>{gel::POSITION},glyphVertices,glyphIndices).assign(altShader);  
            assets.load<gel::Mesh>().assign(final_pos)
                .assign(glm::vec4(0.0f,0.0f,1.0f,1.0f)).assign(glyphVertex).assign(altShader).assign((int)glyphVertices.size());*/

            //Triangle Test
   //         triangulate(glyphVertices,holes,outVertices,outIndices);
   ///         centerPoints(outVertices,params.z * scale,params.w * scale);
   //         glyphVertex = assets.load<gel::VertexReference,gel::Vertex>(
   //             std::vector<gel::VertexSpec>{gel::POSITION},outVertices,outIndices).assign(letter == 'z'?textShader:altShader);
   //         gel::Asset<gel::Mesh> tempMesh;
   //         tempMesh = assets.load<gel::Mesh>().assign(final_pos)
   //             .assign(glm::vec4(0.0f,0.0f,0.0f,1.0f)).assign(glyphVertex).assign(letter == 'z'?textShader:altShader);
   //         if(letter == 'z') {zMesh = tempMesh;
   //                 auto end = std::chrono::system_clock::now();
   // std::chrono::duration<double> elapsed_seconds = end-start;
   // zMesh.assign((float)elapsed_seconds.count());
   //         }
  //      }

  //      SDL_Log("ADVANCE: %f",advance);
   //     final_pos.x += advance;
  //  }
  //  FT_Done_FreeType(library);
    //Freetype TEST
    
    //Create sphere.
    std::vector<GLfloat> vertices = std::vector<GLfloat>();
    std::vector<GLuint> indices = std::vector<GLuint>();
    PhysicsSystem::shapes.push_back(new btSphereShape(1.0f));
    gel::ShapeBuilder::buildSphere(vertices,indices,2,2,2,20,20);
    gel::Asset<gel::VertexReference> groundVertex, sphereVertex = assets.load<gel::VertexReference,gel::Vertex>(
            std::vector<gel::VertexSpec>{gel::POSITION,gel::TEXTURE_0},vertices,indices).assign(texShader);

    //Create player cube.
    std::vector<GLfloat> tempvertices = std::vector<GLfloat>();
    std::vector<GLuint> tempindices = std::vector<GLuint>();
    gel::ShapeBuilder::buildBox(tempvertices,tempindices,0.729f,0.826f,0.091f);
    /*int tempVerticesSize = tempvertices.size();
    for(int i = 0;i < tempVerticesSize;i += 5){
        tempvertices.push_back(tempvertices[i]);
        tempvertices.push_back(tempvertices[i+1]);
        tempvertices.push_back(tempvertices[i+2] + 3.0f * (0.826f * 0.5f));
        tempvertices.push_back(tempvertices[i+3]);
        tempvertices.push_back(tempvertices[i+4]);
    }
    int tempIndicesSize = tempindices.size();
    for(int i = 0;i < tempIndicesSize;i++)tempindices.push_back(8+tempindices[i]);*/
    groundVertex = assets.load<gel::VertexReference,gel::Vertex>(std::vector<gel::VertexSpec>{gel::POSITION,gel::TEXTURE_0},tempvertices,tempindices)
                        .assign(altShader);
    gel::Asset<gel::Mesh> mesh = assets.load<gel::Mesh,gel::Mesh>("main",glm::mat4())
        .assign(glm::mat4()).assign(glm::vec4(1.0f,1.0f,1.0f,1.0f))
        .assign(altShader).assign(groundVertex);
    meshes.push_back(mesh);

    //ASSIMP TEST
   // importModel("assets/model/quadcopter.blend"/*jeep1.fbx*//*dwarf.x*//*quadcopter.blend*/,assets,texShader,glm::mat4(),meshes);
    btMatrix3x3 rotateShapeMatrix;rotateShapeMatrix.setIdentity();
    //PhysicsSystem::shapes.push_back(new btCompoundShape());
    //TODO: 9 shapes: 4x small cylinders for each motor (allow upward impulse at each one), 1x big cylinder for main body, 4x rectangles for arms
   //     ((btCompoundShape)PhysicsSystem::shapes.back()).addChildShape(btTransform(rotateShapeMatrix,btVector3(0,0,0)),new btBoxShape(btVector3(0.729f * 0.5f,0.826f * 0.5f,0.091f * 0.5f)));
   // ((btCompoundShape)PhysicsSystem::shapes.back()).addChildShape(btTransform(rotateShapeMatrix,btVector3(0,0,3)),new btBoxShape(btVector3(0.729f * 0.5f,0.826f * 0.5f,0.091f * 0.5f)));
    /*PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();
    PhysicsSystem::shapes.back()->addChildShape();*/
    PhysicsSystem::shapes.push_back(new /*btCylinderShape*/btBoxShape(btVector3(0.729f * 0.5f,0.826f * 0.5f,0.091f * 0.5f)));
    for(gel::Asset<gel::Mesh> mesh:meshes){
        std::string strfind = mesh.component<gel::Mesh>()->name;
        if(strfind.find("main") != std::string::npos){
           mesh.assign(RigidBody(std::string("main"),1.0f,btVector3(0,0,0),PhysicsSystem::shapes[PhysicsSystem::shapes.size()-1]));
        }
       // SDL_Log("MESH NAME: %s",mesh.component<gel::Mesh>()->name.c_str());
    }
    //ASSIMP TEST
    
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
                        .assign(altShader);
        
        //Build ground entity.
        PhysicsSystem::shapes.push_back(new btBoxShape(btVector3(obstruction[3] / 2.0f,obstruction[4] / 2.0f,obstruction[5] / 2.0f)));
        glm::mat4 transform = glm::translate(glm::mat4(),glm::vec3(obstruction[0],obstruction[1],obstruction[2]));
        /*assets.load<gel::Mesh>()
            .assign(transform).assign(glm::vec4(((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,1.0f))
            .assign(RigidBody(std::string("test"),0.0f,btVector3(obstruction[0],obstruction[1],obstruction[2]),
                PhysicsSystem::shapes[PhysicsSystem::shapes.size()-1]))
            .assign(altShader).assign(groundVertex).assign(sphereTexture);*/
    }
    
    //Load spheres.
    std::stringstream sstream(fileToString("assets/cube-drop.txt"));
    int count = 0;
    float x,y,z;
    sstream>>count;

    while(!sstream.eof()){
        sstream>>x>>y>>z;
        glm::mat4 transform = glm::translate(glm::mat4(),glm::vec3(x,y,z));
       /* assets.load<gel::Mesh>()
            .assign(transform)
            .assign(glm::vec4(((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,1.0f))
            .assign(RigidBody(std::string("test"),10.0f,btVector3(x,y,z),PhysicsSystem::shapes[0]))
            .assign(texShader).assign(sphereVertex).assign(sphereTexture);*/
    }

    //Build UI elements.
    //UIBuilder::addText("Test UI Element",glm::vec2(0.0f,0.0f),glm::vec2(RenderSystem::cam.width*0.25f,RenderSystem::cam.height*0.15f),sans_reg_16,texShader,assets);
    return true;
 }

 void resize(int width, int height){
    RenderSystem::cam.setAspect((float)width,(float)height);
}

void render(){
    float boost = 0.0f,x_dir = 0.0f,z_dir = 0.0f;
    if(keyMap[SDLK_a]){
        SDL_Log("A PRESSED!");
        boost = 1.0f;
    }
    if(keyMap[SDLK_UP]){
        SDL_Log("UP PRESSED!");
        z_dir = -1.0f;
    }
    if(keyMap[SDLK_DOWN]){
        SDL_Log("DOWN PRESSED!");
        z_dir = 1.0f;
    }
    if(keyMap[SDLK_RIGHT]){
        SDL_Log("RIGHT PRESSED!");
        x_dir = 1.0f;
    }
    if(keyMap[SDLK_LEFT]){
        SDL_Log("LEFT PRESSED!");
        x_dir = -1.0f;
    }
    
    for(gel::Asset<gel::Mesh> mesh:meshes){
        std::string strfind = mesh.component<gel::Mesh>()->name;
        if(strfind.find("main") != std::string::npos){
            btTransform trans;
            mesh.component<RigidBody>()->body->getMotionState()->getWorldTransform(trans);
            if(keyMap[SDLK_r]){
                mesh.component<RigidBody>().remove();
                mesh.assign(RigidBody(std::string("main"),1.0f,btVector3(0,0,0),PhysicsSystem::shapes[1]));
                boost = 0.0f;
            }else{
               // SDL_Log("APPLY FORCE");
                mesh.component<RigidBody>()->body->activate(true);
                mesh.component<RigidBody>()->body->applyCentralForce(btVector3(x_dir * 0.2f * (float)dt,boost * 9.0f * (float)dt,z_dir * 0.2f * (float)dt));
            }
            mesh.component<RigidBody>()->body->getMotionState()->getWorldTransform(trans);
            /*SDL_Log("MOTION STATE: trans(%f,%f,%f) and rot(%f,%f,%f,%f,\n%f,%f,%f,%f,\n%f,%f,%f,%f,\n%f,%f,%f,%f)",
                float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()),
                float(trans.getBasis()[0][0]),float(trans.getBasis()[0][1]),float(trans.getBasis()[0][2]),float(trans.getBasis()[0][3]),
                float(trans.getBasis()[1][0]),float(trans.getBasis()[1][1]),float(trans.getBasis()[1][2]),float(trans.getBasis()[1][3]),
                float(trans.getBasis()[2][0]),float(trans.getBasis()[2][1]),float(trans.getBasis()[2][2]),float(trans.getBasis()[2][3]),
                float(trans.getBasis()[3][0]),float(trans.getBasis()[3][1]),float(trans.getBasis()[3][2]),float(trans.getBasis()[3][3]));*/
        }
    }
   /* auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    entityx::ComponentHandle<float> timeRef = zMesh.component<float>();
    *timeRef = elapsed_seconds.count();*/
        DefaultAppListener::render();
for(gel::Asset<gel::Mesh> mesh:meshes){
                    std::string strfind = mesh.component<gel::Mesh>()->name;
                    if(strfind.find("main") != std::string::npos){
                        /**TEST APPLY FORCE**/
                        //SDL_Log("APPLY FORCE: %f",33.0f * (float)dt);
                        //mesh.component<RigidBody>()->body->activate(true);
                        //mesh.component<RigidBody>()->body->applyCentralForce(btVector3(0.0f,140.0f/*33.0f * (float)dt*/,0.0f));
                        /**TEST APPLY FORCE**/
                        /*btTransform trans;
                        mesh.component<RigidBody>()->body->getMotionState()->getWorldTransform(trans);
                        SDL_Log("MOTION STATE: trans(%f,%f,%f) and rot(%f,%f,%f,%f,\n%f,%f,%f,%f,\n%f,%f,%f,%f,\n%f,%f,%f,%f)",
                                float(trans.getOrigin().getX()),float(trans.getOrigin().getY()),float(trans.getOrigin().getZ()),
                            float(trans.getBasis()[0][0]),float(trans.getBasis()[0][1]),float(trans.getBasis()[0][2]),float(trans.getBasis()[0][3]),
                            float(trans.getBasis()[1][0]),float(trans.getBasis()[1][1]),float(trans.getBasis()[1][2]),float(trans.getBasis()[1][3]),
                            float(trans.getBasis()[2][0]),float(trans.getBasis()[2][1]),float(trans.getBasis()[2][2]),float(trans.getBasis()[2][3]),
                            float(trans.getBasis()[3][0]),float(trans.getBasis()[3][1]),float(trans.getBasis()[3][2]),float(trans.getBasis()[3][3]));*/
                    }
                }

    //glTF TEST
    glViewport(0,0,640,480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    altShader.component<gel::ShaderProgram>()->begin();
    altShader.component<gel::ShaderProgram>()->setAttribute("a_color",std::vector<GLfloat>{1.0f,1.0f,1.0f,1.0f});
    altShader.component<gel::ShaderProgram>()->setUniform("u_projView",glm::ortho(0.0f,1.0f,1.0f,0.0f,0.1f,100.0f) * glm::translate(glm::vec3(0,0,-1)),false);
            
    renderModel(model);
    altShader.component<gel::ShaderProgram>()->end();
    //glTF TEST
}

 void pause(){}
 void resume(){}
};
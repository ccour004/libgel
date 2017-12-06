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

#include <vector>
#include <entityx/entityx.h>
#include "json.hpp"

namespace gel{
    //TODO: fill out all of these structs w/ params based on glTF spec
    struct buffer{
        buffer(){}
        buffer(std::string uri,int byteLength):uri(uri),byteLength(byteLength){}
        int byteLength;
        std::string uri;

        //Extra:
        std::vector<unsigned char> data;
    };
    struct bufferView{
        bufferView():byteStride(0){}
        bufferView(int buffer,int byteOffset,int byteLength,int target,int byteStride = 0):buffer(buffer),byteOffset(byteOffset),
            byteLength(byteLength),byteStride(byteStride){}
        int buffer,target,byteLength,byteOffset,byteStride;

        //Extra:
        unsigned int bufferReference;
    };
    struct accessor{
        accessor(){}
        accessor(int bufferView,int byteOffset,int componentType,int count,std::string type):
            bufferView(bufferView),byteOffset(byteOffset),componentType(componentType),count(count),type(type){}
        int bufferView,componentType,count,byteOffset;
        std::string type;
    };
    struct primitive{
        primitive():indices(-1){}
        primitive(std::map<std::string,int> attributes,int indices = -1):attributes(attributes),indices(indices){}
        std::map<std::string,int> attributes;
        int indices;
        //Extra:
        unsigned int bufferReference;
    };
    struct mesh{
        mesh(){}
        mesh(std::vector<gel::primitive> primitives):primitives(primitives){}
        std::vector<gel::primitive> primitives;
    };
    struct node{
        int mesh;
    };
    struct scene{
        std::vector<int> nodes;
    };
    struct model{
        std::vector<gel::scene> scenes;
        std::vector<gel::node> nodes;
        std::vector<gel::mesh> meshes;
        std::vector<gel::buffer> buffers;
        std::vector<gel::bufferView> bufferViews;
        std::vector<gel::accessor> accessors;
    };

    void from_json(const nlohmann::json& j, gel::scene& scene){
        scene.nodes = j.at("nodes").get<std::vector<int>>();
    }

    void from_json(const nlohmann::json& j, gel::bufferView& bufferView){
        bufferView.buffer = j.at("buffer").get<int>();
        bufferView.byteOffset = j.at("byteOffset").get<int>();
        bufferView.byteLength = j.at("byteLength").get<int>();
        bufferView.target = j.at("target").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::buffer& buffer){
        buffer.uri = "assets/"+j.at("uri").get<std::string>();
        buffer.byteLength = j.at("byteLength").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::primitive& primitive){
        primitive.attributes = j.at("attributes").get<std::map<std::string,int>>();
        if(j.find("indices") != j.end()) primitive.indices = j.at("indices").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::mesh& mesh){
        mesh.primitives = j.at("primitives").get<std::vector<gel::primitive>>();
    }

    void from_json(const nlohmann::json& j, gel::accessor& accessor){
        accessor.bufferView = j.at("bufferView").get<int>();
        accessor.byteOffset = j.at("byteOffset").get<int>();
        accessor.componentType = j.at("componentType").get<int>();
        accessor.count = j.at("count").get<int>();
        accessor.type = j.at("type").get<std::string>();
    }

    void from_json(const nlohmann::json& j, gel::model& model){
        model.scenes = j.at("scenes").get<std::vector<gel::scene>>();
        model.buffers = j.at("buffers").get<std::vector<gel::buffer>>();
        model.bufferViews = j.at("bufferViews").get<std::vector<gel::bufferView>>();
        model.accessors = j.at("accessors").get<std::vector<gel::accessor>>();
        model.meshes = j.at("meshes").get<std::vector<gel::mesh>>();
    }
}

std::string loadFileAsString(std::string file){
    SDL_RWops *io = SDL_RWFromFile(file.c_str(), "r");
    std::vector<char> buffer;
    if (io != NULL) {
        Sint64 size = SDL_RWsize(io),total = 0,result = 1;
        char data[size];
        while(total < size && result != 0){
            result = SDL_RWread(io, data,1,size-total);
            for(int i = 0;i < result;i++) buffer.push_back(data[i]);
            total += result;
        }
        SDL_RWclose(io);
    }else{SDL_Log("---Failed to load buffer at: %s",file.c_str());}  
    return std::string(buffer.begin(),buffer.end());  
}

void fillWithJSON(gel::model& model){
    model = nlohmann::json::parse(loadFileAsString("assets/Triangle.gltf"));
}

void loadBuffer(gel::buffer& buffer){
    SDL_RWops *io = SDL_RWFromFile(buffer.uri.c_str(), "rb");
    if (io != NULL) {
        Sint64 size = SDL_RWsize(io),total = 0,result = 1;
        char data[buffer.byteLength];
        while(total < size && result != 0){
            result = SDL_RWread(io, data,1,size-total);
            for(int i = 0;i < result;i++) buffer.data.push_back(data[i]);
            SDL_Log("PUSHED %i bytes, buffer.data.size is: %i",result,buffer.data.size());
            total += result;
        }
        SDL_Log("[buffer] Expected %i bytes,read %i bytes from buffer at %s",buffer.byteLength,total,buffer.uri.c_str());
        SDL_RWclose(io);
    }else{SDL_Log("---Failed to load buffer at: %s",buffer.uri.c_str());}
}

void loadBufferView(gel::bufferView& view,const std::vector<gel::buffer>& buffers){
    GLenum target = view.target == 34963? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    glGenBuffers(1,&view.bufferReference);
    glBindBuffer(target,view.bufferReference);
    SDL_Log("BUFFERS SIZE: %i, BUFFERS[0].DATA.SIZE: %i",buffers.size(),buffers[0].data.size());
    for(int i = 0;i < buffers[0].data.size();i++) SDL_Log("BUFFER DATA BYTE %i is: %i",i,buffers[0].data[i]);; 
    glBufferData(target,view.byteLength,buffers[view.buffer].data.data()+view.byteOffset,GL_STATIC_DRAW);
    SDL_Log("[bufferView] %s loaded with %i bytes",view.target == 34963? "GL_ELEMENT_ARRAY_BUFFER" : "GL_ARRAY_BUFFER",view.byteLength);
}

GLenum getComponentType(int type){
    switch(type){
        case 5121: return GL_UNSIGNED_BYTE;
        case 5122: return GL_SHORT;
        case 5123: return GL_UNSIGNED_SHORT;
        case 5125: return GL_UNSIGNED_INT;
        case 5126: return GL_FLOAT;
    }
    return GL_BYTE;    
}

int getTypeSize(std::string type){
    if(type == "VEC2") return 2;
    if(type == "VEC3") return 3;
    if(type == "VEC4" || type == "MAT2") return 4;
    if(type == "MAT3") return 9;
    if(type == "MAT4") return 16;
}

void loadAccessor(const gel::accessor& accessor,const gel::bufferView& bufferView,unsigned int location){
    GLenum type = getComponentType(accessor.componentType);
    int size = getTypeSize(accessor.type);
    SDL_Log("[accessor] at %i, loading size: %i with type: %i at offset: %i",location,size,type,accessor.byteOffset);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location,size,type,false,bufferView.byteStride,BUFFER_OFFSET(accessor.byteOffset));
}

void loadPrimitive(gel::primitive& primitive,const std::vector<gel::bufferView>& bufferViews,std::vector<gel::accessor>& accessors,
        std::map<std::string,unsigned int>& locations){
    //Start vertex array for holding vbo/ibo state.
    glGenVertexArrays(1,&primitive.bufferReference);
    glBindVertexArray(primitive.bufferReference);

    //Load all vertex attributes.
    for(std::pair<std::string,int> attribute:primitive.attributes){
        gel::accessor accessor = accessors[attribute.second];
        gel::bufferView view = bufferViews[accessor.bufferView];
        glBindBuffer(GL_ARRAY_BUFFER,view.bufferReference);
        loadAccessor(accessor,view,locations[attribute.first]);
    }

    //Load index buffer if applicable.
    if(primitive.indices != -1){
        gel::bufferView view = bufferViews[primitive.indices];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,view.bufferReference);
    }
}

void loadMesh(gel::mesh& mesh,const std::vector<gel::bufferView>& bufferViews,std::vector<gel::accessor>& accessors,
        std::map<std::string,unsigned int>& locations){
    for(int i = 0;i < mesh.primitives.size();i++) loadPrimitive(mesh.primitives[i],bufferViews,accessors,locations);
}

void loadScene(gel::scene& scene){

}

void loadModel(gel::model& model,std::map<std::string,unsigned int>& locations){
    SDL_Log("+++LOADING BUFFERS...");
    for(int i = 0;i < model.buffers.size();i++) loadBuffer(model.buffers[i]);
    SDL_Log("+++LOADING BUFFER VIEWS...");
    for(int i = 0;i < model.bufferViews.size();i++) loadBufferView(model.bufferViews[i],model.buffers);
    SDL_Log("+++LOADING MESHES...");
    for(int i = 0;i < model.meshes.size();i++) loadMesh(model.meshes[i],model.bufferViews,model.accessors,locations);
    SDL_Log("+++LOADING SCENE...");
    for(int i = 0;i < model.scenes.size();i++) loadScene(model.scenes[i]);
}

void renderModel(gel::model& model){
    for(gel::mesh mesh:model.meshes)
    for(gel::primitive primitive:mesh.primitives){
        glBindVertexArray(primitive.bufferReference);
        if(primitive.indices != -1) 
            glDrawElements(GL_TRIANGLES,model.accessors[primitive.indices].count,getComponentType(model.accessors[primitive.indices].componentType),0);
        else glDrawArrays(GL_TRIANGLES,0,model.accessors[primitive.attributes["POSITION"]].count);
    }
}
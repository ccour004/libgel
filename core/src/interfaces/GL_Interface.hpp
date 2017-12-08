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
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include "json.hpp"

namespace gel{
    //TODO: fill out all of these structs w/ params based on glTF spec
    struct buffer{
        buffer(){}
        buffer(std::string uri,int byteLength):uri(uri),byteLength(byteLength){}
        int byteLength;
        std::string uri;

        //Extra:
        std::vector<uint8_t> data;
    };
    struct bufferView{
        bufferView():byteStride(0){}
        bufferView(int buffer,int byteOffset,int byteLength,int target,int byteStride = 0):buffer(buffer),byteOffset(byteOffset),
            byteLength(byteLength),byteStride(byteStride){}
        int buffer,target,byteLength,byteOffset = 0,byteStride = 0;

        //Extra:
        unsigned int bufferReference;
    };
    struct accessor{
        accessor(){}
        accessor(int bufferView,int byteOffset,int componentType,int count,std::string type):
            bufferView(bufferView),byteOffset(byteOffset),componentType(componentType),count(count),type(type){}
        int bufferView,componentType,count,byteOffset = 0;
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
        node(){}
        int mesh = -1;
        glm::mat4 matrix;
        glm::vec4 rotation = glm::vec4(0,0,0,1);
        glm::vec3 translation = glm::vec3(0,0,0),scale = glm::vec3(1,1,1);
        std::vector<int> children;
    };
    struct sampler{
        sampler(){}
        int input,output;
        std::string interpolation;
    };
    struct channel_target{
        channel_target(){}
        int node;
        std::string path;
    };
    struct channel{
        channel(){}
        int sampler;
        gel::channel_target target; 
    };
    struct animation{
        animation(){}
        std::vector<gel::sampler> samplers;
        std::vector<gel::channel> channels;
    };
    struct scene{
        scene(){}
        std::vector<int> nodes;
    };
    struct model{
        model(){}
        std::vector<gel::scene> scenes;
        std::vector<gel::node> nodes;
        std::vector<gel::mesh> meshes;
        std::vector<gel::animation> animations;
        std::vector<gel::buffer> buffers;
        std::vector<gel::bufferView> bufferViews;
        std::vector<gel::accessor> accessors;
    };

    void from_json(const nlohmann::json& j, gel::channel_target& target){
        target.node = j.at("node").get<int>();
        target.path = j.at("path").get<std::string>();
    }

    void from_json(const nlohmann::json& j, gel::channel& channel){
        channel.sampler = j.at("sampler").get<int>();
        channel.target = j.at("target").get<gel::channel_target>();
    }

    void from_json(const nlohmann::json& j, gel::sampler& sampler){
        sampler.input = j.at("input").get<int>();
        sampler.output = j.at("output").get<int>();
        sampler.interpolation = j.at("interpolation").get<std::string>();
    }

    void from_json(const nlohmann::json& j, gel::animation& animation){
        animation.samplers = j.at("samplers").get<std::vector<gel::sampler>>();
        animation.channels = j.at("channels").get<std::vector<gel::channel>>();
    }

    void from_json(const nlohmann::json& j, gel::node& node){
        if(j.find("mesh") != j.end()){
            node.mesh = j.at("mesh").get<int>();
        }
        if(j.find("translation") != j.end()){
            std::vector<float> translation = j.at("translation").get<std::vector<float>>();
            //node.transform *= glm::translate(glm::vec3(translation[0],translation[1],translation[2]));
            node.translation = glm::vec3(translation[0],translation[1],translation[2]);
        }
        if(j.find("rotation") != j.end()){
            std::vector<float> rotation = j.at("rotation").get<std::vector<float>>();
            //node.transform *= glm::mat4_cast(glm::quat(rotation[3],rotation[0],rotation[1],rotation[2]));
            node.rotation = glm::vec4(rotation[0],rotation[1],rotation[2],rotation[3]);
        }
        if(j.find("scale") != j.end()){
            std::vector<float> scale = j.at("scale").get<std::vector<float>>();
            //node.scale *= glm::scale(glm::vec3(scale[0],scale[1],scale[2]));        
            node.scale = glm::vec3(scale[0],scale[1],scale[2]);
        }
        if(j.find("matrix") != j.end()){
            std::vector<float> matrix = j.at("matrix").get<std::vector<float>>();
            node.matrix = glm::transpose(glm::mat4(matrix[0],matrix[1],matrix[2],matrix[3],matrix[4],matrix[5],matrix[6],matrix[7],
                matrix[8],matrix[9],matrix[10],matrix[11],matrix[12],matrix[13],matrix[14],matrix[15]));
            }
        if(j.find("children") != j.end()){
            node.children = j.at("children").get<std::vector<int>>();
        }
    }

    void from_json(const nlohmann::json& j, gel::scene& scene){
        scene.nodes = j.at("nodes").get<std::vector<int>>();
    }

    void from_json(const nlohmann::json& j, gel::bufferView& bufferView){
        bufferView.buffer = j.at("buffer").get<int>();
        if(j.find("byteOffset") != j.end()) bufferView.byteOffset = j.at("byteOffset").get<int>();
        bufferView.byteLength = j.at("byteLength").get<int>();
        if(j.find("target") != j.end()) bufferView.target = j.at("target").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::buffer& buffer){
        buffer.uri = j.at("uri").get<std::string>();
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
        if(j.find("byteOffset") != j.end()) accessor.byteOffset = j.at("byteOffset").get<int>();
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
        if(j.find("animations") != j.end()) model.animations = j.at("animations").get<std::vector<gel::animation>>();
        model.nodes = j.at("nodes").get<std::vector<gel::node>>();
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

void fillWithJSON(gel::model& model,std::string filename){
    model = nlohmann::json::parse(loadFileAsString(filename));
}

void loadBuffer(std::string path,gel::buffer& buffer){
    SDL_RWops *io = SDL_RWFromFile((path+buffer.uri).c_str(), "rb");
    if (io != NULL) {
        Sint64 size = SDL_RWsize(io),total = 0,result = 1;
        char data[buffer.byteLength];
        while(total < size && result != 0){
            result = SDL_RWread(io, data,1,size-total);
            for(int i = 0;i < result;i++) buffer.data.push_back(data[i]);
            SDL_Log("PUSHED %li bytes, buffer.data.size is: %lu",result,buffer.data.size());
            total += result;
        }
        SDL_Log("[buffer] Expected %i bytes,read %li bytes from buffer at %s",buffer.byteLength,total,buffer.uri.c_str());
        SDL_RWclose(io);
    }else{SDL_Log("---Failed to load buffer at: %s",buffer.uri.c_str());}
}

void loadBufferView(gel::bufferView& view,const std::vector<gel::buffer>& buffers){
    GLenum target = view.target == 34963? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
    glGenBuffers(1,&view.bufferReference);
    glBindBuffer(target,view.bufferReference);
    SDL_Log("BUFFERS SIZE: %lu, BUFFERS[0].DATA.SIZE: %lu",buffers.size(),buffers[0].data.size());
    //for(int i = 0;i < buffers[0].data.size();i++) SDL_Log("BUFFER DATA BYTE %i is: %i",i,buffers[0].data[i]);; 
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
    if(type == "SCALAR") return 1;
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
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferViews[accessors[primitive.indices].bufferView].bufferReference);
    }
}

void loadMesh(gel::mesh& mesh,const std::vector<gel::bufferView>& bufferViews,std::vector<gel::accessor>& accessors,
        std::map<std::string,unsigned int>& locations){
    for(int i = 0;i < mesh.primitives.size();i++) loadPrimitive(mesh.primitives[i],bufferViews,accessors,locations);
}

const void* getBufferPointer(const gel::bufferView& view,int offset,const std::vector<gel::buffer>& buffers){
    return buffers[view.buffer].data.data()+view.byteOffset+offset;
}

std::vector<float> getBufferValue(const float* bufferPointer,int index,int size){
    std::vector<float> values;
    SDL_Log("INDEX: %i,SIZE: %i",index,size);
    for(int i = 0;i < size;i++) values.push_back(*(bufferPointer + index * size + i));
    return values;
}

float getLinearBoundingValues(float currentVal,float& previous,float& next,int& prevIndex,int& nextIndex,
    const gel::accessor& accessor,const std::vector<gel::bufferView>& bufferViews,const std::vector<gel::buffer>& buffers){
    //NOTE: Accessor's component type MUST be GL_FLOAT for this to work!!!
    const float* bufferPointer = (const float*)getBufferPointer(bufferViews[accessor.bufferView],accessor.byteOffset,buffers);
    prevIndex = 0;nextIndex = 0;

    //Compute max value, and modulate currentVal based on it.
    float maxVal = *(bufferPointer + (accessor.count-1));
    currentVal = std::fmod(currentVal,maxVal);

    for(int i = 0;i < accessor.count;i++){
        float thisVal = *(bufferPointer+i);
        //for(int j = 0;j < getTypeSize(accessor.type);j++) thisVal.push_back(*bufferPointer++);
        if(currentVal > thisVal){
            previous = thisVal; prevIndex = i;
        }else{
            next = thisVal; nextIndex = i; return currentVal;
        }
    }
    return currentVal;
}

void loadModel(gel::model& model,std::string path,std::map<std::string,unsigned int>& locations){
    SDL_Log("+++LOADING BUFFERS...");
    for(int i = 0;i < model.buffers.size();i++) loadBuffer(path,model.buffers[i]);
    SDL_Log("+++LOADING BUFFER VIEWS...");
    for(int i = 0;i < model.bufferViews.size();i++) loadBufferView(model.bufferViews[i],model.buffers);
    SDL_Log("+++LOADING MESHES...");
    for(int i = 0;i < model.meshes.size();i++) loadMesh(model.meshes[i],model.bufferViews,model.accessors,locations);
}

void renderMesh(gel::mesh& mesh,gel::ShaderProgram& shader,gel::model& model){
    SDL_Log(">>>RENDER MESH");
    for(gel::primitive primitive:mesh.primitives){
        shader.setAttribute("a_color",
            std::vector<GLfloat>{/*((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX,((float) rand()) / (float) RAND_MAX*/1.0f,1.0f,1.0f,1.0f});
        glBindVertexArray(primitive.bufferReference);
        if(primitive.indices != -1){
            SDL_Log(">>>GL_DRAW_ELEMENTS [INDICES: %i,COMPONENT TYPE: %i]",
                model.accessors[primitive.indices].count,getComponentType(model.accessors[primitive.indices].componentType));
            glDrawElements(GL_TRIANGLES,model.accessors[primitive.indices].count,getComponentType(model.accessors[primitive.indices].componentType),
                (void *)(intptr_t)model.accessors[primitive.indices].byteOffset);      
        }
        else glDrawArrays(GL_TRIANGLES,0,model.accessors[primitive.attributes["POSITION"]].count);
    }
}

glm::vec4 getSamplerValue(float curr,std::string path,gel::sampler& sampler,gel::model& model){
    glm::vec4 sampleValue;
    if(sampler.interpolation != "LINEAR"){SDL_Log("!!!UNSUPPORTED SAMPLER INTERPOLATION: %s",sampler.interpolation.c_str());return sampleValue;}
    float prev = 0,next = 0; int prevIndex = 0,nextIndex = 0;
    curr = getLinearBoundingValues(curr,prev,next,prevIndex,nextIndex,model.accessors[sampler.input],model.bufferViews,model.buffers);
    SDL_Log("CURRENT VAL: %f, BOUNDS: (%f,%f) or INDEX(%i,%i)",curr,prev,next,prevIndex,nextIndex);
    float interpolator = (curr - prev) / (next - prev);
    const float* output_ptr = (const float*)getBufferPointer(model.bufferViews[model.accessors[sampler.output].bufferView],model.accessors[sampler.output].byteOffset,model.buffers);
    std::vector<float> outputPrevious = getBufferValue(output_ptr,prevIndex,getTypeSize(model.accessors[sampler.output].type)),
        outputNext = getBufferValue(output_ptr,nextIndex,getTypeSize(model.accessors[sampler.output].type));
    SDL_Log("OUTPUT PREV: %f,%f,%f,%f and OUTPUT NEXT: %f,%f,%f,%f",outputPrevious[0],outputPrevious[1],outputPrevious[2],outputPrevious[3],
        outputNext[0],outputNext[1],outputNext[2],outputNext[3]);
    if(path == "rotation"){
        glm::quat slerped = glm::slerp(glm::quat(outputPrevious[3],outputPrevious[0],outputPrevious[1],outputPrevious[2]),
            glm::quat(outputNext[3],outputNext[0],outputNext[1],outputNext[2]),interpolator);
        return glm::vec4(slerped.x,slerped.y,slerped.z,slerped.w);        
    }else{
        return glm::lerp(glm::vec4(outputPrevious[0],outputPrevious[1],outputPrevious[2],outputPrevious[3]),
            glm::vec4(outputNext[0],outputNext[1],outputNext[2],outputNext[3]),interpolator);
    }
}

void renderChannel(float curr,gel::animation& animation,int i,gel::model& model){
    gel::channel channel = animation.channels[i];
    glm::vec4 sampleValue = getSamplerValue(curr,channel.target.path,animation.samplers[channel.sampler],model);
    SDL_Log("SAMPLE VALUE: %f,%f,%f,%f",sampleValue[0],sampleValue[1],sampleValue[2],sampleValue[3]);
    if(channel.target.path == "rotation") model.nodes[channel.target.node].rotation = sampleValue;
    else if(channel.target.path == "translation") model.nodes[channel.target.node].translation = sampleValue;
    else if(channel.target.path == "scale") model.nodes[channel.target.node].scale = sampleValue;
    //TODO: add 'weights' target
}

float curr = 0.0f;
void renderNode(gel::model& model,gel::ShaderProgram& shader,gel::node& node,float zoom,glm::mat4 parent){
    parent = parent * node.matrix * glm::translate(node.translation) 
        * glm::mat4_cast(glm::quat(node.rotation[3],node.rotation[0],node.rotation[1],node.rotation[2])) 
        * glm::scale(node.scale);
    shader.setUniform("u_projView",
        /*glm::ortho(0.0f,5.0f,5.0f,0.0f,0.1f,100.0f)*/glm::perspective(glm::radians(zoom),640.0f/480.0f,0.1f,100.0f)
        * glm::translate(glm::vec3(0,0,-10)) * parent,false);
    if(node.mesh != -1)
         renderMesh(model.meshes[node.mesh],shader,model);
    //Render all child nodes, if any.
    for(int i = 0;i < node.children.size();i++)
        renderNode(model,shader,model.nodes[node.children[i]],zoom,parent);
}

void renderModel(gel::model& model,gel::ShaderProgram& shader,float zoom){
    glm::mat4 parent;
   // parent *= glm::rotate(curr,glm::vec3(0,1,0));

    //Render any animation channels.
    for(int i = 0;i < model.animations.size();i++)
    renderChannel(curr,model.animations[i],0,model);
    curr += 0.01f;

    //Set shader values.
    shader.begin();

    //Render nodes.
    for(gel::scene scene:model.scenes)
    for(int i = 0;i < scene.nodes.size();i++)
        renderNode(model,shader,model.nodes[scene.nodes[i]],zoom,parent);
    shader.end();
}
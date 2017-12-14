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
#include <glm/gtc/type_ptr.hpp>
#include "json.hpp"

#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

namespace gel{
    #define FRAGMENT_SHADER 35632
    #define VERTEX_SHADER 35633
    struct camera;
    void renderCamera(camera& camera);
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
        int material = -1,mode = 4;
        //Extra:
        unsigned int bufferReference;
    };
    struct mesh{
        mesh(){}
        mesh(std::vector<gel::primitive> primitives):primitives(primitives){}
        std::vector<gel::primitive> primitives;
        //TODO: finish
        //Extra:
        bool isVisible = true;
    };
    struct node{
        node(){}
        int mesh = -1;
        glm::mat4 matrix;
        glm::vec4 rotation = glm::vec4(0,0,0,1);
        glm::vec3 translation = glm::vec3(0,0,0),scale = glm::vec3(1,1,1);
        std::vector<int> children;
    };
    struct orthographic{
        orthographic():xmag(1),ymag(1),zfar(100.0f),znear(0.1f){}
        orthographic(float xmag,float ymag,float znear,float zfar):xmag(xmag),ymag(ymag),zfar(zfar),znear(znear){}
        float xmag,ymag,zfar,znear;
    };
    struct perspective{
        perspective():aspectRatio(1.333f),yfov(glm::radians(45.0f)),zfar(-1),znear(0.1f){}
        perspective(float aspectRatio,float yfov,float znear,float zfar):aspectRatio(aspectRatio),yfov(glm::radians(yfov)),zfar(zfar),znear(znear){}
        float aspectRatio,yfov,zfar,znear;
    };
    struct camera{
        camera():type("perspective"){renderCamera(*this);}
        camera(gel::orthographic ortho):type("orthographic"),orthographic(ortho){renderCamera(*this);}
        camera(gel::perspective persp):type("perspective"),perspective(persp){renderCamera(*this);}
        void setAspectRatio(float width,float height){perspective.aspectRatio = width/height;glViewport(0,0,width,height);renderCamera(*this);}
        void setFov(float yfov){perspective.yfov = glm::radians(yfov);renderCamera(*this);}
        float getFov(){return glm::degrees(perspective.yfov);}
        void setXMag(float xmag){orthographic.xmag = xmag;renderCamera(*this);}
        void setYMag(float ymag){orthographic.ymag = ymag;renderCamera(*this);}
        void setZNear(float znear){if(type == "perspective")perspective.znear = znear;else orthographic.znear = znear;renderCamera(*this);}
        void setZFar(float zfar){if(type == "perspective")perspective.zfar = zfar;else orthographic.zfar = zfar;renderCamera(*this);}
        void setTranslate(glm::vec3 translate){this->translate = translate;renderCamera(*this);}
        void setRotate(float rotateX,float rotateY,float rotateZ){rotate = glm::quat(glm::vec3(rotateY,rotateX,rotateZ));renderCamera(*this);}
        std::string type;
        gel::perspective perspective;
        gel::orthographic orthographic;
        //Extra:
        glm::vec3 translate;
        glm::quat rotate;
        glm::mat4 transform;
    };
    void renderCamera(gel::camera& camera){
        if(camera.type == "perspective"){
            gel::perspective persp = camera.perspective;
            camera.transform = glm::perspective(persp.yfov,persp.aspectRatio,persp.znear,persp.zfar) * glm::translate(camera.translate) * glm::mat4_cast(camera.rotate);
        }else{
            gel::orthographic ortho = camera.orthographic;
            camera.transform = glm::ortho(-ortho.xmag,ortho.xmag,-ortho.ymag,ortho.ymag,ortho.znear,ortho.zfar) * glm::translate(camera.translate) * glm::mat4_cast(camera.rotate);
        }
    }
    struct animation_sampler{
        animation_sampler(){}
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
        std::vector<gel::animation_sampler> samplers;
        std::vector<gel::channel> channels;
    };
    struct scene{
        scene(){}
        std::vector<int> nodes;
    };
    struct textureInfo{
        int index = -1,texCoord;
    };
    struct pbrMetallicRoughness{
        pbrMetallicRoughness(){}
        std::vector<float> baseColorFactor = std::vector<float>{0.0f,0.0f,0.0f,1.0f};
        gel::textureInfo baseColorTexture,metallicRoughnessTexture;
        float metallicFactor = -1.0f,roughnessFactor;
    };
    struct pbrSpecularGlossiness{
        pbrSpecularGlossiness(){}
        std::vector<float> diffuseFactor = std::vector<float>{0.0f,0.0f,0.0f,1.0f},specularFactor;
        gel::textureInfo diffuseTexture,specularGlossinessTexture;
        int glossinessFactor = -1;
    };
    struct material{
        material(){}
        material(int technique):technique(technique){}
        gel::pbrMetallicRoughness pbrMetallicRoughness;
        /*gel::normalTextureInfo normalTexture;
        gel::occlusionTextureInfo occlusionTexture;
        gel::textureInfo emissiveTexture;
        std::vector<float> emissiveFactor;
        std::string alphaMode;
        float alphaCutoff;
        bool doubleSided;*/
        gel::pbrSpecularGlossiness pbrSpecularGlossiness;
        int technique;
    };
    struct shader{
        shader(){}
        shader(std::string uri,int type):uri(uri),type(type){}
        std::string uri;
        int type,bufferView;
        //Extra:
        unsigned int shaderReference;
    };
    struct program{
        program(){}
        program(int vertexShader,int fragmentShader,std::vector<std::string> attributes):vertexShader(vertexShader),fragmentShader(fragmentShader),attributes(attributes){}
        std::vector<std::string> attributes;
        int vertexShader,fragmentShader;
        //Extra:
        unsigned int programReference;
    };
    struct technique_parameters{
        technique_parameters(){}
        technique_parameters(int type):type(type){}
        technique_parameters(std::string semantic,int type):semantic(semantic),type(type){}
        int count,node,type;
        std::string semantic;
        //std::vector<void*> value; //Determined by type/count.
        //Extra:
        unsigned int parameterReference;
    };

    struct technique{
        technique(){}
        technique(std::map<std::string,std::string> attributes,std::map<std::string,std::string> uniforms,
            std::map<std::string,gel::technique_parameters> parameters,int program):attributes(attributes),uniforms(uniforms),parameters(parameters),program(program){}
        int program;
        std::map<std::string,gel::technique_parameters> parameters;
        std::map<std::string,std::string> attributes;
        std::map<std::string,std::string> uniforms;
        //gel::technique_states states;
    };

    struct texture{
        int sampler,source;
        //Extra:
        unsigned int textureReference;
    };

    struct sampler{
        int magFilter,minFilter,wrapS,wrapT;
    };

    struct image{
        std::string uri,mimeType;
        int bufferView;
    };

    struct model{
        model(){}
        void clear(){
            scenes.clear();
            nodes.clear();
            meshes.clear();
            animations.clear();
            buffers.clear();
            bufferViews.clear();
            accessors.clear();
            materials.clear();
            shaders.clear();
            textures.clear();
            images.clear();
            samplers.clear();
            programs.clear();
            techniques.clear();   
            glBindTexture(GL_TEXTURE_2D,0);     
        }
        std::vector<gel::scene> scenes;
        std::vector<gel::node> nodes;
        std::vector<gel::mesh> meshes;
        std::vector<gel::animation> animations;
        std::vector<gel::buffer> buffers;
        std::vector<gel::bufferView> bufferViews;
        std::vector<gel::accessor> accessors;
        std::vector<gel::material> materials;
        std::vector<gel::shader> shaders;
        std::vector<gel::texture> textures;
        std::vector<gel::image> images;
        std::vector<gel::sampler> samplers;
        std::vector<gel::program> programs;
        std::vector<gel::technique> techniques;
    };
    
    void from_json(const nlohmann::json& j, gel::texture& texture){
        if(j.find("sampler") != j.end()) texture.sampler = j.at("sampler").get<int>();
        if(j.find("source") != j.end()) texture.source = j.at("source").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::image& image){
        if(j.find("uri") != j.end()) image.uri = j.at("uri").get<std::string>();
        //TODO: finish this
    }

    void from_json(const nlohmann::json& j, gel::sampler& sampler){
        if(j.find("minFilter") != j.end()) sampler.minFilter = j.at("minFilter").get<int>();
        if(j.find("magFilter") != j.end()) sampler.magFilter = j.at("magFilter").get<int>();
        if(j.find("wrapS") != j.end()) sampler.wrapS = j.at("wrapS").get<int>();
        if(j.find("wrapT") != j.end()) sampler.wrapT = j.at("wrapT").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::technique_parameters& technique_parameters){
        if(j.find("type") != j.end()) technique_parameters.type = j.at("type").get<int>();
        if(j.find("count") != j.end()) technique_parameters.count = j.at("count").get<int>();
        if(j.find("node") != j.end()) technique_parameters.node = j.at("node").get<int>();
        if(j.find("semantic") != j.end()) technique_parameters.semantic = j.at("semantic").get<std::string>();
        //TODO: value
    }

    void from_json(const nlohmann::json& j, gel::technique& technique){
        technique.program = j.at("program").get<int>();
        if(j.find("attributes") != j.end()) technique.attributes = j.at("attributes").get<std::map<std::string,std::string>>();
        if(j.find("uniforms") != j.end()) technique.uniforms = j.at("uniforms").get<std::map<std::string,std::string>>();
        if(j.find("parameters") != j.end()) technique.parameters = j.at("parameters").get<std::map<std::string,gel::technique_parameters>>();
    }

    void from_json(const nlohmann::json& j, gel::program& program){
        if(j.find("attributes") != j.end()) program.attributes = j.at("attributes").get<std::vector<std::string>>();
        program.vertexShader = j.at("vertexShader").get<int>();
        program.fragmentShader = j.at("fragmentShader").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::shader& shader){
        if(j.find("uri") != j.end()) shader.uri = j.at("uri").get<std::string>();
        shader.type = j.at("type").get<int>();
        if(j.find("bufferView") != j.end()) shader.bufferView = j.at("bufferView").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::textureInfo& textureInfo){
        if(j.find("index") != j.end()) textureInfo.index = j.at("index").get<int>();
        if(j.find("texCoord") != j.end()) textureInfo.index = j.at("texCoord").get<int>();
    }

    void from_json(const nlohmann::json& j, gel::pbrMetallicRoughness& material){
        if(j.find("metallicFactor") != j.end()) material.metallicFactor = j.at("metallicFactor").get<float>();
        if(j.find("roughnessFactor") != j.end()) material.roughnessFactor = j.at("roughnessFactor").get<float>();
        if(j.find("baseColorFactor") != j.end()) material.baseColorFactor = j.at("baseColorFactor").get<std::vector<float>>();
        if(j.find("baseColorTexture") != j.end()) material.baseColorTexture = j.at("baseColorTexture").get<gel::textureInfo>();
        if(j.find("metallicRoughnessTexture") != j.end()) material.metallicRoughnessTexture = j.at("metallicRoughnessTexture").get<gel::textureInfo>();
        //TODO: fill out the rest
    } 
    
    void from_json(const nlohmann::json& j, gel::pbrSpecularGlossiness& material){
        if(j.find("diffuseTexture") != j.end()) material.diffuseTexture = j.at("diffuseTexture").get<gel::textureInfo>();
        if(j.find("specularGlossinessTexture") != j.end()) material.specularGlossinessTexture = j.at("specularGlossinessTexture").get<gel::textureInfo>();
        if(j.find("specularFactor") != j.end()) material.specularFactor = j.at("specularFactor").get<std::vector<float>>();
        if(j.find("diffuseFactor") != j.end()) material.diffuseFactor = j.at("diffuseFactor").get<std::vector<float>>();
        if(j.find("glossinessFactor") != j.end()) material.glossinessFactor = j.at("glossinessFactor").get<float>();
    }

    void from_json(const nlohmann::json& j, gel::material& material){
        if(j.find("pbrMetallicRoughness") != j.end()) material.pbrMetallicRoughness = j.at("pbrMetallicRoughness").get<gel::pbrMetallicRoughness>();
        if(j.find("extensions") != j.end() && j.at("extensions").find("KHR_materials_pbrSpecularGlossiness") != j.at("extensions").end()){
            material.pbrSpecularGlossiness = j.at("extensions").at("KHR_materials_pbrSpecularGlossiness").get<gel::pbrSpecularGlossiness>();
        }
        if(j.find("technique") != j.end()) material.technique = j.at("technique").get<int>(); else material.technique = 0;
        //TODO: fill out the rest
    }    

    void from_json(const nlohmann::json& j, gel::channel_target& target){
        target.node = j.at("node").get<int>();
        target.path = j.at("path").get<std::string>();
    }

    void from_json(const nlohmann::json& j, gel::channel& channel){
        channel.sampler = j.at("sampler").get<int>();
        channel.target = j.at("target").get<gel::channel_target>();
    }

    void from_json(const nlohmann::json& j, gel::animation_sampler& sampler){
        sampler.input = j.at("input").get<int>();
        sampler.output = j.at("output").get<int>();
        sampler.interpolation = j.at("interpolation").get<std::string>();
    }

    void from_json(const nlohmann::json& j, gel::animation& animation){
        animation.samplers = j.at("samplers").get<std::vector<gel::animation_sampler>>();
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
        if(j.find("material") != j.end()) primitive.material = j.at("material").get<int>(); else primitive.material = 0;
        //TODO: finish
    }

    void from_json(const nlohmann::json& j, gel::mesh& mesh){
        mesh.primitives = j.at("primitives").get<std::vector<gel::primitive>>();
        //TODO: finish
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
        if(j.find("materials") != j.end()) model.materials = j.at("materials").get<std::vector<gel::material>>();
        if(j.find("animations") != j.end()) model.animations = j.at("animations").get<std::vector<gel::animation>>();
        if(j.find("shaders") != j.end()) model.shaders = j.at("shaders").get<std::vector<gel::shader>>();
        model.nodes = j.at("nodes").get<std::vector<gel::node>>();
        if(j.find("images") != j.end()) model.images = j.at("images").get<std::vector<gel::image>>();
        if(j.find("samplers") != j.end()) model.samplers = j.at("samplers").get<std::vector<gel::sampler>>();
        if(j.find("textures") != j.end()) model.textures = j.at("textures").get<std::vector<gel::texture>>();

        //Default shader/program/technique values in case they're missing from the model (they're extensions["KHR_technique_webgl"] atm, anyhow):
        model.materials.push_back(gel::material(0));
        for(gel::mesh mesh:model.meshes)
        for(gel::primitive primitive:mesh.primitives){
            if(primitive.material == -1)
                primitive.material = 0;
        }
        model.shaders.push_back(gel::shader("default.vert",VERTEX_SHADER));
        model.shaders.push_back(gel::shader("default.frag",FRAGMENT_SHADER));
        model.programs.push_back(gel::program(model.shaders.size()-2,model.shaders.size()-1,std::vector<std::string>{
            "a_position","a_texcoord0"
        }));
        std::map<std::string,std::string> attributes,uniforms;
        std::map<std::string,gel::technique_parameters> parameters;
        attributes["a_position"] = "position";
        attributes["a_texcoord0"] = "texcoord0";
        parameters["projectionMatrix"] = gel::technique_parameters("PROJECTION",35676);
        parameters["position"] = gel::technique_parameters("POSITION",35665);
        parameters["texcoord0"] = gel::technique_parameters("TEXCOORD_0",35665);
        parameters["diffuse"] = gel::technique_parameters(35666);
        uniforms["u_projView"] = "projectionMatrix";
        uniforms["u_diffuse"] = "diffuse";
        //TODO: fill out for default technique
        model.techniques.push_back(gel::technique(attributes,uniforms,parameters,0));
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

void loadPrimitive(gel::primitive& primitive,const std::vector<gel::bufferView>& bufferViews,std::vector<gel::accessor>& accessors,gel::model& model){
    //Start vertex array for holding vbo/ibo state.
    glGenVertexArrays(1,&primitive.bufferReference);
    glBindVertexArray(primitive.bufferReference);

    //Load all vertex attributes.
    for(std::pair<std::string,int> attribute:primitive.attributes){
        SDL_Log("ACCESSOR: %i",attribute.second);
        gel::accessor accessor = accessors[attribute.second];
        gel::bufferView view = bufferViews[accessor.bufferView];
        glBindBuffer(GL_ARRAY_BUFFER,view.bufferReference);

        //Find the attribute location via the technique.
        std::map<std::string,gel::technique_parameters> parameters = model.techniques[model.materials[primitive.material].technique].parameters;
        for(std::pair<std::string,gel::technique_parameters> parameter:parameters){
            if(parameter.second.semantic == attribute.first){
                SDL_Log("SEMANTIC: %s,ATTRIBUTE: %s",parameter.second.semantic.c_str(),attribute.first.c_str());
                loadAccessor(accessor,view,parameter.second.parameterReference);
                break;
            }
        }
    }

    //Load index buffer if applicable.
    if(primitive.indices != -1){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferViews[accessors[primitive.indices].bufferView].bufferReference);
    }
}

void loadTechnique(gel::technique& tech,std::vector<gel::program>& programs){
    for(std::pair<std::string,std::string> element:tech.attributes)
        tech.parameters[element.second].parameterReference = glGetAttribLocation(programs[tech.program].programReference,element.first.c_str());
     for(std::pair<std::string,std::string> element:tech.uniforms)
        tech.parameters[element.second].parameterReference = glGetUniformLocation(programs[tech.program].programReference,element.first.c_str());   
    //TODO: finish
}

void loadProgram(gel::program& program,std::vector<gel::shader>& shaders){
    SDL_Log("===LOADING PROGRAM");
    program.programReference = glCreateProgram();
    glAttachShader(program.programReference,shaders[program.vertexShader].shaderReference);
    glAttachShader(program.programReference,shaders[program.fragmentShader].shaderReference);
    glLinkProgram(program.programReference);
    std::vector<int> results;results.push_back(0);
    glGetProgramiv(program.programReference,GL_LINK_STATUS,&results[0]);
    SDL_Log("===LINK STATUS IS: %i",results[0]);
    if(results[0] == 0){
        std::vector<char> log(256);
        glGetProgramInfoLog(program.programReference,log.size(),NULL,&log[0]);
        SDL_Log("---PROGRAM LINK STATUS IS: %s",std::string(log.data()).c_str());
    }
}

void loadShader(std::string path,gel::shader& shader){
    SDL_Log("===LOADING SHADER: %s",(path+shader.uri).c_str());
    shader.shaderReference = glCreateShader(shader.type == VERTEX_SHADER?GL_VERTEX_SHADER:GL_FRAGMENT_SHADER);
    std::vector<std::string> contents = std::vector<std::string>{"#version 300 es\n"+loadFileAsString(path+shader.uri)};
    glShaderSource(shader.shaderReference,1,(const char* const*)&contents[0],NULL);
    glCompileShader(shader.shaderReference);
    std::vector<int> results;results.push_back(0);
    glGetShaderiv(shader.shaderReference,GL_COMPILE_STATUS,&results[0]);
    SDL_Log("===COMPILE STATUS IS: %i",results[0]);
    if(results[0] == 0){
        std::vector<char> log(256);
        glGetShaderInfoLog(shader.shaderReference,log.size(),0,&log[0]);
        SDL_Log("---SHADER COMPILE FAILED: %s,SOURCE DUMP: <<<%s>>>",std::string(log.data()).c_str(),loadFileAsString(path+shader.uri).c_str());
    }
}

void loadMesh(gel::mesh& mesh,const std::vector<gel::bufferView>& bufferViews,std::vector<gel::accessor>& accessors,gel::model& model){
    for(int i = 0;i < mesh.primitives.size();i++) loadPrimitive(mesh.primitives[i],bufferViews,accessors,model);
    //TODO: finish
}

const void* getBufferPointer(const gel::bufferView& view,int offset,const std::vector<gel::buffer>& buffers){
    return buffers[view.buffer].data.data()+view.byteOffset+offset;
}

std::vector<float> getBufferValue(const float* bufferPointer,int index,int size){
    std::vector<float> values;
    //SDL_Log("INDEX: %i,SIZE: %i",index,size);
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

void setShaderValue(void* value,gel::technique_parameters parameter,bool isAttribute){
    if(parameter.parameterReference == -1) return; //Not found in shader.
    switch(parameter.type){
        case 5120://BYTE
        //if(isAttribute) glVertexAttrib1b(parameter.parameterReference,*(char*)value);
        //else            glUniform1b(parameter.parameterReference,*(char*)value);
        break;
        case 5121://UNSIGNED_BYTE
        //if(isAttribute) glVertexAttrib1ub(parameter.parameterReference,*(unsigned char*)value);
        //else            glUniform1ub(parameter.parameterReference,*(unsigned char*)value);
        break;
        case 5122://SHORT
        //if(isAttribute) glVertexAttrib1s(parameter.parameterReference,*(short*)value);
        //else            glUniform1s(parameter.parameterReference,*(short*)value);
        break;
        case 5123://UNSIGNED_SHORT
        //if(isAttribute) glVertexAttrib1us(parameter.parameterReference,*(unsigned short*)value);
        //else            glUniform1us(parameter.parameterReference,*(unsigned short*)value);
        break;
        case 5124://INT
        if(isAttribute) /*glVertexAttrib1i(parameter.parameterReference,*(int*)value)*/;
        else            glUniform1i(parameter.parameterReference,*(int*)value);
        break;
        case 5125://UNSIGNED_INT
        if(isAttribute) /*glVertexAttribI1ui(parameter.parameterReference,*(unsigned int*)value)*/;
        else            glUniform1ui(parameter.parameterReference,*(unsigned int*)value);
        break;
        case 5126://FLOAT
        if(isAttribute) glVertexAttrib1f(parameter.parameterReference,*(float*)value);
        else            glUniform1f(parameter.parameterReference,*(float*)value);
        break;
        case 35664://FLOAT_VEC2
        if(isAttribute) glVertexAttrib2fv(parameter.parameterReference,(float*)value);
        else            glUniform2fv(parameter.parameterReference,1,(float*)value);
        break;
        case 35665://FLOAT_VEC3
        if(isAttribute) glVertexAttrib3fv(parameter.parameterReference,(float*)value);
        else            glUniform3fv(parameter.parameterReference,1,(float*)value);
        break;
        case 35666://FLOAT_VEC4
        if(isAttribute) glVertexAttrib4fv(parameter.parameterReference,(float*)value);
        else            glUniform4fv(parameter.parameterReference,1,(float*)value);
        break;
        case 35667://INT_VEC2
        if(isAttribute) /*glVertexAttribI2iv(parameter.parameterReference,(int*)value)*/;
        else            glUniform2iv(parameter.parameterReference,1,(int*)value);
        break;
        case 35668://INT_VEC3
        if(isAttribute) /*glVertexAttribI3iv(parameter.parameterReference,(int*)value)*/;
        else            glUniform3iv(parameter.parameterReference,1,(int*)value);
        break;
        case 35669://INT_VEC4
        if(isAttribute) /*glVertexAttribI4iv(parameter.parameterReference,(int*)value)*/;
        else            glUniform4iv(parameter.parameterReference,1,(int*)value);
        break;
        /*case 35670://BOOL
        break;
        case 35671://BOOL_VEC2
        break;
        case 35672://BOOL_VEC3
        break;
        case 35673://BOOL_VEC4
        break; */
        case 35674://FLOAT_MAT2
        if(isAttribute) /*TODO*/;
        else            glUniformMatrix2fv(parameter.parameterReference,1,false,(float*)value);
        break;
        case 35675://FLOAT_MAT3
        if(isAttribute) /*TODO*/;
        else            glUniformMatrix3fv(parameter.parameterReference,1,false,(float*)value);
        break;
        case 35676://FLOAT_MAT4
        if(isAttribute) /*TODO*/;
        else            glUniformMatrix4fv(parameter.parameterReference,1,false,(float*)value);
        break;
        /*case 35678://SAMPLER_2D
        //??? Need to figure out how/where in OpenGL this might be set.
        break;*/
    }
}

void loadTexture(gel::texture& texture,std::string path,gel::model& model){
    glGenTextures(1, &texture.textureReference);
    glBindTexture(GL_TEXTURE_2D,texture.textureReference);

    //Load the image data.
    gel::image image = model.images[texture.source];
    SDL_Surface* imageData = IMG_Load((path+image.uri).c_str()); 
    int format = imageData->format->BytesPerPixel == 1 || imageData->format->BytesPerPixel == 3 ? GL_RGB : GL_RGBA;
    //SDL_Log("IMAGE width: %i,height: %i,bitsPerPixel: %i,bytesPerPixel: %i,format: %i",imageData->w, imageData->h,imageData->format->BitsPerPixel,
    //    imageData->format->BytesPerPixel,imageData->format->format);
    if(imageData->format->palette){
            //SDL_Log("PALETTE: %i",imageData->format->palette->ncolors);
            //for(int i = 0;i < imageData->format->palette->ncolors;i++)
            //    SDL_Log("PALETTE COLOR %i: (%i,%i,%i)",i,imageData->format->palette->colors[i].r,imageData->format->palette->colors[i].g,imageData->format->palette->colors[i].b);
            unsigned char* pixels = new unsigned char[imageData->w * imageData->h * 3];
            int pixelCounter = 0;
            for(int i = 0;i < imageData->w * imageData->h;i++){
               // SDL_Log("COLOR index %i is: %i",i,((unsigned char*)imageData->pixels)[i]);
                pixels[pixelCounter++] = imageData->format->palette->colors[((unsigned char*)imageData->pixels)[i]].r;
                pixels[pixelCounter++] = imageData->format->palette->colors[((unsigned char*)imageData->pixels)[i]].g;
                pixels[pixelCounter++] = imageData->format->palette->colors[((unsigned char*)imageData->pixels)[i]].b;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, format, imageData->w, imageData->h, 0,format, GL_UNSIGNED_BYTE,pixels);
    }else glTexImage2D(GL_TEXTURE_2D, 0, format,imageData->w,imageData->h, 0,format, GL_UNSIGNED_BYTE,imageData->pixels);
    
    gel::sampler sampler = model.samplers[texture.sampler];
    GLenum magFilter = GL_NEAREST,minFilter = GL_NEAREST,wrapS = GL_REPEAT,wrapT = GL_REPEAT;
    /*switch(sampler.magFilter){
        case 9729: magFilter = GL_LINEAR;break;
    }
    switch(sampler.minFilter){
        case 9729: minFilter = GL_LINEAR;break;
        case 9984: minFilter = GL_NEAREST_MIPMAP_NEAREST; break;
        case 9985: minFilter = GL_LINEAR_MIPMAP_NEAREST; break;
        case 9986: minFilter = GL_NEAREST_MIPMAP_LINEAR; break;
        case 9987: minFilter = GL_LINEAR_MIPMAP_LINEAR; break;
    }
    switch(sampler.wrapS){
        case 33071: wrapS = GL_CLAMP_TO_EDGE; break;
        case 33648: wrapS = GL_MIRRORED_REPEAT; break;
    }
    switch(sampler.wrapT){
        case 33071: wrapT = GL_CLAMP_TO_EDGE; break;
        case 33648: wrapT = GL_MIRRORED_REPEAT; break;
    }*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);          
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    SDL_FreeSurface(imageData);
}

void loadImage(gel::image& image){
    //TODO: finish
    //load into data buffer if bufferView is defined, or if the uri is a data-uri
}

void loadModel(gel::model& model,std::string path){
    SDL_Log("+++LOADING IMAGES...");
    for(int i = 0;i < model.images.size();i++) loadImage(model.images[i]);
    SDL_Log("+++LOADING TEXTURES...");
    for(int i = 0;i < model.textures.size();i++) loadTexture(model.textures[i],path,model);
    SDL_Log("+++LOADING BUFFERS...");
    for(int i = 0;i < model.buffers.size();i++) loadBuffer(path,model.buffers[i]);
    SDL_Log("+++LOADING BUFFER VIEWS...");
    for(int i = 0;i < model.bufferViews.size();i++) loadBufferView(model.bufferViews[i],model.buffers);
    SDL_Log("+++LOADING SHADERS...");
    for(int i = 0;i < model.shaders.size();i++) loadShader(path,model.shaders[i]);
    SDL_Log("+++LOADING PROGRAMS...");
    for(int i = 0;i < model.programs.size();i++) loadProgram(model.programs[i],model.shaders);
    SDL_Log("+++LOADING TECHNIQUES...");
    for(int i = 0;i < model.techniques.size();i++) loadTechnique(model.techniques[i],model.programs);
    SDL_Log("+++LOADING MESHES...");
    for(int i = 0;i < model.meshes.size();i++) loadMesh(model.meshes[i],model.bufferViews,model.accessors,model);
}

void renderTechnique(gel::technique& technique,gel::model& model,std::map<std::string,void*> values){
    //TODO: only call glUseProgram if program changed
    glUseProgram(model.programs[technique.program].programReference);
    for(std::pair<std::string,void*> value:values){
        if(technique.uniforms.find(value.first) != technique.uniforms.end())
            setShaderValue(value.second,technique.parameters[technique.uniforms[value.first]],false);
        else setShaderValue(value.second,technique.parameters[technique.attributes[value.first]],true);
    }
    //TODO: finish
}

void renderTexture(gel::material material,gel::model& model){
    if(material.pbrMetallicRoughness.baseColorTexture.index != -1)
        glBindTexture(GL_TEXTURE_2D,model.textures[material.pbrMetallicRoughness.baseColorTexture.index].textureReference);
    else if(material.pbrSpecularGlossiness.diffuseTexture.index != -1){
        glBindTexture(GL_TEXTURE_2D,model.textures[material.pbrSpecularGlossiness.diffuseTexture.index].textureReference);
    }
    //TODO: fill this out a lot more with various textures, factors, etc. for both metallicRoughness and specularGlossiness
}

void renderMesh(gel::mesh& mesh,glm::mat4 transform,gel::model& model){
    //SDL_Log(">>>RENDER MESH");
    if(mesh.isVisible)
    for(gel::primitive primitive:mesh.primitives){
        renderTechnique(model.techniques[model.materials[primitive.material].technique],model,
            std::map<std::string,void*>{
                {"u_projView",glm::value_ptr(transform)},
                {"u_diffuse",
                model.materials[primitive.material].pbrMetallicRoughness.metallicFactor !=-1.0f?
                model.materials[primitive.material].pbrMetallicRoughness.baseColorFactor.data():
                model.materials[primitive.material].pbrSpecularGlossiness.diffuseFactor.data()}
            });
        renderTexture(model.materials[primitive.material],model);
        glBindVertexArray(primitive.bufferReference);
        if(primitive.indices != -1){
            //SDL_Log(">>>GL_DRAW_ELEMENTS [INDICES: %i,COMPONENT TYPE: %i]",
            //    model.accessors[primitive.indices].count,getComponentType(model.accessors[primitive.indices].componentType));
            glDrawElements(GL_TRIANGLES,model.accessors[primitive.indices].count,getComponentType(model.accessors[primitive.indices].componentType),
                (void *)(intptr_t)model.accessors[primitive.indices].byteOffset);      
        }
        else glDrawArrays(GL_TRIANGLES,0,model.accessors[primitive.attributes["POSITION"]].count);
    }
}

glm::vec4 getSamplerValue(float curr,std::string path,gel::animation_sampler& sampler,gel::model& model){
    glm::vec4 sampleValue;
    if(sampler.interpolation != "LINEAR"){SDL_Log("!!!UNSUPPORTED SAMPLER INTERPOLATION: %s",sampler.interpolation.c_str());return sampleValue;}
    float prev = 0,next = 0; int prevIndex = 0,nextIndex = 0;
    curr = getLinearBoundingValues(curr,prev,next,prevIndex,nextIndex,model.accessors[sampler.input],model.bufferViews,model.buffers);
    //SDL_Log("CURRENT VAL: %f, BOUNDS: (%f,%f) or INDEX(%i,%i)",curr,prev,next,prevIndex,nextIndex);
    float interpolator = (curr - prev) / (next - prev);
    const float* output_ptr = (const float*)getBufferPointer(model.bufferViews[model.accessors[sampler.output].bufferView],model.accessors[sampler.output].byteOffset,model.buffers);
    std::vector<float> outputPrevious = getBufferValue(output_ptr,prevIndex,getTypeSize(model.accessors[sampler.output].type)),
        outputNext = getBufferValue(output_ptr,nextIndex,getTypeSize(model.accessors[sampler.output].type));
    //SDL_Log("OUTPUT PREV: %f,%f,%f,%f and OUTPUT NEXT: %f,%f,%f,%f",outputPrevious[0],outputPrevious[1],outputPrevious[2],outputPrevious[3],
    //    outputNext[0],outputNext[1],outputNext[2],outputNext[3]);
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
   // SDL_Log("SAMPLE VALUE: %f,%f,%f,%f",sampleValue[0],sampleValue[1],sampleValue[2],sampleValue[3]);
    if(channel.target.path == "rotation") model.nodes[channel.target.node].rotation = sampleValue;
    else if(channel.target.path == "translation") model.nodes[channel.target.node].translation = sampleValue;
    else if(channel.target.path == "scale") model.nodes[channel.target.node].scale = sampleValue;
    //TODO: add 'weights' target
}

float curr = 0.0f;
void renderNode(gel::model& model,gel::node& node,gel::camera& camera,glm::mat4 parent){
    parent = parent * node.matrix * glm::translate(node.translation) 
        * glm::mat4_cast(glm::quat(node.rotation[3],node.rotation[0],node.rotation[1],node.rotation[2])) 
        * glm::scale(node.scale);
    if(node.mesh != -1)
         renderMesh(model.meshes[node.mesh],camera.transform * parent,model);
    //Render all child nodes, if any.
    for(int i = 0;i < node.children.size();i++)
        renderNode(model,model.nodes[node.children[i]],camera,parent);
}

void renderModel(gel::model& model,gel::camera& camera){
    glm::mat4 parent;

    //Render any animation channels.
    for(int i = 0;i < model.animations.size();i++)
    renderChannel(curr,model.animations[i],0,model);
    curr += 0.01f;

    //Set shader values.
    //shader.begin();

    //Render nodes.
    for(gel::scene scene:model.scenes)
    for(int i = 0;i < scene.nodes.size();i++)
        renderNode(model,model.nodes[scene.nodes[i]],camera,parent);
    //shader.end();
}
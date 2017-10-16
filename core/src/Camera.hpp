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

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp>

namespace gel
{
    class Camera
    {
        glm::mat4 projection,ortho,view;
    public:
        float fovY,aspect,near,far,width,height;

        Camera(){}
        Camera(float fovY,float width,float height,float near,float far):
            fovY(fovY),width(width),height(height),aspect(width/height),near(near),far(far),view(glm::mat4(1.0f)){
                projection = glm::perspective(fovY,aspect,near,far);
                ortho = glm::ortho(0.0f,width,height,0.0f,near,far);
            }
        ~Camera(){};
        
        void lookAt(glm::vec3 eye,glm::vec3 center,glm::vec3 up){
            view = glm::lookAt(eye,center,up);
        }
        
        void translate(glm::vec3 amount){
            view = glm::translate(view,amount);
        }
        
        void rotate(glm::vec3 amount){
            view = glm::rotate(view,amount.y,glm::vec3(1.0f,0.0f,0.0f));
            view = glm::rotate(view,amount.x,glm::vec3(0.0f,1.0f,0.0f));
            view = glm::rotate(view,amount.z,glm::vec3(0.0f,0.0f,1.0f));
        }
        
        void rotate(glm::vec3 point,glm::vec3 amount){
            translate(-point);
            rotate(amount);
            translate(point);
        }
        
        void setAspect(float width,float height){
            this->width = width;this->height = height;
            aspect = width / height;
            projection = glm::perspective(fovY,aspect,near,far);
            ortho = glm::ortho(0.0f,width,height,0.0f,near,far);
        }
        
        glm::mat4 getCombined(){
            return projection * view;
        }

        glm::mat4 getOrtho(){
            return ortho;
        }
    };


}

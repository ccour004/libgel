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

#include <SDL.h>

namespace gel
{
    class RawInputProcessor {
    public:
        //SDL_AudioDeviceEvent(SDL_AUDIODEVICEADDED,SDL_AUDIODEVICEREMOVED)
        virtual bool controllerAxisEvent(const SDL_ControllerAxisEvent& event) = 0;
        virtual bool controllerButtonEvent(const SDL_ControllerButtonEvent& event) = 0;
        virtual bool controllerDeviceEvent(const SDL_ControllerDeviceEvent& event) = 0;
        //SDL_DollarGestureEvent(SDL_DOLLARGESTURE,SDL_DOLLARRECORD)
        //SDL_DropEvent(SDL_DROPFILE,SDL_DROPTEXT,SDL_DROPBEGIN,SDL_DROPCOMPLETE)
        virtual bool touchFingerEvent(const SDL_TouchFingerEvent& event) = 0;
        virtual bool keyboardEvent(const SDL_KeyboardEvent& event) = 0;
        //SDL_JoyAxisEvent(SDL_JOYAXISMOTION)
        //SDL_JoyBallEvent(SDL_JOYBALLMOTION)
        //SDL_JoyHatEvent(SDL_JOYHATMOTION)
        //SDL_JoyButtonEvent(SDL_JOYBUTTONDOWN,SDL_JOYBUTTONUP)
        //SDL_JoyDeviceEvent(SDL_JOYDEVICEADDED,SDL_JOYDEVICEREMOVED)
        virtual bool mouseMotionEvent(const SDL_MouseMotionEvent& event) = 0;
        virtual bool mouseButtonEvent(const SDL_MouseButtonEvent& event) = 0;
        virtual bool mouseWheelEvent(const SDL_MouseWheelEvent& event) = 0;
        virtual bool multiGestureEvent(const SDL_MultiGestureEvent& event) = 0;
        //SDL_TextEditingEvent(SDL_TEXTEDITING)
        //SDL_TextInputEvent(SDL_TEXTINPUT)
        //SDL_UserEvent(SDL_USEREVENT)
    };
}
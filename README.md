# libgel
A cross-platform 3D game engine library written in C++11/14, built using other FOSS libraries.

How to build:
-------------
* Make sure that your ANDROID_HOME and ANDROID_NDK_HOME path variables are set.
* Make sure that CMake is in your path.
* In CMakeLists.txt: change the HOME_DIR variable to the full path of the libgel folder on your machine!
* The default ABI for Android is: arm64-v8a. Make sure to change this if this doesn't match your target device.
* Connect an Android device to your build machine via ADB.
* Run ./build.sh (note: this is assuming a Unix-based build machine for now - this will change).
* Run ./clean.sh to clean up your build environment.

Notes:
------
* If using GCC/G++ to compile, the version must be at least 4.9.
* CMake respects the CC/CXX environment varibles in a Linux environment for specifying the compiler (i.e., GCC vs Clang).

Supported platforms:
--------------------
* Desktop (Linux, Windows, and Mac OS)
* Mobile (Android)

Future desired support:
-----------------------
* Mobile (iOS)
* Browser (WebAssembly)

Build/toolchain dependencies:
-----------------------------
* CMake
* Android SDK (adb)
* Android NDK (ndk-build)

Project dependencies:
---------------------
* SDL2 (low-level windowing, input, threads, audio)
* SDL2_image (texture loading)
* OpenGL (graphics processing)
* GLEW (graphics extension 'wrangling' for desktop only)
* glm (linear algebra)
* entityx (entity component system)
* bullet3 (physics and collision response; only using v.2 code from the repo currently)

Future project dependencies:
----------------------------
* Vulkan (really low-level graphics processing)
* assimp (3D scene/model loader)
* Cap n' proto (serialization)


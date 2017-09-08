#!/bin/bash

#Desktop
mkdir build
cd build && ln -s ../assets assets 
cmake ..
make
./Core &> /dev/null
cd ..

#Android
cd android
./gradlew installDebug
adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
cd ..

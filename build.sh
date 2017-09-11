#!/bin/bash

#Desktop
mkdir build && cd build
cmake ..
make
./Core &> /dev/null
cd ..

#Android
cd android
./gradlew installDebug
adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity
cd ..

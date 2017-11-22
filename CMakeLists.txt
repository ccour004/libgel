cmake_minimum_required (VERSION 3.4.1)
project (Core)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_ANDROID_STL_TYPE "c++_shared")
#set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(HOME_DIR /home/colin/git/libgel)
set(EXTERNAL external/build)
set(PREBUILT ${HOME_DIR}/external/prebuilt/${CMAKE_HOST_SYSTEM_NAME}_Ubuntu_${CMAKE_HOST_SYSTEM_PROCESSOR})
#set(ANDROID_DIR ${HOME_DIR}/android)
#set(DEP ${ANDROID_DIR}/dependencies)
#set(LIBS ${DEP}/libs)
#set(JNI ${DEP}/jni)
#set(DEP_LIST SDL2;SDL2_image;SDL2_ttf;Bullet;entityx)

#set(ANDROID_APP_NAME libgel_test)
#set(ANDROID_COMPILE_SDK 26)
#set(ANDROID_BUILD_TOOLS 25.0.3)
#set(ANDROID_APPLICATION_MK ${DEP}/jni/Application.mk)
#set(ANDROID_APPLICATION_ID org.libsdl.app)

#set(ANDROID_ABI "arm64-v8a")#"arm64-v8a,armeabi-v7a,x86,x86_64"
#set(ANDROID_MIN_SDK 18)
#set(ANDROID_TARGET_SDK 26)

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
        message(STATUS "BUILDING FOR ANDROID...")

	find_library(GLES2_LIBRARY libGLESv2.so HINTS ${GLES2_INCLUDE_DIR}/../lib)
	find_library(GLES3_LIBRARY libGLESv3.so HINTS ${GLES3_INCLUDE_DIR}/../lib)	

	#Run ndk-build
	execute_process(COMMAND ndk-build WORKING_DIRECTORY ${JNI})

	#Copy shared object libraries into the app's build chain (these aren't auto-copied by cmake for some reason???).
	file(MAKE_DIRECTORY ${ANDROID_DIR}/app/build/intermediates/cmake/debug/obj)
	file(COPY ${LIBS}/. DESTINATION ${ANDROID_DIR}/app/build/intermediates/cmake/debug/obj)

	#GLM
	include_directories(${JNI}/GLM/src/GLM_DL)

	#RapidXML
	include_directories(${JNI}/XML/src/XML_DL)

	#poly2tri
	include_directories(${JNI}/P2T/src/P2T_DL)

	foreach(DEPENDENCY ${DEP_LIST})
		message(STATUS "BUILDING DEPENDENCY ${DEPENDENCY}...")
		add_library(${DEPENDENCY} SHARED IMPORTED)
		set_target_properties(${DEPENDENCY} PROPERTIES IMPORTED_LOCATION ${LIBS}/${ANDROID_ABI}/lib${DEPENDENCY}.so)
		include_directories(${JNI}/${DEPENDENCY}/src/${DEPENDENCY}_DL)
		include_directories(${JNI}/${DEPENDENCY}/src/${DEPENDENCY})
		include_directories(${JNI}/${DEPENDENCY}/src/${DEPENDENCY}/${DEPENDENCY})
		include_directories(${JNI}/${DEPENDENCY}/src/${DEPENDENCY}/src)
		include_directories(${JNI}/${DEPENDENCY}/src/${DEPENDENCY}_DL/include)	
	endforeach()

	#main
	aux_source_directory(${HOME_DIR}/core/src/ SOURCE)
	include_directories(${HOME_DIR}/core/src/)
	add_library(main SHARED ${SOURCE} ${JNI}/SDL2/src/SDL2_DL/src/main/android/SDL_android_main.c)

	#Link everything together (NB: ORDER MAY MATTER!!).
	target_link_libraries(main ${GLES2_LIBRARY} ${GLES3_LIBRARY})

	foreach(DEPENDENCY ${DEP_LIST})
		target_link_libraries(main ${DEPENDENCY})
	endforeach()

	#Install Android app and launch.
	execute_process(COMMAND adb shell am start -n org.libsdl.app/org.libsdl.app.SDLActivity)
else()
        #message(STATUS "ANDROID DEV CONFIGURE...")

	#Buid up the Android dependency string.
	#set(ANDROID_LIBRARIES "")
	#foreach(DEPENDENCY ${DEP_LIST})
	#	set(ANDROID_LIBRARIES ${ANDROID_LIBRARIES}${DEPENDENCY}",")
	#endforeach()
	#set(ANDROID_LIBRARIES ${ANDROID_LIBRARIES}main)

	#Build the Android app directories.
	#file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/java/org/libsdl/app)
	#file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/assets)
	#file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/cpp)
	#file(COPY ${ANDROID_DIR}/res DESTINATION ${ANDROID_DIR}/app/src/main)
	#file(COPY ${ANDROID_DIR}/jni DESTINATION ${ANDROID_DIR}/dependencies)

	#Symlinks, or deep-copy directories if host is non-UNIX.
	#if(UNIX)
	#	execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${HOME_DIR}/assets ${ANDROID_DIR}/app/src/main/assets/assets)
	#	execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${HOME_DIR}/core ${ANDROID_DIR}/app/src/main/cpp/core)
	#else()
	#	file(COPY ${HOME_DIR}/assets DESTINATION ${ANDROID_DIR}/app/src/main/assets/assets)
	#	file(COPY ${HOME_DIR}/core DESTINATION ${ANDROID_DIR}/app/src/main/cpp/core)
	#endif()

	#Edit configuration files.
	#configure_file(${ANDROID_DIR}/proguard-rules_in.pro ${ANDROID_DIR}/app/proguard-rules.pro)
	#configure_file(${ANDROID_DIR}/AndroidManifest_in.xml ${ANDROID_DIR}/app/src/main/AndroidManifest.xml)
	#configure_file(${ANDROID_DIR}/build_in.gradle ${ANDROID_DIR}/app/build.gradle)
	#configure_file(${ANDROID_DIR}/SDLActivity_in.java ${ANDROID_DIR}/app/src/main/java/org/libsdl/app/SDLActivity.java)
	#configure_file(${ANDROID_DIR}/strings_in.xml ${ANDROID_DIR}/app/src/main/res/values/strings.xml)

	#Build settings.gradle,Application.mk
	#file(WRITE ${ANDROID_DIR}/settings.gradle "include ':app'")
	#file(WRITE ${ANDROID_APPLICATION_MK} "NDK_TOOLCHAIN_VERSION := ${CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION}\n")
	#file(APPEND ${ANDROID_APPLICATION_MK} "APP_PLATFORM := android-${ANDROID_TARGET_SDK}\n")
	#file(APPEND ${ANDROID_APPLICATION_MK} "APP_CPPFLAGS += -std=c++${CMAKE_CXX_STANDARD}\n")
	#file(APPEND ${ANDROID_APPLICATION_MK} "APP_CPPFLAGS += -frtti\n")
	#file(APPEND ${ANDROID_APPLICATION_MK} "APP_STL := ${CMAKE_ANDROID_STL_TYPE}\n")		
	#file(APPEND ${ANDROID_APPLICATION_MK} "APP_ABI := ${ANDROID_ABI}\n")	

        message(STATUS "BUILDING FOR DESKTOP (${CMAKE_HOST_SYSTEM_NAME},${CMAKE_HOST_SYSTEM_PROCESSOR})...")

	#OPENGL (GL and GLU)
	find_package(OpenGL)
	if (OPENGL_FOUND)
	    include_directories(${OPENGL_INCLUDE_DIR})
	endif()

	#GLEW
	find_package(GLEW)
	if (GLEW_FOUND)
	    include_directories(${GLEW_INCLUDE_DIRS})
	endif()

	#FREETYPE
	find_package(Freetype)
	if(FREETYPE_FOUND)
	    include_directories(${FREETYPE_INCLUDE_DIRS})
	endif()

	##HEADER ONLY
	include_directories(${EXTERNAL}/GLM/src/GLM_DL) #GLM
	include_directories(${EXTERNAL}/XML/src/XML_DL) #RapidXML
	include_directories(${EXTERNAL}/P2T/src/P2T_DL) #Poly2Tri

	#SDL2
	include_directories(${EXTERNAL}/SDL2/src/SDL2_DL/include)
	add_library(SDL2 SHARED IMPORTED)
	set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libSDL2-2.0.so)

	#SDL2_image
	include_directories(${EXTERNAL}/SDL2_image/src/SDL2_image_DL)
	add_library(SDL2_image SHARED IMPORTED)
	set_target_properties(SDL2_image PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libSDL2_image-2.0.so)

	#SDL2_ttf
	include_directories(${EXTERNAL}/SDL2_ttf/src/SDL2_ttf_DL)
	add_library(SDL2_ttf SHARED IMPORTED)
	set_target_properties(SDL2_ttf PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libSDL2_ttf-2.0.so)

	#Assimp
	include_directories(${EXTERNAL}/Assimp/src/Assimp/include)
	include_directories(${EXTERNAL}/Assimp/src/Assimp-build/include)
	add_library(assimp SHARED IMPORTED)
	set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libassimp.so)

	#Bullet Physics (v.3, but only using v.2 CPU-based code)
	include_directories(${EXTERNAL}/Bullet/src/Bullet/src)

	add_library(linearMath SHARED IMPORTED)
	add_library(common SHARED IMPORTED)
	add_library(collision SHARED IMPORTED)
	add_library(dynamics SHARED IMPORTED)

	set_target_properties(linearMath PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libLinearMath.so)
	set_target_properties(common PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libBullet3Common.so)
	set_target_properties(collision PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libBulletCollision.so)
	set_target_properties(dynamics PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libBulletDynamics.so)

	#entityx
	include_directories(${EXTERNAL}/entityx/src/entityx)
	add_library(entityx SHARED IMPORTED)
	set_target_properties(entityx PROPERTIES IMPORTED_LOCATION ${PREBUILT}/libentityx.so)
		
	#final setup
	aux_source_directory(core/src/ SOURCE)
	aux_source_directory(${EXTERNAL}/P2T/src/P2T_DL/poly2tri/sweep P2T_SWEEP)
	aux_source_directory(${EXTERNAL}/P2T/src/P2T_DL/poly2tri/common P2T_COMMON)
	add_executable(Core ${P2T_COMMON} ${P2T_SWEEP} ${SOURCE})
	include_directories(core/src)
        target_link_libraries(Core ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${FREETYPE_LIBRARIES} SDL2 SDL2_image SDL2_ttf
		linearMath common collision dynamics entityx assimp)
endif()



cmake_minimum_required (VERSION 3.4.1)
project (Core)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_ANDROID_STL_TYPE "c++_shared")
set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(HOME_DIR /home/colin/git/libgel)
set(ANDROID_DIR ${HOME_DIR}/android)
set(DEP ${ANDROID_DIR}/dependencies)
set(LIBS ${DEP}/libs)
set(JNI ${DEP}/jni)
set(DEP_LIST SDL2;SDL2_image;SDL2_ttf;Bullet;entityx)

set(ANDROID_APP_NAME libgel_test)
set(ANDROID_COMPILE_SDK 26)
set(ANDROID_BUILD_TOOLS 25.0.3)
set(ANDROID_APPLICATION_MK ${DEP}/jni/Application.mk)
set(ANDROID_APPLICATION_ID org.libsdl.app)

set(ANDROID_ABI "arm64-v8a")#"arm64-v8a,armeabi-v7a,x86,x86_64"
set(ANDROID_MIN_SDK 18)
set(ANDROID_TARGET_SDK 26)

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
        message(STATUS "ANDROID DEV CONFIGURE...")

	#Buid up the Android dependency string.
	set(ANDROID_LIBRARIES "")
	foreach(DEPENDENCY ${DEP_LIST})
		set(ANDROID_LIBRARIES ${ANDROID_LIBRARIES}${DEPENDENCY}",")
	endforeach()
	set(ANDROID_LIBRARIES ${ANDROID_LIBRARIES}main)

	#Build the Android app directories.
	file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/java/org/libsdl/app)
	file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/assets)
	file(MAKE_DIRECTORY ${ANDROID_DIR}/app/src/main/cpp)
	file(COPY ${ANDROID_DIR}/res DESTINATION ${ANDROID_DIR}/app/src/main)
	file(COPY ${ANDROID_DIR}/jni DESTINATION ${ANDROID_DIR}/dependencies)

	#Symlinks, or deep-copy directories if host is non-UNIX.
	if(UNIX)
		execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${HOME_DIR}/assets ${ANDROID_DIR}/app/src/main/assets/assets)
		execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${HOME_DIR}/core ${ANDROID_DIR}/app/src/main/cpp/core)
	else()
		file(COPY ${HOME_DIR}/assets DESTINATION ${ANDROID_DIR}/app/src/main/assets/assets)
		file(COPY ${HOME_DIR}/core DESTINATION ${ANDROID_DIR}/app/src/main/cpp/core)
	endif()

	#Edit configuration files.
	configure_file(${ANDROID_DIR}/proguard-rules_in.pro ${ANDROID_DIR}/app/proguard-rules.pro)
	configure_file(${ANDROID_DIR}/AndroidManifest_in.xml ${ANDROID_DIR}/app/src/main/AndroidManifest.xml)
	configure_file(${ANDROID_DIR}/build_in.gradle ${ANDROID_DIR}/app/build.gradle)
	configure_file(${ANDROID_DIR}/SDLActivity_in.java ${ANDROID_DIR}/app/src/main/java/org/libsdl/app/SDLActivity.java)
	configure_file(${ANDROID_DIR}/strings_in.xml ${ANDROID_DIR}/app/src/main/res/values/strings.xml)

	#Build settings.gradle,Application.mk
	file(WRITE ${ANDROID_DIR}/settings.gradle "include ':app'")
	file(WRITE ${ANDROID_APPLICATION_MK} "NDK_TOOLCHAIN_VERSION := ${CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION}\n")
	file(APPEND ${ANDROID_APPLICATION_MK} "APP_PLATFORM := android-${ANDROID_TARGET_SDK}\n")
	file(APPEND ${ANDROID_APPLICATION_MK} "APP_CPPFLAGS += -std=c++${CMAKE_CXX_STANDARD}\n")
	file(APPEND ${ANDROID_APPLICATION_MK} "APP_CPPFLAGS += -frtti\n")
	file(APPEND ${ANDROID_APPLICATION_MK} "APP_STL := ${CMAKE_ANDROID_STL_TYPE}\n")		
	file(APPEND ${ANDROID_APPLICATION_MK} "APP_ABI := ${ANDROID_ABI}\n")	

        message(STATUS "BUILDING FOR DESKTOP...")

	#Symlinks, or deep-copy directories if host is non-UNIX.
	if(UNIX)
		execute_process(COMMAND "${CMAKE_COMMAND}" -E create_symlink ${HOME_DIR}/assets ${HOME_DIR}/build/assets)
	else()
		file(COPY ${HOME_DIR}/assets DESTINATION ${HOME_DIR}/build/assets)
	endif()

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

	#PTHREADS
	#set(THREADS_PREFER_PTHREAD_FLAG ON)
	#find_package(Threads REQUIRED)

	#GLM
	#if(FORCE_SYSTEM_LIBRARIES)
	#	find_package(GLM REQUIRED)
	#	include_directories(${GLM_INCLUDE_DIRS})
	#else()
		ExternalProject_Add(GLM_DL 
			   URL https://github.com/g-truc/glm/releases/download/0.9.8.4/glm-0.9.8.4.zip
			   PREFIX ${JNI}/GLM
			   CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND cmake -E echo "Skipping build step.")
		include_directories(${JNI}/GLM)
	#endif()

	#SDL2
	#if(FORCE_SYSTEM_LIBRARIES)
	#	find_package(SDL2 REQUIRED)
	#	include_directories(${SDL2_INCLUDE_DIR})  
	#	target_link_libraries(Core ${SDL2_LIBRARY})
	#else()
		ExternalProject_Add(SDL2_DL 
			   URL https://libsdl.org/release/SDL2-2.0.5.zip
			   PREFIX ${JNI}/SDL2
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND make)

		ExternalProject_Get_Property(SDL2_DL binary_dir)
		ExternalProject_Get_Property(SDL2_DL source_dir)

		include_directories(${source_dir}/include)

		add_library(SDL2 SHARED IMPORTED)
		add_dependencies(SDL2 SDL2_DL)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${binary_dir}/libSDL2-2.0.so)
	#endif()

	#SDL2_image
	#if(FORCE_SYSTEM_LIBRARIES)
	#	find_package(SDL2_image REQUIRED)
	#	include_directories(${SDL2IMAGE_INCLUDE_DIR})
	#	target_link_libraries(Core ${SDL2IMAGE_LIBRARY})
	#else()
		ExternalProject_Add(SDL2_image_DL 
			   URL https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.1.zip
			   PREFIX ${JNI}/SDL2_image
			   CONFIGURE_COMMAND sh ${JNI}/SDL2_image/src/SDL2_image_DL/configure
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND make)

		ExternalProject_Get_Property(SDL2_image_DL binary_dir)
		ExternalProject_Get_Property(SDL2_image_DL source_dir)

		include_directories(${source_dir})

		add_library(SDL2_image SHARED IMPORTED)
		add_dependencies(SDL2_image SDL2_image_DL)
		set_target_properties(SDL2_image PROPERTIES IMPORTED_LOCATION ${binary_dir}/.libs/libSDL2_image.so)
	#endif()

	#SDL2_ttf
	#if(FORCE_SYSTEM_LIBRARIES)
	#	find_package(SDL2_ttf REQUIRED)
	#	include_directories(${SDL2_TTF_INCLUDE_DIRS})
	#else()
		ExternalProject_Add(SDL2_ttf_DL 
			   URL https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.zip
			   PREFIX ${JNI}/SDL2_ttf
			   CONFIGURE_COMMAND sh ${JNI}/SDL2_ttf/src/SDL2_ttf_DL/external/freetype-2.4.12/configure 
				COMMAND make -C ${JNI}/SDL2_ttf/src/SDL2_ttf_DL/external/freetype-2.4.12
				COMMAND make install -C ${JNI}/SDL2_ttf/src/SDL2_ttf_DL/external/freetype-2.4.12
				COMMAND sh ${JNI}/SDL2_ttf/src/SDL2_ttf_DL/configure
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND make)

		ExternalProject_Get_Property(SDL2_ttf_DL binary_dir)
		ExternalProject_Get_Property(SDL2_ttf_DL source_dir)

		include_directories(${source_dir})

		add_library(SDL2_ttf SHARED IMPORTED)
		add_dependencies(SDL2_ttf SDL2_ttf_DL)
		set_target_properties(SDL2_ttf PROPERTIES IMPORTED_LOCATION ${binary_dir}/.libs/libSDL2_ttf.so)
	#endif()

	#Bullet Physics (v.3, but only using v.2 CPU-based code)
	ExternalProject_Add(Bullet 
		    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
		    PREFIX ${JNI}/Bullet
		    CMAKE_ARGS 
		       -DBUILD_SHARED_LIBS=ON 
		       -DBUILD_UNIT_TESTS=OFF 
		       -DBUILD_BULLET3=ON 
		       -DBUILD_EXTRAS=OFF 
		       -DBUILD_CPU_DEMOS=OFF 
		       -DUSE_GLUT=OFF 
		       -DBUILD_PYBULLET=OFF 
		       -DBUILD_BULLET2_DEMOS=OFF
		    INSTALL_COMMAND cmake -E echo "Skipping install step."
		    BUILD_COMMAND make)

	ExternalProject_Get_Property(Bullet binary_dir)
	ExternalProject_Get_Property(Bullet source_dir)

	add_library(linearMath SHARED IMPORTED)
	add_library(common SHARED IMPORTED)
	add_library(collision SHARED IMPORTED)
	add_library(dynamics SHARED IMPORTED)
	add_library(softBody SHARED IMPORTED)
	add_library(inverseDynamics SHARED IMPORTED)

	add_dependencies(linearMath Bullet)
	add_dependencies(common Bullet)
	add_dependencies(collision Bullet)
	add_dependencies(dynamics Bullet)
	add_dependencies(softBody Bullet)
	add_dependencies(inverseDynamics Bullet)

	set_target_properties(linearMath PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/LinearMath/libLinearMath.so)
	set_target_properties(common PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/Bullet3Common/libBullet3Common.so)
	set_target_properties(collision PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletCollision/libBulletCollision.so)
	set_target_properties(dynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletDynamics/libBulletDynamics.so)
	set_target_properties(softBody PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletSoftBody/libBulletSoftBody.so)
	set_target_properties(inverseDynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletInverseDynamics/libBulletInverseDynamics.so)

	include_directories(${source_dir}/src)

	#entityx
	ExternalProject_Add(entityx 
		   GIT_REPOSITORY https://github.com/alecthomas/entityx.git
		   PREFIX ${JNI}/entityx
		   INSTALL_COMMAND cmake -E echo "Skipping install step."
		   BUILD_COMMAND make)

	ExternalProject_Get_Property(entityx binary_dir)
	ExternalProject_Get_Property(entityx source_dir)

	add_library(project_entityx SHARED IMPORTED)
	add_dependencies(project_entityx entityx)
	set_target_properties(project_entityx PROPERTIES IMPORTED_LOCATION ${binary_dir}/libentityx.so)
	include_directories(${source_dir})

	#zeromq
	ExternalProject_Add(libzmq
		GIT_REPOSITORY https://github.com/zeromq/libzmq.git
		PREFIX ${JNI}/libzmq
		INSTALL_COMMAND sudo make -j4 install
		BUILD_COMMAND make)	

	ExternalProject_Add(cppzmq
		GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
		PREFIX ${JNI}/cppzmq
		INSTALL_COMMAND cmake -E echo "Skipping install step."
		BUILD_COMMAND make)	

	#final setup
	aux_source_directory(core/src/ SOURCE)
	add_executable(Core ${SOURCE})
	include_directories(core/src)

        #target_link_libraries(Core Threads::Threads ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} SDL2 SDL2_image SDL2_ttf
	#	linearMath common collision dynamics softBody inverseDynamics
	#	project_entityx)

        target_link_libraries(Core ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} SDL2 SDL2_image SDL2_ttf
		linearMath common collision dynamics softBody inverseDynamics
		project_entityx)
endif()



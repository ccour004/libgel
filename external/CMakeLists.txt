cmake_minimum_required (VERSION 3.4.1)
project (Core)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
        
message(STATUS "BUILDING FOR DESKTOP...")

##HEADER ONLY

#GLM
ExternalProject_Add(GLM_DL 
	URL https://github.com/g-truc/glm/releases/download/0.9.8.4/glm-0.9.8.4.zip
	PREFIX GLM
	CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND cmake -E echo "Skipping build step.")

#RapidXML
ExternalProject_Add(XML_DL 
	URL https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip?r=http%3A%2F%2Frapidxml.sourceforge.net%2F&ts=1508434717&use_mirror=astuteinternet
	PREFIX XML
	CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND cmake -E echo "Skipping build step.")

#poly2tri
ExternalProject_Add(P2T_DL
	GIT_REPOSITORY https://github.com/greenm01/poly2tri.git
	PREFIX P2T
	CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND cmake -E echo "Skipping build step.")

##BUILD

#SDL2
ExternalProject_Add(SDL2_DL 
	URL https://libsdl.org/release/SDL2-2.0.5.zip
	PREFIX SDL2
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND make)

#SDL2_image
ExternalProject_Add(SDL2_image_DL 
	URL https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.1.zip
	PREFIX SDL2_image
	CONFIGURE_COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/SDL2_image/src/SDL2_image_DL/configure
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND make)

#SDL2_ttf
ExternalProject_Add(SDL2_ttf_DL 
	URL https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.14.zip
	PREFIX SDL2_ttf
	CONFIGURE_COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/SDL2_ttf/src/SDL2_ttf_DL/configure
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND make)

#Freetype
#ExternalProject_Add(freetype 
#	GIT_REPOSITORY git://git.sv.nongnu.org/freetype/freetype2.git
#	PREFIX freetype 
#	INSTALL_COMMAND cmake -E echo "Skipping install step."
#	BUILD_COMMAND make)

#Bullet Physics (v.3, but only using v.2 CPU-based code)
ExternalProject_Add(Bullet 
	GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
	PREFIX Bullet
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

#entityx
ExternalProject_Add(entityx 
	GIT_REPOSITORY https://github.com/alecthomas/entityx.git
	PREFIX entityx 
	#CMAKE_ARGS 
	#	-DENTITYX_BUILD_SHARED=0
	INSTALL_COMMAND cmake -E echo "Skipping install step."
	BUILD_COMMAND make)


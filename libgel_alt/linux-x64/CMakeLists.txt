cmake_minimum_required (VERSION 3.4.1)
project (Core)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
        
message(STATUS "BUILDING FOR DESKTOP...")

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

	#GLM
	ExternalProject_Add(GLM_DL 
			   URL https://github.com/g-truc/glm/releases/download/0.9.8.4/glm-0.9.8.4.zip
			   PREFIX GLM
			   CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND cmake -E echo "Skipping build step.")
	ExternalProject_Get_Property(GLM_DL source_dir)
	include_directories(${source_dir})

	#RapidXML
	ExternalProject_Add(XML_DL 
		URL https://downloads.sourceforge.net/project/rapidxml/rapidxml/rapidxml%201.13/rapidxml-1.13.zip?r=http%3A%2F%2Frapidxml.sourceforge.net%2F&ts=1508434717&use_mirror=astuteinternet
		PREFIX XML
		CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
		INSTALL_COMMAND cmake -E echo "Skipping install step."
		BUILD_COMMAND cmake -E echo "Skipping build step.")
	ExternalProject_Get_Property(XML_DL source_dir)
	include_directories(${source_dir})

	#poly2tri
	ExternalProject_Add(P2T_DL
		    GIT_REPOSITORY https://github.com/greenm01/poly2tri.git
		    PREFIX P2T
	            CONFIGURE_COMMAND cmake -E echo "Skipping configure step."
		    INSTALL_COMMAND cmake -E echo "Skipping install step."
		    BUILD_COMMAND cmake -E echo "Skipping build step.")
	ExternalProject_Get_Property(P2T_DL source_dir)
	include_directories(${source_dir})

	#SDL2
		ExternalProject_Add(SDL2_DL 
			   URL https://libsdl.org/release/SDL2-2.0.5.zip
			   PREFIX SDL2
			   INSTALL_COMMAND cmake -E echo "Skipping install step."
			   BUILD_COMMAND make)

		ExternalProject_Get_Property(SDL2_DL binary_dir)
		ExternalProject_Get_Property(SDL2_DL source_dir)

		include_directories(${source_dir}/include)

		#add_library(SDL2 SHARED IMPORTED)
		add_library(SDL2 STATIC IMPORTED)
		add_dependencies(SDL2 SDL2_DL)
                #set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${binary_dir}/libSDL2-2.0.so)
		set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${binary_dir}/libSDL2.a)
	#endif()

	#SDL2_image
		ExternalProject_Add(SDL2_image_DL 
			   URL https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.1.zip
			   PREFIX SDL2_image
			   CONFIGURE_COMMAND sh /home/ubuntu/SDL2_image/src/SDL2_image_DL/configure
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
	find_package(SDL2_ttf REQUIRED)
	if (SDL2TTF_FOUND)
	    include_directories(${SDL2TTF_INCLUDE_DIR})
	endif() 
	#endif()

	#Bullet Physics (v.3, but only using v.2 CPU-based code)
	ExternalProject_Add(Bullet 
		    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
		    PREFIX Bullet
		    CMAKE_ARGS 
		       #-DBUILD_SHARED_LIBS=ON 
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

	#add_library(linearMath SHARED IMPORTED)
	#add_library(common SHARED IMPORTED)
	#add_library(collision SHARED IMPORTED)
	#add_library(dynamics SHARED IMPORTED)
	#add_library(softBody SHARED IMPORTED)
	#add_library(inverseDynamics SHARED IMPORTED)
	add_library(linearMath STATIC IMPORTED)
	add_library(common STATIC IMPORTED)
	add_library(collision STATIC IMPORTED)
	add_library(dynamics STATIC IMPORTED)
	add_library(softBody STATIC IMPORTED)
	add_library(inverseDynamics STATIC IMPORTED)

	add_dependencies(linearMath Bullet)
	add_dependencies(common Bullet)
	add_dependencies(collision Bullet)
	add_dependencies(dynamics Bullet)
	add_dependencies(softBody Bullet)
	add_dependencies(inverseDynamics Bullet)

	#set_target_properties(linearMath PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/LinearMath/libLinearMath.so)
	#set_target_properties(common PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/Bullet3Common/libBullet3Common.so)
	#set_target_properties(collision PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletCollision/libBulletCollision.so)
	#set_target_properties(dynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletDynamics/libBulletDynamics.so)
	#set_target_properties(softBody PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletSoftBody/libBulletSoftBody.so)
	#set_target_properties(inverseDynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletInverseDynamics/libBulletInverseDynamics.so)
	set_target_properties(linearMath PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/LinearMath/libLinearMath.a)
	set_target_properties(common PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/Bullet3Common/libBullet3Common.a)
	set_target_properties(collision PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletCollision/libBulletCollision.a)
	set_target_properties(dynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletDynamics/libBulletDynamics.a)
	set_target_properties(softBody PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletSoftBody/libBulletSoftBody.a)
	set_target_properties(inverseDynamics PROPERTIES IMPORTED_LOCATION ${binary_dir}/src/BulletInverseDynamics/libBulletInverseDynamics.a)

	include_directories(${source_dir}/src)

	#entityx
	ExternalProject_Add(entityx 
		   GIT_REPOSITORY https://github.com/alecthomas/entityx.git
		   PREFIX entityx
		   CMAKE_ARGS 
			-DENTITYX_BUILD_SHARED=0
		   INSTALL_COMMAND cmake -E echo "Skipping install step."
		   BUILD_COMMAND make)

	ExternalProject_Get_Property(entityx binary_dir)
	ExternalProject_Get_Property(entityx source_dir)

	#add_library(project_entityx SHARED IMPORTED)
	add_library(project_entityx STATIC IMPORTED)
	add_dependencies(project_entityx entityx)
	#set_target_properties(project_entityx PROPERTIES IMPORTED_LOCATION ${binary_dir}/libentityx.so)
	set_target_properties(project_entityx PROPERTIES IMPORTED_LOCATION ${binary_dir}/libentityx.a)
	include_directories(${source_dir})

	#zeromq
	ExternalProject_Add(libzmq
		GIT_REPOSITORY https://github.com/zeromq/libzmq.git
		PREFIX libzmq
		INSTALL_COMMAND sudo make -j4 install
		BUILD_COMMAND make)	

	ExternalProject_Add(cppzmq
		GIT_REPOSITORY https://github.com/zeromq/cppzmq.git
		PREFIX cppzmq
		INSTALL_COMMAND cmake -E echo "Skipping install step."
		BUILD_COMMAND make)	

	#final setup
	aux_source_directory(core/src/ SOURCE)
	add_executable(Core ${SOURCE})
	include_directories(core/src)

        target_link_libraries(Core linearMath common collision dynamics softBody inverseDynamics)
        target_link_libraries(Core ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} 
"-L/usr/lib/x86_64-linux-gnu -lSDL2 -Wl,--no-undefined -lm -ldl -lasound -lm -ldl -lpthread -lpulse-simple -lpulse -lsndio -lX11 -lXext -lXcursor -lXinerama -lXi -lXrandr -lXss -lXxf86vm -lwayland-egl -lwayland-client -lwayland-cursor -lxkbcommon -lpthread -lrt"
                SDL2 SDL2_image SDL2_ttf
		linearMath common collision dynamics softBody inverseDynamics
		project_entityx)

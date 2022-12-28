## General
cmake_minimum_required(VERSION 3.13.4)

project(AbstractShaderEngine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0601 /EHsc /bigobj -DSHADERC_ENABLE_SHARED_CRT=ON")

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(lib/glfw)

# GLM
message(STATUS "Adding GLM library")
add_subdirectory(lib/glm)

find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS})

# core
message(STATUS "Adding main abstract shader library")
find_package(OpenMP REQUIRED)
add_library(core STATIC	src/core/core.cpp
						src/core/utility.cpp)
target_include_directories(core PUBLIC ${PROJECT_SOURCE_DIR}/include/core)
target_link_libraries(core PUBLIC	OpenMP::OpenMP_CXX
									glfw
									glm)

## render
message(STATUS "Adding render library")
add_library(render STATIC	src/render/opengl/opengl_core.cpp)
target_include_directories(render PUBLIC	
								${PROJECT_SOURCE_DIR}/include/render
								${PROJECT_SOURCE_DIR}/lib/stb
								${PROJECT_SOURCE_DIR}/lib/tiny_obj_loader)
target_link_libraries(render PUBLIC	core ${OPENGL_INCLUDE_DIRS})

## ray tracer
message(STATUS "Adding ray tracer library")
add_library(ray_tracer STATIC	src/ray_tracer/ray_tracer.cpp
								src/ray_tracer/ray_tracer_core.cpp)
target_include_directories(ray_tracer PUBLIC ${PROJECT_SOURCE_DIR}/include/ray_tracer)
target_link_libraries(ray_tracer PUBLIC core)

# Link Dependencies 

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC	core
												render 
												ray_tracer)

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/openmp)
endif()
## General
cmake_minimum_required(VERSION 3.13.4)
project(UnknownShaderEngine LANGUAGES C CXX)


# VULKAN
message(STATUS "Adding Vulkan library")
add_subdirectory(lib/vulkan)
set(vulkan-prebuilt_DIR "lib/vulkan/install/lib/cmake/vulkan-prebuilt")
find_package(vulkan-prebuilt)
if (NOT _FOUND AND NOT VULKAN-PREBUILT_FOUND AND NOT vulkan-prebuilt_FOUND)
    message(FATAL_ERROR "Vulkan not found")
endif()

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(lib/glfw)

# GLM
message(STATUS "Adding GLM library")
add_subdirectory(lib/glm)

## Render
add_library(Render STATIC 
                src/render.cpp 
                src/hittable_list.cpp 
                src/sphere.cpp
                src/camera.cpp
                src/material.cpp)

# Link Dependencies 

find_package(OpenMP REQUIRED)
target_include_directories(Render PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC 
                        Render 
                        OpenMP::OpenMP_CXX 
                        vulkan-prebuilt::vulkan-prebuilt 
                        glfw 
                        glm)

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/openmp)
endif()
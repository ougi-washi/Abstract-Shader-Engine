## General
cmake_minimum_required(VERSION 3.13.4)

project(AbstractShaderEngine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0601 /EHsc /bigobj -DSHADERC_ENABLE_SHARED_CRT=ON")

# VULKAN
message(STATUS "Attempting auto locate Vulkan using CMake......")
find_package(Vulkan)

if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
	set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
	STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

if(NOT Vulkan_FOUND)
	message(STATUS "Failed to locate Vulkan SDK, retrying again...")
	if(EXISTS "${VULKAN_PATH}")
		message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
	else()
		message("Error: Unable to locate Vulkan SDK. Please turn off auto locate option by specifying 'AUTO_LOCATE_VULKAN' as 'OFF'")
		message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt.")
		return()
	endif()
endif()

# Add any required preprocessor definitions here
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
# vulkan-1 library for build Vulkan application.
set(VULKAN_LIB_LIST "vulkan-1")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	include_directories(AFTER ${VULKAN_PATH}/Include)
	link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
endif()


# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(lib/glfw)

# GLM
message(STATUS "Adding GLM library")
add_subdirectory(lib/glm)

## CREATE ANOTHER MODULE, ADD THE DEPENDENCIES TO THE OTHER MODULE THEN BIND THEM TO THE MAIN RENDER LIB
## Render
add_library(Render STATIC	src/core.cpp
							src/render/vulkan.cpp
							src/render/vulkan_core.cpp)

target_include_directories(Render PUBLIC	
								${PROJECT_SOURCE_DIR}/include
								${PROJECT_SOURCE_DIR}/lib/stb
								${PROJECT_SOURCE_DIR}/lib/tiny_obj_loader)

target_link_libraries(Render PUBLIC  
							OpenMP::OpenMP_CXX
							shaderc_combinedd
							vulkan-1
							glfw 
							glm)

## Path tracer
add_library(RayTracer STATIC	src/ray_tracer/ray_tracer.cpp
								src/ray_tracer/ray_tracer_core.cpp)
target_link_libraries(RayTracer PUBLIC Render)

# Link Dependencies 

find_package(OpenMP REQUIRED)


add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC 
							Render 
							RayTracer)

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/openmp)
endif()
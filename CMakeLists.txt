# Abstract-shader-engine https://github.com/ougi-washi/Abstract-Shader-Engine

## General
cmake_minimum_required(VERSION 3.22.1)

project(Abstract_SHADER_ENGINE LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_GENERATOR)
    find_program(NINJA_EXECUTABLE ninja)
    if(NINJA_EXECUTABLE)
        set(CMAKE_GENERATOR "Ninja" CACHE INTERNAL "Ninja")
        message(STATUS "Ninja build system found. Using Ninja generator.")
    else()
        message(STATUS "Ninja build system not found. Using default generator.")
    endif()
endif()

# Directory setup
set(LIB_DIR "lib")
set(SRC_DIR "src")
set(BIN_DIR "bin")
set(RESOURCES_DIR "resources")
set(EXAMPLES_DIR "examples")

# GLSL
find_package(OpenGL REQUIRED)

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(${LIB_DIR}/glfw)

# STB
message(STATUS "Adding STB library")
include_directories(${LIB_DIR}/stb)


find_library(PULSE_SIMPLE_LIB pulse-simple)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse-simple libpulse)

# setup library macro
macro(setup_library arg_lib_name arg_source_path arg_include_path)
	message(STATUS "Adding ${arg_lib_name}")
	file(GLOB CURRENT_SRC_FILES	"${arg_source_path}/*.c" 
					"${arg_source_path}/*.cpp"
					"${arg_source_path}/**/*.c" 
					"${arg_source_path}/**/*.cpp")
	add_library(${arg_lib_name} STATIC	${CURRENT_SRC_FILES})
	target_include_directories(${arg_lib_name} PUBLIC ${arg_include_path})
endmacro()

# Main module
set(MAIN_MODULE_INCLUDES ${PROJECT_SOURCE_DIR}/${SRC_SRC} ${PROJECT_SOURCE_DIR}/${LIB_DIR})
setup_library(main_module ${SRC_DIR} "${MAIN_MODULE_INCLUDES}")
target_link_libraries(main_module PUBLIC glfw OpenGL::GL ${PULSEAUDIO_LIBRARIES})  

macro(setup_executable arg_exec_dir arg_exec_name modules)
    message(STATUS "Generating executable ${arg_exec_name}")
    add_executable(${arg_exec_name} "${arg_exec_dir}/${arg_exec_name}.c")
    target_link_libraries(${arg_exec_name} PUBLIC main_module)
    set_target_properties(${arg_exec_name} PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})
endmacro()

file(GLOB EXAMPLES 	"${EXAMPLES_DIR}/*.c" 
			"${EXAMPLES_DIR}/*.cpp"
			"${EXAMPLES_DIR}/**/*.c" 
			"${EXAMPLES_DIR}/**/*.cpp")

foreach(EXAMPLE ${EXAMPLES})
   	get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
   	message(STATUS "Adding strategy ${EXAMPLE_NAME}")
   	setup_executable(${EXAMPLES_DIR} ${EXAMPLE_NAME} main_module)
endforeach()

# Copy resources
file(COPY ${RESOURCES_DIR} DESTINATION ${PROJECT_SOURCE_DIR}/${BIN_DIR})



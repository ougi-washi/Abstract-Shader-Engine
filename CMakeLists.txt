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
set(STRATS_DIR "examples")
set(BIN_DIR "bin")

# GLSL
find_package(OpenGL REQUIRED)

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(${LIB_DIR}/glfw)

# STB
message(STATUS "Adding STB library")
include_directories(${LIB_DIR}/stb)


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
target_link_libraries(main_module PUBLIC glfw OpenGL::GL) 

macro(setup_executable arg_exec_dir arg_exec_name modules)
    message(STATUS "Generating executable ${arg_exec_name}")
    add_executable(${arg_exec_name} "${arg_exec_dir}/${arg_exec_name}.c")
    target_link_libraries(${arg_exec_name} PUBLIC main_module)
    set_target_properties(${arg_exec_name} PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})
endmacro()

file(GLOB STRATEGIES	"${STRATS_DIR}/*.c" 
                        "${STRATS_DIR}/*.cpp"
                        "${STRATS_DIR}/**/*.c" 
                        "${STRATS_DIR}/**/*.cpp")

foreach(STRATEGY ${STRATEGIES})
    get_filename_component(STRATEGY_NAME ${STRATEGY} NAME_WE)
    message(STATUS "Adding strategy ${STRATEGY_NAME}")
    setup_executable(${STRATS_DIR} ${STRATEGY_NAME} main_module)
endforeach()

# Add shaders and models to build tree
file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/*.glsl")
file(GLOB MODELS "${CMAKE_SOURCE_DIR}/*.obj")
foreach(_file IN LISTS SHADERS MODELS)
    configure_file(${_file} ${_file} COPYONLY)
endforeach()


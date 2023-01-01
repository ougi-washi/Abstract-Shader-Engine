## General
cmake_minimum_required(VERSION 3.24.1)

project(AbstractShaderEngine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

IF (WIN32)
	set (CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0601 /EHsc /bigobj")
ELSE()
ENDIF()

# Directory setup
set(LIB_DIR "lib")
set(INC_DIR "include")
set(SRC_DIR "src")

# setup library macro
macro(setup_library arg_lib_name arg_source_path arg_include_path)
	message(STATUS "Adding ${arg_lib_name}")
	file(GLOB GLAD_SRC_FILES	"${arg_source_path}/*.c" 
								"${arg_source_path}/*.cpp"
								"${arg_source_path}/**/*.c" 
								"${arg_source_path}/**/*.cpp")
	add_library(${arg_lib_name} STATIC	${GLAD_SRC_FILES})
	target_include_directories(${arg_lib_name} PUBLIC ${arg_include_path})
endmacro()

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(lib/glfw)

# GLM
message(STATUS "Adding GLM library")
add_subdirectory(lib/glm)

# GLAD
setup_library(glad ${LIB_DIR}/glad/src ${PROJECT_SOURCE_DIR}/lib/glad/include)

# CORE
setup_library(core ${SRC_DIR}/core ${PROJECT_SOURCE_DIR}/${INC_DIR}/core ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(core PUBLIC glad glfw glm)

# RENDER
setup_library(render ${SRC_DIR}/render ${PROJECT_SOURCE_DIR}/${INC_DIR}/render)
target_link_libraries(render PUBLIC	core)


# RAY_TRACER
setup_library(ray_tracer ${SRC_DIR}/ray_tracer ${PROJECT_SOURCE_DIR}/${INC_DIR}/ray_tracer)
target_link_libraries(ray_tracer PUBLIC core)

# Executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC	render 
												ray_tracer)

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
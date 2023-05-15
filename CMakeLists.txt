## General
cmake_minimum_required(VERSION 3.22.1)

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
set(BIN_DIR "binaries")

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

# GLFW
#message(STATUS "Adding GLFW library")
#add_subdirectory(${LIB_DIR}/glfw)

# GLM
#message(STATUS "Adding GLM library")
#add_subdirectory(${LIB_DIR}/glm)

# ASSIMP
#message(STATUS "Adding ASSIMP library")
#add_subdirectory(${LIB_DIR}/assimp)

# GLAD
#setup_library(glad ${LIB_DIR}/glad/src ${PROJECT_SOURCE_DIR}/${LIB_DIR}/glad/include)

# RAYLIB
message(STATUS "Adding Raylib library")
add_subdirectory(${LIB_DIR}/raylib)

# CORE
set(CORE_INCLUDES ${PROJECT_SOURCE_DIR}/${INC_DIR}/core ${PROJECT_SOURCE_DIR}/${LIB_DIR})
setup_library(core ${SRC_DIR}/core "${CORE_INCLUDES}")
target_link_libraries(core PUBLIC raylib)

# Executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC core)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
# AbstractShaderEngine - Jed Fakhfekh - https://github.com/ougi-washi

## General
cmake_minimum_required(VERSION 3.22.1)

project(AbstractShaderEngine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

IF (WIN32)
	set (CMAKE_CXX_FLAGS "-D_WIN32_WINNT=0x0601 /EHsc /bigobj -std=c17 /std:c17")
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

find_package(Threads REQUIRED)

# GLFW
message(STATUS "Adding GLFW library")
add_subdirectory(${LIB_DIR}/glfw)

# STB
message(STATUS "Adding STB library")
include_directories(${LIB_DIR}/stb)

# VULKAN
message(STATUS "Adding Vulkan library")
find_package(Vulkan REQUIRED)
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
if(WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()
# vulkan-1 library for build Vulkan application.
set(VULKAN_LIB_LIST "vulkan-1")
include_directories(AFTER ${VULKAN_PATH}/Include)
link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)


# MAIN MODULE
set(MAIN_MODULE_INCLUDES ${PROJECT_SOURCE_DIR}/${INC_DIR} ${PROJECT_SOURCE_DIR}/${LIB_DIR})
setup_library(main_module ${SRC_DIR} "${MAIN_MODULE_INCLUDES}")
target_include_directories(main_module PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(main_module PUBLIC glfw ${Vulkan_LIBRARIES} shaderc_sharedd)

# Executable
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} PUBLIC main_module)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})

## Definitions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

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

message(STATUS "Adding PULSEAUDIO library")
find_library(PULSE_SIMPLE_LIB pulse-simple)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse-simple libpulse)

message(STATUS "Adding PORTAUDIO library")
if (PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS)
  # in cache already
  set(PORTAUDIO_FOUND TRUE)
else (PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS)
  if (NOT WIN32)
   include(FindPkgConfig)
   pkg_check_modules(PORTAUDIO2 portaudio-2.0)
  endif (NOT WIN32)

  if (PORTAUDIO2_FOUND)
    set(PORTAUDIO_INCLUDE_DIRS
      ${PORTAUDIO2_INCLUDE_DIRS}
    )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(PORTAUDIO_LIBRARIES "${PORTAUDIO2_LIBRARY_DIRS}/lib${PORTAUDIO2_LIBRARIES}.dylib")
    else (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
      set(PORTAUDIO_LIBRARIES
        ${PORTAUDIO2_LIBRARIES}
      )
    endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(PORTAUDIO_VERSION
      19
    )
    set(PORTAUDIO_FOUND TRUE)
  else (PORTAUDIO2_FOUND)
    find_path(PORTAUDIO_INCLUDE_DIR
      NAMES
        portaudio.h
      PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )

    find_library(PORTAUDIO_LIBRARY
      NAMES
        portaudio
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    find_path(PORTAUDIO_LIBRARY_DIR
      NAMES
        portaudio
      PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
    )

    set(PORTAUDIO_INCLUDE_DIRS
      ${PORTAUDIO_INCLUDE_DIR}
    )
    set(PORTAUDIO_LIBRARIES
      ${PORTAUDIO_LIBRARY}
    )

    set(PORTAUDIO_LIBRARY_DIRS
      ${PORTAUDIO_LIBRARY_DIR}
    )

    set(PORTAUDIO_VERSION
      18
    )

    if (PORTAUDIO_INCLUDE_DIRS AND PORTAUDIO_LIBRARIES)
       set(PORTAUDIO_FOUND TRUE)
    endif (PORTAUDIO_INCLUDE_DIRS AND PORTAUDIO_LIBRARIES)

    if (PORTAUDIO_FOUND)
      if (NOT Portaudio_FIND_QUIETLY)
        message(STATUS "Found Portaudio: ${PORTAUDIO_LIBRARIES}")
      endif (NOT Portaudio_FIND_QUIETLY)
    else (PORTAUDIO_FOUND)
      if (Portaudio_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find Portaudio")
      endif (Portaudio_FIND_REQUIRED)
    endif (PORTAUDIO_FOUND)
  endif (PORTAUDIO2_FOUND)


  # show the PORTAUDIO_INCLUDE_DIRS and PORTAUDIO_LIBRARIES variables only in the advanced view
  mark_as_advanced(PORTAUDIO_INCLUDE_DIRS PORTAUDIO_LIBRARIES)

endif (PORTAUDIO_LIBRARIES AND PORTAUDIO_INCLUDE_DIRS)


# setup library macro
macro(setup_library arg_lib_name arg_source_path arg_include_path)
	message(STATUS "Adding ${arg_lib_name}")
	file(GLOB CURRENT_SRC_FILES	"${arg_source_path}/*.c" 
					"${arg_source_path}/*.cpp"
					"${arg_source_path}/**/*.c" 
					"${arg_source_path}/**/*.cpp")
	add_library(${arg_lib_name} STATIC	${CURRENT_SRC_FILES})
	target_include_directories(${arg_lib_name} PUBLIC ${arg_include_path})
    target_compile_definitions(${arg_lib_name} PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/${RESOURCES_DIR}/")
endmacro()


# Main module
set(MAIN_MODULE_INCLUDES ${PROJECT_SOURCE_DIR}/${SRC_SRC} ${PROJECT_SOURCE_DIR}/${LIB_DIR} ${PROJECT_SOURCE_DIR}/${LIB_DIR}/portaudio/include)
setup_library(main_module ${SRC_DIR} "${MAIN_MODULE_INCLUDES}")
target_link_libraries(main_module PUBLIC glfw OpenGL::GL portaudio fftw3  ${PULSEAUDIO_LIBRARIES})  

macro(setup_executable arg_exec_dir arg_exec_name modules)
    message(STATUS "Generating executable ${arg_exec_name}")
    add_executable(${arg_exec_name} "${arg_exec_dir}/${arg_exec_name}.c")
    target_link_libraries(${arg_exec_name} PUBLIC main_module)
    target_compile_definitions(${arg_exec_name} PRIVATE RESOURCES_DIR="${CMAKE_SOURCE_DIR}/${RESOURCES_DIR}/")
    set_target_properties(${arg_exec_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/${BIN_DIR})
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


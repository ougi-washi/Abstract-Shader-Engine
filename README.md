# Abstract Shader Engine
A small vulkan engine for generating scenes with SDF and shapes.

## Building
- This project has submodule dependencies, so make sure to use `git clone --recursive`
- Build normally with `cmake`
- Side note: if you download it without submodules, use `git submodule update`
- Make sure the binaries are right under `binaries/` and not under `binaries/debug/` as some cmake configurations will force it to be so

## Requirements
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [CMake](https://cmake.org/download/)

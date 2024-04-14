# Abstract Shader Engine
A small vulkan engine for generating scenes with SDF and shapes.
![](docs/gifs/scene_sample_001.gif)

## Functionalities
- SDF library (3D and 2D)
- SDF interaction (eg. smooth union) between different object meshes 
- Shader monitors (runtime shader recompilation)
- Shader serialization (caching)
- Text (ASCII only, shaders approach)
- Runtime console commands
- Input handlers
- Timer handlers
- Scene serialization (in progress)
 
## Building
- This project has submodule dependencies, so make sure to use `git clone --recursive`
- Build normally with `cmake`
- Side note: if you download it without submodules, use `git submodule update`
- Make sure the built binaries are right under `binaries/` and not under `binaries/debug/` as some cmake configurations will force it to be so

## Requirements
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [CMake](https://cmake.org/download/)

# Abstract Shader Engine
A small vulkan engine for generating scenes with SDF and shapes.
![](docs/gifs/scene_sample_001.gif)

[Full video](https://www.youtube.com/watch?v=tBGiiRAWWSA)

![](docs/gifs/instance_sdf_001.gif)

[Full video](https://youtu.be/t9NroLUA1OM)

## Functionalities
- SDF library (3D and 2D)
- SDF interaction (eg. smooth union between different object meshes in separate drawcalls)
- Shader monitors (runtime shader recompilation)
- Shader serialization (caching)
- Text (ASCII only, shaders approach)
- Runtime console commands
- Input handlers
- Timer handlers
- Scene serialization (in progress)
 
## Building
(Currently it only supports windows, Linux porting is work in progress, it should be ready soon)
- This project has submodule dependencies, so make sure to use `git clone --recursive`
- Build normally with `cmake`
- Side note: if you download it without submodules, use `git submodule update`
- Make sure the built binaries are right under `binaries/` and not under `binaries/debug/` as some cmake configurations will force it to be so

## Requirements
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [CMake](https://cmake.org/download/)

## TODO
- Finish porting to Linux
- Audio input/output
- Rework command buffers to have a more cache-aligned draw call
- Finish scene serialization
- Improve clean-ups when exiting (memory leaks)
- Add more functions to the console command
- Add helper functionality/auto-complete to the console command
- Set up a proper `tests/` folder with multiple examples
- LOD system

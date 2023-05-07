# Abstract Shader Engine
A small C/C++ graphics/compute engine for path tracing and generating scenes with shapes.

This project is developed with a set of rules in mind:
- For artists and programmers
- Robustness
- Performance
- Readability
- Modularity
- No forced architecture
- API Exposure to both low and high level programming
- EMACS-like experience (where you may not need a mouse to progress faster)
- Portability for embedded systems (starting with raspberry pi lite os) to prove that a couple of RTX's are not required to make nice looking art

## Requirements
- [CMake](https://cmake.org/download/)

## TODOs
- Remove all utils from the engine_core file (eg. string conversions)
- Handle per-shader multiple textures allocation and deallocation
- Add render thread
- Add caching to the entities and matching files and create a thread that parses the files and rebuild the entity if any change has happened to the cached file, then update the cache.
- Add entities and shader file changes thread (check every few frames and update the scene according to the change)
- Drag and drop adds entities to the world json and updates the scene
- Add Vulkan support back
- Add EGLS support
- Separate module : expose a simple script method in C++ that could make it possible to interact with other world entities's scripts (no hot-reload, just copying what's in the files during compilation)
- Separate module : Network shader compilation
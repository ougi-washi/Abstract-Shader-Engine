## Abstract-shader-engine
OpenGL shader engine in C.

### Features
* Handle buffer objects
* Handle uniforms
* Handle SDF objects and operations

### Building
CMAke 3.22 is required.
```bash
mkdir build
cd build
cmake ..
make
```

### Usage
```c
#include "src/engine.h"

i32 main() {
    engine_t engine = {0};
    engine_init(&engine, 1280, 720, "Abstract Shader Engine");

    shader_t* main_shader = shader_load(&engine, "vert.glsl", "frag.glsl");

    while (!engine_should_close(&engine)) {
        engine_poll_events(&engine);
        engine_update(&engine);
        
        shader_use(&engine, main_shader, true);
        engine_render(&engine);
        engine_swap_buffers(&engine);
    }
    
    render_buffer_cleanup(&buffer1);
    engine_cleanup(&engine);
    return 0;
}
```

Side note: 
Resources such as shaders should be under "resources/", they get copied to the build directory automatically by cmake.


### License
MIT License


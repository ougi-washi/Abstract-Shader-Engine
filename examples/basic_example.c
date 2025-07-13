#include "src/engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


i32 main() {

    audio_init();
    
    engine_t engine = {0};
    bool engine_success = engine_init(&engine, 1280, 720, "Abstract Shader Engine");
    assert(engine_success);
    
    //audio_device_t audio_device = {0};
    //audio_init(&audio_device);

    shader_t* main_shader = shader_load(&engine, "vert.glsl", "frag_main.glsl");
    assert(main_shader);

    // buffer 1 setup
    shader_t* buffer1_shader = shader_load(&engine, "vert.glsl", "frag_buffer1.glsl");
    assert(buffer1_shader);
    render_buffer_t buffer1 = {0};
    bool render_buffer_res = render_buffer_create(&buffer1, 512, 512);
    assert(render_buffer_res);
   
    // mesh setup
    shader_t* mesh_shader = shader_load(&engine, "vert_mesh.glsl", "frag_mesh.glsl");
    assert(mesh_shader);
    model_t model = {0};
    model_load_obj(&model, "cube.obj", mesh_shader);

    while (!engine_should_close(&engine)) {

        // input
        engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));

        engine_update(&engine);
        
        uniform_set_vec3(&engine, "amps", audio_get_amplitudes());

        shader_use(&engine, buffer1_shader, true);
        render_buffer_bind(&buffer1);
        engine_clear();
        //engine_render_quad(&engine);
        model_render(&engine, &model);
        render_buffer_unbind();
        
        shader_use(&engine, main_shader, true);
        uniform_set_texture(&engine, "buffer1", buffer1.texture);
        engine_render(&engine);
        engine_swap_buffers(&engine);
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    model_cleanup(&model);
    audio_cleanup();
    render_buffer_cleanup(&buffer1);
    engine_cleanup(&engine);
    return 0;
}

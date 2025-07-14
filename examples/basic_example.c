#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WIDTH 1920
#define HEIGHT 1080

i32 main() {
    as_engine engine = {0};
    bool as_engine_success = as_engine_init(&engine, WIDTH, HEIGHT, "Abstract Shader Engine");
    assert(as_engine_success);
   
    as_shader* main_shader = as_shader_load(&engine, "vert.glsl", "frag_main.glsl");
    assert(main_shader);

    // buffer 1 setup
    as_shader* buffer1_shader = as_shader_load(&engine, "vert.glsl", "frag_buffer1.glsl");
    assert(buffer1_shader);
    as_render_buffer buffer1 = {0};
    bool as_render_buffer_res = as_render_buffer_create(&buffer1, WIDTH, HEIGHT);
    assert(as_render_buffer_res);
   
    // mesh setup
    as_shader* as_mesh_shader_0 = as_shader_load(&engine, "vert_mesh.glsl", "frag_mesh.glsl");
    as_shader** as_mesh_shaders = &as_mesh_shader_0;
    assert(as_mesh_shaders);
    as_model model = {0};
    as_model_load_obj(&model, "cube.obj", as_mesh_shaders, 1);
    
    as_audio_init();
   
    while (!as_engine_should_close(&engine)) {

        // input
        as_engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        as_engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));

        as_engine_update(&engine);
        
        as_uniform_set_vec3(&engine, "amps", as_audio_get_amplitudes());

        as_shader_use(&engine, buffer1_shader, true);
        as_render_buffer_bind(&buffer1);
        as_engine_clear();
        //as_engine_render_quad(&engine);
        const as_vec3 rot_angle = {0,0,get_delta_time(&engine)};
        as_model_rotate(&model, &rot_angle);
        as_model_render(&engine, &model);
        as_render_buffer_unbind();
        
        as_shader_use(&engine, main_shader, true);
        as_uniform_set_texture(&engine, "buffer1", buffer1.texture);
        as_engine_render(&engine);
        as_engine_swap_buffers(&engine);
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    as_model_cleanup(&model);
    as_audio_cleanup();
    as_render_buffer_cleanup(&buffer1);
    as_engine_cleanup(&engine);
    return 0;
}

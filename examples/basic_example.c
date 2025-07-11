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
    
    while (!engine_should_close(&engine)) {
        engine_poll_events(&engine);
        i32 keys[] = {GLFW_KEY_ESCAPE}; //{GLFW_KEY_LEFT_ALT, GLFW_KEY_F4};
        engine_check_exit_keys(&engine, keys, sizeof(keys) / sizeof(i32));
        engine_update(&engine);
        //const f32 amplitude = audio_get_amplitude(&audio_device);
        //const f32 frequency = audio_get_frequency(&audio_device);
        //printf("Amplitude: %f, Frequency: %f\n", amplitude, frequency);
        //uniform_set_float(&engine, "amplitude", amplitude);
        //uniform_set_float(&engine, "frequency", frequency);
        shader_use(&engine, buffer1_shader, true);
        render_buffer_bind(&buffer1);
        engine_clear();
        engine_render_quad(&engine);
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
    
    //audio_cleanup(&audio_device);
    render_buffer_cleanup(&buffer1);
    engine_cleanup(&engine);
    return 0;

/*
    
    printf("Engine initialized successfully!\n");
    printf("Controls:\n");
    printf("  ESC - Exit\n");
    printf("  R - Reload shaders\n");
    printf("  1-4 - Switch render buffers\n");
    printf("  SPACE - Toggle wireframe mode\n");
    
    // Create example shader files

    // Load a custom shader for hot-reloading
    shader_t custom_shader = {0};
    bool use_custom_shader = false;
    
    if (shader_load(&custom_shader, "./resources/vert.glsl", "./resources/frag.glsl")) {
        printf("Custom shader loaded successfully!\n");
        use_custom_shader = true;
        
        // Add custom shader to engine's shader list
        engine.shaders = realloc(engine.shaders, (engine.shader_count + 1) * sizeof(shader_t));
        engine.shaders[engine.shader_count] = custom_shader;
        engine.shader_count++;
    }
    
    // Create some render buffers
    render_buffer_t buffer1, buffer2;
    if (render_buffer_create(&buffer1, 512, 512)) {
        printf("Buffer 1 created (512x512)\n");
    }
    if (render_buffer_create(&buffer2, 256, 256)) {
        printf("Buffer 2 created (256x256)\n");
    }
    
    // Load a model (if available)
    model_t test_model = {0};
    bool model_loaded = false;
    if (model_load_obj(&test_model, "./resources/cube.obj")) {
        printf("Test model loaded successfully!\n");
        model_loaded = true;
    }
    
    // Set some initial uniforms
    uniform_set_vec3(&engine, "light_pos", vec3_create(5.0f, 5.0f, 5.0f));
    uniform_set_vec3(&engine, "camera_pos", vec3_create(0.0f, 0.0f, 3.0f));
    uniform_set_float(&engine, "metallic", 0.5f);
    uniform_set_float(&engine, "roughness", 0.3f);
    
    i32 current_buffer = 0;
    bool wireframe = false;
    
    // Main render loop
    while (!engine_should_close(&engine)) {
        engine_poll_events(&engine);
        engine_update(&engine);
        
        // Handle keyboard input
        if (engine.keys[GLFW_KEY_ESCAPE]) {
            glfwSetWindowShouldClose(engine.window, GLFW_TRUE);
        }
        
        if (engine.keys[GLFW_KEY_R]) {
            printf("Manually reloading shaders...\n");
            for (uint32_t i = 0; i < engine.shader_count; i++) {
                shader_reload_if_changed(&engine.shaders[i]);
            }
            engine.keys[GLFW_KEY_R] = false; // Prevent continuous reloading
        }
        
        if (engine.keys[GLFW_KEY_SPACE]) {
            wireframe = !wireframe;
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            engine.keys[GLFW_KEY_SPACE] = false;
        }
        
        // Switch between buffers
        if (engine.keys[GLFW_KEY_1]) current_buffer = 0;
        if (engine.keys[GLFW_KEY_2]) current_buffer = 1;
        if (engine.keys[GLFW_KEY_3]) current_buffer = 2;
        if (engine.keys[GLFW_KEY_4]) current_buffer = 3;
        
        // Update some dynamic uniforms
        uniform_set_vec2(&engine, "mouse_norm", vec2_create(
            engine.mouse_x / engine.window_width,
            1.0f - engine.mouse_y / engine.window_height
        ));
        
        uniform_set_float(&engine, "wave_freq", 5.0f + sin(engine.time) * 2.0f);
        uniform_set_vec3(&engine, "color_shift", vec3_create(
            sin(engine.time * 0.5f) * 0.5f + 0.5f,
            cos(engine.time * 0.3f) * 0.5f + 0.5f,
            sin(engine.time * 0.7f) * 0.5f + 0.5f
        ));
        
        // Render to buffer 1
        render_buffer_bind(&buffer1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (use_custom_shader) {
            shader_use(&custom_shader);
            uniform_apply_all(&engine, &custom_shader);
        } else {
            shader_use(&engine.default_shader);
            uniform_apply_all(&engine, &engine.default_shader);
        }
        
        // Render fullscreen quad or model
        if (model_loaded && engine.keys[GLFW_KEY_M]) {
            model_render(&test_model);
        } else {
            glBindVertexArray(engine.quad_vao);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
        }
        
        render_buffer_unbind();
        
        // Render to buffer 2 (using buffer 1 as input)
        render_buffer_bind(&buffer2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use buffer 1 as texture input
        uniform_set_texture(&engine, "texture0", buffer1.texture);
        
        shader_use(&engine.default_shader);
        uniform_apply_all(&engine, &engine.default_shader);
        
        glBindVertexArray(engine.quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        
        render_buffer_unbind();
        
        // Render to main screen
        glViewport(0, 0, engine.window_width, engine.window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Choose which buffer to display
        GLuint display_texture = 0;
        switch (current_buffer) {
            case 0: display_texture = 0; break; // No texture (default shader)
            case 1: display_texture = buffer1.texture; break;
            case 2: display_texture = buffer2.texture; break;
            default: display_texture = 0; break;
        }
        
        if (display_texture) {
            uniform_set_texture(&engine, "texture0", display_texture);
        }
        
        if (use_custom_shader && current_buffer == 0) {
            shader_use(&custom_shader);
            uniform_apply_all(&engine, &custom_shader);
        } else {
            shader_use(&engine.default_shader);
            uniform_apply_all(&engine, &engine.default_shader);
        }
        
        glBindVertexArray(engine.quad_vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        
        engine_swap_buffers(&engine);
        
        // Print some debug info occasionally
        if (engine.frame_count % 300 == 0) {
            printf("Frame %d, Time: %.2f, FPS: %.1f\n", 
                   engine.frame_count, engine.time, 1.0 / engine.delta_time);
        }
    }
    
    // Cleanup
    if (model_loaded) {
        model_cleanup(&test_model);
    }
    
    render_buffer_cleanup(&buffer1);
    render_buffer_cleanup(&buffer2);
    
    if (use_custom_shader) {
        shader_cleanup(&custom_shader);
    }
    
    engine_cleanup(&engine);
    
    printf("Engine shutdown complete.\n");

*/

    return 0;
}

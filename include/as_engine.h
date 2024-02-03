// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#include "core/as_render.h"
#include "core/as_shader.h"
#include "core/as_shapes.h"
#include "defines/as_global.h"
#include "as_memory.h"

extern void as_engine_init();
extern void as_engine_clear();
extern bool as_engine_should_loop();
extern void as_engine_draw(); // generally at the end of the engine loop
extern void as_engine_set_scene(as_scene* scene);
extern void as_engine_reset_scene();

extern f64 as_get_time();
extern f64 as_get_delta_time();

extern as_texture* as_texture_create(const char* texture_path);
extern as_shader* as_shader_create(const char* vertex_shader_path, const char* fragment_shader_path);
extern as_object* as_object_create(as_shape* shape, as_shader* shader);
extern as_object* as_object_create_with_tick(as_shape* shape, as_shader* shader, void tick_func_ptr(as_object*, const f64));
extern as_camera* as_camera_create(const as_vec3* position, const as_vec3* target);
extern void as_camera_set_view(as_camera* camera, const as_camera_type type);

extern sz as_assign_texture_to_shader(as_shader* shader, as_texture* texture);

extern bool as_is_pressed(const i32 key);
extern void as_input_loop_tick();
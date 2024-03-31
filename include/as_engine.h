// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_window.h"
#include "core/as_render.h"
#include "core/as_shader.h"
#include "core/as_shapes.h"
#include "core/as_content.h"
#include "core/as_ui.h"
#include "defines/as_global.h"
#include "as_memory.h"

extern void as_engine_init();
extern void as_engine_clear();
extern bool as_engine_should_loop();
extern void as_engine_draw(); // generally at the end of the engine loop
extern void as_engine_set_scene(as_scene* scene);
extern as_render* as_engine_get_render();
extern struct as_content* as_engine_get_content();
extern void as_engine_reset_scene();

extern f64 as_get_time();
extern f64 as_get_delta_time();

extern as_texture* as_texture_create(const char* texture_path);
extern as_shader* as_shader_create(const char* vertex_shader_path, const char* fragment_shader_path);
extern as_object* as_object_create(as_shape* shape, as_shader* shader);
extern as_object* as_object_create_with_tick(as_shape* shape, as_shader* shader, void tick_func_ptr(as_object*, const f64));
extern as_camera* as_camera_create(const as_vec3* position, const as_vec3* target);
extern as_screen_object* as_screen_object_create(const char* fragment_shader_path);
extern void as_camera_set_view(as_camera* camera, const as_camera_type type);
extern as_asset* as_asset_register(void* ptr, const as_asset_type type);

extern as_ui_text* as_ui_text_create(const char* text, const u32 font_size, const f32 spacing, as_texture* font_texture);
extern as_ui_text* as_ui_text_create_with_tick(const char* text, const u32 font_size, const f32 spacing, as_texture* font_texture, void tick_func_ptr(as_ui_text*, const f64));

extern sz as_assign_texture_to_screen_object(as_screen_object* object, as_texture* texture);
extern sz as_assign_texture_to_shader(as_shader* shader, as_texture* texture);

extern bool as_is_pressed(const i32 key);
extern bool as_is_released(const i32 key);
extern void as_input_loop_tick();

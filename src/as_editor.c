// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_editor.h"
#include "core/as_content.h"

void as_rotate_object1(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, 0., -cos(as_get_time() * .5) * 1., 0.));
}

void as_rotate_object2(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, 0., sin(as_get_time() * 3.) * 2.));
}

void as_rotate_object3(as_object* object, const f64 delta_time)
{
	as_object_set_translation(object, AS_VEC_PTR(as_vec3, 0., cos(as_get_time() * 2.) * 3.));
}

void as_update_ui_text_fps(as_ui_text* ui_text, const f64 delta_time)
{
	const f64 fps = as_render_get_fps(as_engine_get_render());
	const f64 render_delta_time = as_render_get_delta_time(as_engine_get_render());
	char fps_text[128] = "";
	sprintf(fps_text, "delta-time: %.3f | fps: %.1f", render_delta_time, fps);
	as_ui_text_set_text(ui_text, fps_text);
}

void as_editor_set_default_scene()
{
	as_scene* editor_scene = as_scene_create(as_engine_get_render(), AS_PATH_DEFAULT_SCENE);
	as_engine_set_scene(editor_scene);

	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, -15.f, -15.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	as_shape* shape_sphere = as_generate_sphere(.92, 8, 8);
	as_asset_register(shape_sphere, AS_ASSET_TYPE_SHAPE); // registering it as asset to make sure it's cleared when shutting down the engine 

	as_object* object1 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object1);
	as_object_set_translation(object1, AS_VEC_PTR(as_vec3, 0., 1.3, 0.));
	as_object_set_instance_count(object1, 2000);

	as_object* object2 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object2);
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, 0., -1.3, 0.));
	as_object_set_instance_count(object2, 2000);

	as_object* object3 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object3);
	as_object_set_instance_count(object3, 2000);

	//as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	//as_screen_object* screen_obj_test = as_screen_object_create(NULL);
	//as_assign_texture_to_screen_object(screen_obj_test, texture);
	
	as_texture* font_texture = as_texture_create(AS_PATH_DEFAULT_UI_TEXT_TEXTURE);
	as_ui_text* engine_title_text = as_ui_text_create("Abstract shader engine", 10, 2.5f, font_texture);
	as_screen_object_set_position(engine_title_text, AS_VEC_PTR(as_vec2, .1f, .1f));

	as_ui_text* fps_text = as_ui_text_create_with_tick("fps: -", 10, 2.5f, font_texture, &as_update_ui_text_fps);
	//as_ui_text* fps_text =  as_ui_text_create("Abstract", 10, 2.5f, font_texture);
	as_screen_object_set_position(fps_text, AS_VEC_PTR(as_vec2, .1f, .15f));
}

bool as_editor_should_loop()
{
	return as_engine_should_loop();
}

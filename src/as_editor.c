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

void as_editor_set_default_scene()
{
	as_scene* editor_scene = as_scene_create(as_engine_get_render(), AS_PATH_DEFAULT_SCENE);
	as_engine_set_scene(editor_scene);

	as_camera* camera = as_camera_create(AS_VEC_PTR(as_vec3, -15.f, -15.f, 10.f), AS_VEC_PTR(as_vec3, 0.f, 0.f, 0.f));
	as_camera_set_view(camera, AS_CAMERA_FREE);

	as_shader* shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_DEFAULT_FRAG_SHADER);
	
	as_shape* shape_sphere = as_generate_sphere(1., 8, 8);
	as_asset_register(shape_sphere, AS_ASSET_TYPE_SHAPE); // registering it as asset to make sure it's cleared when shutting down the engine 

	as_object* object1 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object1);
	as_object_set_translation(object1, AS_VEC_PTR(as_vec3, 0., 1.3, 0.));

	as_object* object2 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object2);
	as_object_set_translation(object2, AS_VEC_PTR(as_vec3, 0., -1.3, 0.));

	as_object* object3 = as_object_create_with_tick(shape_sphere, shader, &as_rotate_object3);

	as_shader* gray_shader = as_shader_create(AS_PATH_DEFAULT_VERT_SHADER, AS_PATH_EMPTY_GRAY_FRAG_SHADER);
	as_shape* shape_cube = as_generate_cube();
	as_asset_register(shape_cube, AS_ASSET_TYPE_SHAPE); // registering it as asset to make sure it's cleared when shutting down the engine 

	as_object* object4 = as_object_create(shape_cube, gray_shader);
	as_object* object5 = as_object_create(shape_cube, gray_shader);
	as_object* object6 = as_object_create(shape_cube, gray_shader);
	as_object_set_translation(object4, AS_VEC_PTR(as_vec3, 0., 0., -5.));
	as_object_set_translation(object5, AS_VEC_PTR(as_vec3, 0., 3., -5.));
	as_object_set_translation(object6, AS_VEC_PTR(as_vec3, 0., -3., -5.));
		
	as_texture* texture = as_texture_create(AS_PATH_DEFAULT_TEXTURE);
	as_screen_object* screen_obj_test = as_screen_object_create(NULL);
	as_assign_texture_to_screen_object(screen_obj_test, texture);
}

bool as_editor_should_loop()
{
	return as_engine_should_loop();
}

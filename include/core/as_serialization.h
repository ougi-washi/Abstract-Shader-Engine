// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_math.h"
#include "as_utility.h"
#include "core/as_render.h"
#include "core/as_render_queue.h"
#include "core/as_shapes.h"
#include "defines/as_global.h"

typedef struct as_serialized_shader
{
	char filename_vertex[AS_MAX_PATH_SIZE];
	char filename_fragment[AS_MAX_PATH_SIZE];

	//struct as_shader_uniforms_32 uniforms; resolve this later
} as_serialized_shader;

typedef struct as_serialized_object
{
	as_transform transform;
	u32 instance_count;
	as_shape shape;
	as_serialized_shader shader;
	i32 scene_gpu_index; // index of the object in the GPU scene 

} as_serialized_object;
AS_ARRAY_DECLARE(as_serialized_scene_objects, AS_MAX_SCENE_CAMERAS, as_serialized_object);

typedef struct as_serialized_light
{
	as_vec3 position;
	as_vec3 color;
	f32 radius;

} as_serialized_light;
AS_ARRAY_DECLARE(as_serialized_scene_lights, AS_MAX_SCENE_LIGHTS, as_serialized_light);

typedef struct as_serialized_camera
{
	as_vec3 position;
	as_vec3 target;
	as_vec3 up;
	f32 fov;
	as_camera_type type;
	f64 movement_speed;

	as_vec3 cached_direction;
} as_serialized_camera;
AS_ARRAY_DECLARE(as_serialized_scene_cameras, AS_MAX_SCENE_LIGHTS, as_serialized_camera);

typedef struct as_serialized_scene 
{
	char path[AS_MAX_PATH_SIZE];
	as_serialized_scene_objects objects;
	as_serialized_scene_lights lights;
	as_serialized_scene_cameras cameras;

} as_serialized_scene;

extern void as_serialize_shader(as_shader* shader, as_serialized_shader* serialized_shader);
extern void as_deserialize_shader(as_shader* shader, as_serialized_shader* serialized_shader, as_render* render, as_render_queue* render_queue);

extern void as_serialize_object(as_object* object, as_serialized_object* serialized_object);
extern void as_deserialize_object(as_object* object, as_serialized_object* serialized_object, as_render* render, as_render_queue* render_queue);

extern void as_serialize_light(as_light* light, as_serialized_light* serialized_light);
extern void as_deserialize_light(as_light* light, as_serialized_light* serialized_light, as_render* render, as_render_queue* render_queue);

extern void as_serialize_camera(as_camera* camera, as_serialized_camera* serialized_camera);
extern void as_deserialize_camera(as_camera* camera, as_serialized_camera* serialized_camera, as_render* render, as_render_queue* render_queue);

extern void as_serialize_scene(as_scene* scene, as_serialized_scene* serialized_scene);
extern void as_deserialize_scene(as_scene* scene, as_serialized_scene* serialized_scene, as_render* render, as_render_queue* render_queue);
extern void as_serialized_scene_destroy(as_serialized_scene* serializaed_scene);

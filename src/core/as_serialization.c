// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_serialization.h"
#include "core/as_render_queue.h"
#include "as_memory.h"

void as_serialize_shader(as_shader* shader, as_serialized_shader* serialized_shader)
{
	AS_ASSERT(shader, "Cannot serialize shader, invalid shader");
	AS_ASSERT(serialized_shader, "Cannot serialize shader, invalid serialized_shader");

	AS_WAIT_AND_LOCK(shader);
	strcpy(serialized_shader->filename_vertex, shader->filename_vertex);
	strcpy(serialized_shader->filename_fragment, shader->filename_fragment);
	AS_UNLOCK(shader);
}

void as_deserialize_shader(as_shader* shader, as_serialized_shader* serialized_shader, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(shader, "Cannot deserialize shader, invalid shader");
	AS_ASSERT(serialized_shader, "Cannot deserialize shader, invalid serialized shader");

	strcpy(shader->filename_vertex, serialized_shader->filename_vertex);
	strcpy(shader->filename_fragment, serialized_shader->filename_fragment);
	AS_SET_VALID(shader);

	as_rq_shader_update(render_queue, render, shader);
}

void as_serialize_object(as_object* object, as_serialized_object* serialized_object)
{
	AS_ASSERT(object, "Cannot serialize object, invalid ptr");
	AS_WARNING_RETURN_IF_FALSE(object->shape, "Cannot serialize object, invalid shape");

	AS_WAIT_AND_LOCK(object);

	serialized_object->transform = object->transform;
	serialized_object->instance_count = object->instance_count;
	as_serialize_shader(object->shader, &serialized_object->shader);
	serialized_object->shape = *object->shape;

	AS_UNLOCK(object);
}

void as_deserialize_object(as_object* object, as_serialized_object* serialized_object, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(object, "Cannot deserialize object, invalid obj");
	AS_ASSERT(serialized_object, "Cannot deserialize object, invalid serialized obj");

	object->transform = serialized_object->transform;
	object->instance_count = serialized_object->instance_count;
	object->shader = AS_MALLOC_SINGLE(as_shader);
	object->shape = &serialized_object->shape;
	as_deserialize_shader(object->shader, &serialized_object->shader, render, render_queue);
	AS_SET_VALID(object);

	as_rq_object_update(render_queue, render, object, &serialized_object->shape, object->shader);
}

void as_serialize_light(as_light* light, as_serialized_light* serialized_light)
{
	AS_ASSERT(light, "Cannot serialize light, invalid light");
	AS_ASSERT(serialized_light, "Cannot serialize light, invalid serialized_light");

	serialized_light->color = light->color;
	serialized_light->position = light->position;
	serialized_light->radius = light->radius;
}

void as_deserialize_light(as_light* light, as_serialized_light* serialized_light, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(light, "Cannot deserialize light, invalid ligth");
	AS_ASSERT(serialized_light, "Cannot deserialize light, invalid serialized_light");

	light->color = serialized_light->color;
	light->position = serialized_light->position;
	light->radius = serialized_light->radius;
}

void as_serialize_camera(as_camera* camera, as_serialized_camera* serialized_camera)
{
	AS_ASSERT(camera, "Cannot serialize camera, invalid camera");
	AS_ASSERT(serialized_camera, "Cannot serialize camera, invalid serialized_camera");

	AS_WAIT_AND_LOCK(camera);

	serialized_camera->position = camera->position;
	serialized_camera->target = camera->target;
	serialized_camera->up = camera->up;
	serialized_camera->fov = camera->fov;
	serialized_camera->type = camera->type;
	serialized_camera->movement_speed = camera->movement_speed;
	serialized_camera->cached_direction = camera->cached_direction;

	AS_UNLOCK(camera);
}

void as_deserialize_camera(as_camera* camera, as_serialized_camera* serialized_camera, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(camera, "Cannot deserialize camera, invalid serialized camera");
	AS_ASSERT(serialized_camera, "Cannot deserialize camera, invalid serialized camera");

	camera->position = serialized_camera->position;
	camera->target = serialized_camera->target;
	camera->up = serialized_camera->position;
	camera->fov = serialized_camera->fov;
	camera->type = serialized_camera->type;
	camera->movement_speed = serialized_camera->movement_speed;
	camera->cached_direction = serialized_camera->cached_direction;

	AS_SET_VALID(camera);
}

void as_serialize_scene(as_scene* scene, as_serialized_scene* serialized_scene)
{
	AS_ASSERT(scene, "Cannot serialize scene, invalid scene");
	AS_ASSERT(serialized_scene, "Cannot serialize scene, invalid serialized_scene");

	AS_WAIT_AND_LOCK(scene);

	strcpy(serialized_scene->path, scene->path);

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(scene->objects) ; i++)
	{
		as_object* object = AS_ARRAY_GET(scene->objects, i);
		as_serialized_object* serialized_object = AS_ARRAY_INCREMENT(serialized_scene->objects);

		AS_WAIT_AND_LOCK(object);
		as_serialize_object(object, serialized_object);
		AS_UNLOCK(object);
	}

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(scene->cameras); i++)
	{
		as_camera* camera = AS_ARRAY_GET(scene->cameras, i);
		as_serialized_camera* serialized_camera = AS_ARRAY_INCREMENT(serialized_scene->cameras);

		AS_WAIT_AND_LOCK(camera);
		as_serialize_camera(camera, serialized_camera);
		AS_UNLOCK(camera);
	}

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(scene->lights); i++)
	{
		as_light* light = AS_ARRAY_GET(scene->lights, i);
		as_serialized_light* serialized_light = AS_ARRAY_INCREMENT(serialized_scene->lights);
		as_serialize_light(light, serialized_light);
	}
	
	AS_UNLOCK(scene);
}

void as_deserialize_scene(as_scene* scene, as_serialized_scene* serialized_scene, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(scene, "Cannot deserialize scene, invalid scene");
	AS_ASSERT(serialized_scene, "Cannot deserialize scene, invalid serialized_scene");

	strcpy(scene->path, serialized_scene->path);

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(serialized_scene->objects); i++)
	{
		as_serialized_object* serialized_object = AS_ARRAY_GET(serialized_scene->objects, i);
		as_object* object = AS_ARRAY_INCREMENT(scene->objects);
		as_deserialize_object(object, serialized_object, render, render_queue);
	}

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(serialized_scene->cameras); i++)
	{
		as_serialized_camera* serialized_camera = AS_ARRAY_GET(serialized_scene->cameras, i);
		as_camera* camera = AS_ARRAY_INCREMENT(scene->cameras);
		as_deserialize_camera(camera, serialized_camera, render, render_queue);
	}

	for (sz i = 0; i < AS_ARRAY_GET_SIZE(serialized_scene->lights); i++)
	{
		as_serialized_light* serialized_light = AS_ARRAY_GET(serialized_scene->lights, i);
		as_light* light = AS_ARRAY_INCREMENT(scene->lights);
		as_deserialize_light(light, serialized_light, render, render_queue);
	}

	AS_SET_VALID(scene);
}

void as_serialized_scene_destroy(as_serialized_scene* serializaed_scene)
{
	AS_FREE(serializaed_scene);
}

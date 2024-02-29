// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_serialization.h"
#include "core/as_render_queue.h"
#include "as_memory.h"

as_serialized_shader* as_serialize_shader(as_shader* shader)
{
	AS_ASSERT(shader, "Cannot serialize shader, invalid ptr");

	as_serialized_shader* serialized_shader = AS_MALLOC_SINGLE(as_serialized_shader);
	strcpy(serialized_shader->filename_vertex, shader->filename_vertex);
	strcpy(serialized_shader->filename_fragment, shader->filename_fragment);

	return serialized_shader;
}

as_shader* as_deserialize_shader(as_serialized_shader* serialized_shader, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(serialized_shader, "Cannot deserialize shader, invalid serialized ptr");

	as_shader* shader = AS_MALLOC_SINGLE(as_shader);

	strcpy(shader->filename_vertex, serialized_shader->filename_vertex);
	strcpy(shader->filename_fragment, serialized_shader->filename_fragment);
	AS_SET_VALID(shader);
	as_rq_shader_update(render_queue, render, shader);

	return shader;
}

as_serialized_object* as_serialize_object(as_object* object)
{
	AS_ASSERT(object, "Cannot serialize object, invalid ptr");

	as_serialized_object* serialized_object = AS_MALLOC_SINGLE(as_serialized_object);

	serialized_object->transform = object->transform;
	serialized_object->instance_count = object->instance_count;
	serialized_object->shader = *as_serialize_shader(object->shader);

	return serialized_object;
}

as_object* as_deserialize_object(as_serialized_object* serialized_object, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(serialized_object, "Cannot deserialize object, invalid serialized ptr");

	as_object* object = AS_MALLOC_SINGLE(as_object);

	object->transform = serialized_object->transform;
	object->instance_count = serialized_object->instance_count;
	object->shader = as_deserialize_shader(&serialized_object->shader, render, render_queue);
	AS_SET_VALID(object);
	as_rq_object_update(render_queue, render, object, );

	return object;
}

as_serialized_light* as_serialize_light(as_light* light)
{
	AS_ASSERT(light, "Cannot serialize light, invalid ptr");

	as_serialized_light* serialized_light = AS_MALLOC_SINGLE(as_serialized_light);

	serialized_light->color = light->color;
	serialized_light->position = light->position;
	serialized_light->radius = light->radius;

	return serialized_light;
}

as_light* as_deserialize_light(as_serialized_light* serialized_light, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(serialized_light, "Cannot deserialize light, invalid serialized ptr");

	as_light* light = AS_MALLOC_SINGLE(as_light);

	light->color = serialized_light->color;
	light->position = serialized_light->position;
	light->radius = serialized_light->radius;

	return light;
}

as_serialized_camera* as_serialize_camera(as_camera* camera)
{
	AS_ASSERT(camera, "Cannot serialize camera, invalid ptr");

	as_serialized_camera* serialized_camera = AS_MALLOC_SINGLE(as_serialized_camera);

	serialized_camera->position = camera->position;
	serialized_camera->target = camera->target;
	serialized_camera->up = camera->up;
	serialized_camera->fov = camera->fov;
	serialized_camera->type = camera->type;
	serialized_camera->movement_speed = camera->movement_speed;
	serialized_camera->cached_direction = camera->cached_direction;

	return serialized_camera;
}

as_camera* as_deserialize_camera(as_serialized_camera* serialized_camera, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(serialized_camera, "Cannot deserialize camera, invalid serialized ptr");

	as_camera* camera = AS_MALLOC_SINGLE(as_serialized_camera);

	camera->position = serialized_camera->position;
	camera->target = serialized_camera->target;
	camera->up = serialized_camera->position;
	camera->fov = serialized_camera->fov;
	camera->type = serialized_camera->type;
	camera->movement_speed = serialized_camera->movement_speed;
	camera->cached_direction = serialized_camera->cached_direction;

	AS_SET_VALID(camera);

	return camera;
}

as_serialized_scene* as_serialize_scene(as_scene* scene)
{
	AS_ASSERT(scene, "Cannot serialize scene, invalid ptr");

	as_serialized_scene* serialized_scene = AS_MALLOC_SINGLE(as_serialized_scene);

	serialized_scene->path = scene->path;
	AS_ARRAY_FOR_EACH(scene->objects, as_object, object,
	{
		as_serialized_object* serialized_object = as_serialize_object(object);
		AS_ARRAY_PUSH_BACK(serialized_scene->objects, serialized_object);
	});
	AS_ARRAY_FOR_EACH(scene->lights, as_light, light,
	{
		as_serialized_light* serialized_light = as_serialize_light(light);
		AS_ARRAY_PUSH_BACK(serialized_scene->lights, serialized_light);
	});
	AS_ARRAY_FOR_EACH(scene->cameras, as_camera, camera,
	{
		as_serialized_camera* serialized_camera = as_serialize_camera(camera);
		AS_ARRAY_PUSH_BACK(serialized_scene->cameras, serialized_camera);
	});
	return serialized_scene;
}

as_scene* as_deserialize_scene(as_serialized_scene* serialized_scene, as_render* render, as_render_queue* render_queue)
{
	AS_ASSERT(serialized_scene, "Cannot deserialize scene, invalid serialized ptr");

	as_scene* scene = AS_MALLOC_SINGLE(as_scene);
	scene->path = serialized_scene->path;
	AS_ARRAY_FOR_EACH(serialized_scene->objects, as_serialized_object, serialized_object,
		{
			as_object* object = as_deserialize_object(serialized_object, render, render_queue);
			AS_ARRAY_PUSH_BACK(scene->objects, object);
		});
	AS_ARRAY_FOR_EACH(serialized_scene->lights, as_serialized_light, serialized_light,
		{
			as_light* light = as_deserialize_light(serialized_light, render, render_queue);
			AS_ARRAY_PUSH_BACK(scene->lights, light);
		});
	AS_ARRAY_FOR_EACH(serialized_scene->cameras, as_serialized_camera, serialized_camera,
		{
			as_camera* camera = as_deserialize_camera(serialized_camera, render, render_queue);
			AS_ARRAY_PUSH_BACK(scene->cameras, serialized_camera);
		});
	return serialized_scene;
}

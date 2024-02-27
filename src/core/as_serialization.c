// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_serialization.h"
#include "core/as_render_queue.h"
#include "as_memory.h"

as_serialized_shader* as_serialize_shader(as_shader* shader)
{
	AS_ASSERT(shader, "Cannot serialize shader, invalid ptr");
	return NULL;
}

as_shader* as_deserialize_shader(as_render* render, as_render_queue* render_queue, as_serialized_shader* serialized_shader)
{
	AS_ASSERT(serialized_shader, "Cannot deserialize shader, invalid serialized ptr");

	as_shader* shader = AS_MALLOC_SINGLE(as_shader);
	
	strcpy(shader->filename_vertex, serialized_shader->filename_vertex);
	strcpy(shader->filename_fragment, serialized_shader->filename_fragment);
	as_rq_shader_update(render_queue, render, shader);
	return shader;
}

as_serialized_object* as_serialize_object(as_object* object)
{
	AS_ASSERT(object, "Cannot serialize object, invalid ptr");
	return NULL;
}

as_object* as_deserialize_object(as_render* render, as_render_queue* render_queue, as_serialized_object* serialized_object)
{
	AS_ASSERT(serialized_object, "Cannot deserialize object, invalid serialized ptr");
	return NULL;
}

as_serialized_scene* as_serialize_scene(as_scene* scene)
{
	AS_ASSERT(scene, "Cannot serialize scene, invalid ptr");
	return NULL;
}

as_scene* as_deserialize_scene(as_render* render, as_render_queue* render_queue, as_serialized_scene* serialized_scene)
{
	AS_ASSERT(serialized_scene, "Cannot deserialize scene, invalid serialized ptr");
	return NULL;
}

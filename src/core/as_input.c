// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_input.h"
#include "defines/as_keys.h"
#include "as_memory.h"

void as_input_add(as_input_buffer* buffer, const i32 key, const i32 action)
{
    AS_ASSERT(buffer, "Trying to add input, but buffer is NULL");

    as_input_key new_key = {0};
    new_key.key = key;
    new_key.action = action;
    
    // AS_WAIT_AND_LOCK(buffer);
    AS_ARRAY_INSERT_AT(buffer->keys, 0, new_key);
    if(AS_ARRAY_GET_SIZE(buffer->keys) > AS_MAX_INPUT_BUFFER_SIZE)
    {
        AS_ARRAY_REMOVE_AT(buffer->keys, AS_ARRAY_GET_LAST_INDEX(buffer->keys));
    }
    // AS_SET_UNLOCKED(buffer);
    if(buffer->on_add)
    {    
        buffer->on_add(AS_ARRAY_GET(buffer->keys, 0));
    }
}

bool as_input_is_pressed(as_input_buffer* buffer, const i32 key)
{
    AS_ASSERT(buffer, "Trying to read buffer, but buffer is NULL");
    // AS_WAIT_AND_LOCK(buffer);
    for(sz i = 0 ; i < buffer->keys.size ; i++)
    {
        as_input_key* current_key = AS_ARRAY_GET(buffer->keys, i);
        if (current_key && current_key->key == key)
        {
            return (current_key->action == AS_PRESS || current_key->action == AS_REPEAT);
        }
    }
    // AS_SET_UNLOCKED(buffer);
    return false;
}

as_input_buffer* as_input_create()
{
    as_input_buffer* new_buffer = AS_MALLOC_SINGLE(as_input_buffer);
    AS_SET_VALID(new_buffer);
    return new_buffer;
}

void as_input_destory(as_input_buffer* buffer)
{
    if (AS_IS_INVALID(buffer)) { return; }
    AS_SET_INVALID(buffer);
    AS_FREE(buffer);
}

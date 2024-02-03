// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_tick.h"
#include "as_memory.h"

as_tick_handle* as_tick_handle_create(as_tick_system* tick_system)
{
    AS_ASSERT(tick_system, "Cannot add tick handle to system, invalid tick system");
    AS_WAIT_AND_LOCK(tick_system);
    as_tick_handle* handle = AS_ARRAY_INCREMENT(tick_system->handles);
    AS_UNLOCK(tick_system);
    return handle;
}

void as_tick_handle_execute(const as_tick_handle* tick_handle, const u64 delta_time)
{
    AS_ASSERT(tick_handle, "Cannot execute tick handle, invalid tick handle");
    AS_ASSERT(tick_handle->func_ptr, "Cannot execute tick handle, invalid function pointer");
    tick_handle->func_ptr(tick_handle->arg, delta_time);
}

as_tick_system* as_tick_system_create()
{
    as_tick_system* tick_system = AS_MALLOC_SINGLE(as_tick_system);
    AS_SET_VALID(tick_system);
    return tick_system;
}

void as_tick_system_destroy(as_tick_system* tick_system)
{
    if (AS_IS_INVALID(tick_system)) { return; }
    AS_FREE(tick_system);
}

void as_tick_system_execute(const as_tick_system* tick_system, const u64 delta_time)
{
    AS_ASSERT(tick_system, "Cannot execute tick system, invalid tick system");
    for (sz i = 0; i < tick_system->handles.size ; i++)
    {
        const as_tick_handle* tick_handle = AS_ARRAY_GET(tick_system->handles, i );
        as_tick_handle_execute(tick_handle, delta_time);
    }
}

#ifdef AS_USE_RAYLIB
// internal dependencies (engine)
#include "engine_utility.h"
#include "engine_core.h"

void as::init_window(const u16& width, const u16& height, const char* title)
{
	InitWindow(width, height, title);
}

#endif //AS_USE_RAYLIB
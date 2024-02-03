// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_editor.h"

i32 main()
{
	as_engine_init();

#if AS_EDITOR
	as_editor_set_default_scene();
#endif

	while (as_engine_should_loop())
	{
		as_engine_draw();
	}

	as_engine_clear();
}

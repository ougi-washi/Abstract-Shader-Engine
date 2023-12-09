// MMO_ECS_SERVER - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once
#include "as_types.h"

inline void as_i32_to_str(const i32 integer, char* out_str)
{
	sprintf(out_str, "%d", integer);
};
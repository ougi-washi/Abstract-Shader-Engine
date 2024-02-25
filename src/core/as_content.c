// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "core/as_content.h"
#include "as_types.h"
#include "as_memory.h"

as_content* as_content_create()
{
	return AS_MALLOC_SINGLE(as_content);
}

void as_content_destroy(as_content* content)
{
	for (sz i = 0 ; i < AS_ARRAY_GET_SIZE(content->assets) ; i++)
	{
		as_asset* asset = AS_ARRAY_GET(content->assets, i);
		AS_FREE(asset->ptr); // Maybe have to check each type and clear it accordingly
	}
	AS_FREE(content);
}

i32 as_content_add_asset(as_content* content, void* ptr, const as_asset_type type)
{
	AS_ASSERT(content, "Cannot add asset to content, NULL content");
	AS_ASSERT(ptr, "Cannot add asset to content, NULL ptr");

	as_asset* asset = AS_ARRAY_INCREMENT(content->assets);
	AS_ASSERT(asset, "Cannot add asset to content, asset array could be full");

	asset->ptr = ptr;
	asset->type = type;
	AS_SET_VALID(asset);

	return AS_ARRAY_GET_LAST_INDEX(content->assets);
}

void as_content_remove_asset(as_content* content, const sz index)
{
	AS_ASSERT(content, "Cannot remove asset from content, NULL content");
	as_asset* asset = AS_ARRAY_GET(content->assets, index);
	asset->ptr = NULL;
	AS_SET_INVALID(asset);
	AS_ARRAY_REMOVE_AT(content->assets, index);
}

as_asset* as_content_get_asset(as_content* content, const sz index)
{
	AS_ASSERT(content, "Cannot get asset, NULL content");
	return AS_ARRAY_GET(content->assets, index);
}

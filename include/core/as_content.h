// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_array.h"

typedef enum as_asset_type
{
	AS_ASSET_TYPE_UNKOWN,
	AS_ASSET_TYPE_TEXTURE,
	AS_ASSET_TYPE_SHADER,
	AS_ASSET_TYPE_OBJECT,
	AS_ASSET_TYPE_LIGHT,
	AS_ASSET_TYPE_SHAPE,
	AS_ASSET_TYPE_FUNCTION
}as_asset_type;

typedef struct as_asset
{
	AS_DECLARE_TYPE;
	as_asset_type type;
	void* ptr;
	void (*destory_func_ptr)(void*);
	bool free_on_destruction;
} as_asset;
AS_ARRAY_DECLARE(as_assets_1024, 1024, as_asset);

typedef struct as_content
{
	AS_DECLARE_TYPE;
	as_assets_1024 assets;
} as_content;

extern as_content* as_content_create();
extern void as_content_destroy(as_content* content);
extern i32 as_content_add_asset(as_content* content, void* ptr, const as_asset_type type, const bool free_on_destruction, void destroy_func_ptr(void*));
extern void as_content_remove_asset(as_content* content, const sz index, const bool destroy);
extern as_asset* as_content_get_asset(as_content* content, const sz index);

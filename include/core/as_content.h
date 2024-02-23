// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_array.h"

typedef enum as_asset_type
{
	AS_ASSET_TYPE_TEXTURE,
	AS_ASSET_TYPE_SHADER,
	AS_ASSET_TYPE_OBJECT,
	AS_ASSET_TYPE_LIGHT,
	AS_ASSET_TYPE_SHAPE,
	AS_ASSET_TYPE_FUNCTION
}as_asset_type;

typedef struct as_asset
{
	as_asset_type type;
	void* ptr;
	AS_FLAG;
} as_asset;
AS_ARRAY_DECLARE(as_assets_1024, 1024, as_asset);

typedef struct as_content
{
	as_assets_1024 assets;
} as_content;

as_content* as_content_create();
void as_content_destroy(as_content* content);
void as_content_add_asset(as_content* content, void* ptr, const as_asset_type type);
void as_content_remove_asset(as_content* content, const sz index);
as_asset* as_content_get_asset(as_content* content, const sz index);

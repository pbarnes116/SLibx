#ifndef CHECKHEADER_SLIB_MAP_UTIL
#define CHECKHEADER_SLIB_MAP_UTIL

#include "definition.h"

#include "../../slib/core/string.h"
#include "../../slib/math/geometry.h"
#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

class MapTileLocation
{
public:
	sl_uint32 level;
	double y; // latitude
	double x; // longitude
};

class MapTilePath
{
public:
	static String makeGenericStylePath(const MapTileLocation& location, String* packagePath = sl_null, String* filePath = sl_null);
	static String makeVWStylePath(const MapTileLocation& location, String* packagePath = sl_null, String* filePath = sl_null);
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_TILE
#define CHECKHEADER_SLIB_MAP_TILE

#include "definition.h"

SLIB_MAP_NAMESPACE_START

class MapTileAddress
{
public:
	sl_uint32 level;
	sl_uint32 y; // latitude
	sl_uint32 x; // longitude
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_VW_BUILDING
#define CHECKHEADER_SLIB_MAP_VW_BUILDING

#include "definition.h"

#include "data.h"

SLIB_MAP_NAMESPACE_START

class VW_Building
{
public:

	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

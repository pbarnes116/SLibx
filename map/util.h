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


class PackageFilePath
{
public:
	static String makePackageFilePath(const MapTileLocation& location, const String& subFolderName, String* packagePath = sl_null, String* filePath = sl_null);
	static String makePackageFilePath(const LatLon& location, sl_int32 zoomLevel, const String& subFolderName, String* packagePath = sl_null, String* filePath = sl_null);
	static void getPackageFileOffsetXY(const MapTileLocation& location, sl_int32& blockOffsetX, sl_int32& blockOffsetY);
	static void getPackageFileOffsetXY(const LatLon& location, sl_int32 zoomLevel, sl_int32& blockOffsetX, sl_int32& blockOffsetY);
};
SLIB_MAP_NAMESPACE_END

#endif

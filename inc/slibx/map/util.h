#ifndef CHECKHEADER_SLIB_MAP_UTIL
#define CHECKHEADER_SLIB_MAP_UTIL

#include "definition.h"

#include <slib/core/string.h>
#include <slib/graphics/image.h>

SLIB_MAP_NAMESPACE_BEGIN

template <class T>
class SLIB_EXPORT MapTileLocationT
{
public:
	sl_uint32 level;
	T y; // latitude
	T x; // longitude

	SLIB_INLINE MapTileLocationT() {}

	SLIB_INLINE MapTileLocationT(sl_uint32 _level, T _y, T _x)
	{
		level = _level;
		y = _y;
		x = _x;
	}

	template <class O>
	SLIB_INLINE MapTileLocationT(const MapTileLocationT<O>& other)
	{
		level = other.level;
		y = (T)(other.y);
		x = (T)(other.x);
	}

	template <class O>
	SLIB_INLINE MapTileLocationT<T>& operator=(const MapTileLocationT<O>& other)
	{
		level = other.level;
		y = (T)(other.y);
		x = (T)(other.x);
		return *this;
	}

	SLIB_INLINE sl_bool operator==(const MapTileLocationT<T>& other)
	{
		return level == other.level && y == other.y && x == other.x;
	}
};

typedef MapTileLocationT<double> MapTileLocation;
typedef MapTileLocationT<sl_int32> MapTileLocationi;

template <>
SLIB_INLINE sl_uint32 Hash<MapTileLocationi>::hash(const MapTileLocationi& location)
{
	sl_uint64 c = location.level;
	c <<= 30;
	c ^= location.y;
	c <<= 30;
	c ^= location.x;
	return Hash<sl_uint64>::hash(c);
}

class SLIB_EXPORT MapTilePath
{
public:
	static String makeGenericStylePath(const MapTileLocationi& location, String* packagePath = sl_null, String* filePath = sl_null);
	static String makeVWStylePath(const MapTileLocationi& location, String* packagePath = sl_null, String* filePath = sl_null);
};

class IMapTileDataLoader;
class SLIB_EXPORT MapPictureUtil
{
public:
	static Ref<Image> capturePictureFromTiles(IMapTileDataLoader* loader, sl_uint32 width, sl_uint32 height, sl_uint32 level, double X0, double Y0, double X1, double Y1, sl_bool flagOpaque, sl_bool& outFlagContainsAlpha);
};

SLIB_MAP_NAMESPACE_END

#endif

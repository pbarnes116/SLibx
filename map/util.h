#ifndef CHECKHEADER_SLIB_MAP_UTIL
#define CHECKHEADER_SLIB_MAP_UTIL

#include "definition.h"

#include "../../slib/core/string.h"
#include "../../slib/math/geometry.h"
#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

template <class T>
class MapTileLocationT
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

sl_uint32 hashCode(const MapTileLocationi& location);

class MapTilePath
{
public:
	static String makeGenericStylePath(const MapTileLocationi& location, String* packagePath = sl_null, String* filePath = sl_null);
	static String makeVWStylePath(const MapTileLocationi& location, String* packagePath = sl_null, String* filePath = sl_null);
};

SLIB_MAP_NAMESPACE_END

#endif

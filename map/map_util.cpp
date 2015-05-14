#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slib/core/hashtable.h"

SLIB_MAP_NAMESPACE_BEGIN

sl_uint32 hashCode(const MapTileLocationi& location)
{
	sl_uint64 c = location.level;
	c <<= 30;
	c ^= location.y;
	c <<= 30;
	c ^= location.x;
	return hashCode(c);
}

String MapTilePath::makeGenericStylePath(const MapTileLocationi& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += "/";
	packagePath += location.y;
	String filePath;
	filePath += location.x;
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + "/" + filePath;
}

String MapTilePath::makeVWStylePath(const MapTileLocationi& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += "/";
	String y;
	if (location.level <= 11) {
		y = String::fromUint32(location.y, 10, 4);
	} else {
		y = String::fromUint32(location.y, 10, 8);
	}
	packagePath += y;
	String filePath = y;
	filePath += "_";
	if (location.level <= 10) {
		filePath += String::fromUint32(location.x, 10, 4);
	} else {
		filePath += String::fromUint32(location.x, 10, 8);
	}
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + "/" + filePath;
}

SLIB_MAP_NAMESPACE_END

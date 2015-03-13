#include "util.h"

SLIB_MAP_NAMESPACE_START

String MapTilePath::makeGenericStylePath(const MapTileLocation& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += _SLT("/");
	packagePath += (sl_uint32)(location.y);
	String filePath;
	filePath += (sl_uint32)(location.x);
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + _SLT("/") + filePath;
}

String MapTilePath::makeVWStylePath(const MapTileLocation& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += _SLT("/");
	String y;
	if (location.level <= 11) {
		y = String::fromUint32((sl_uint32)(location.y), 10, 4);
	} else {
		y = String::fromUint32((sl_uint32)(location.y), 10, 8);
	}
	packagePath += y;
	String filePath = y;
	filePath += _SLT("_");
	if (location.level <= 10) {
		filePath += String::fromUint32((sl_uint32)(location.x), 10, 4);
	} else {
		filePath += String::fromUint32((sl_uint32)(location.x), 10, 8);
	}
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + _SLT("/") + filePath;
}

SLIB_MAP_NAMESPACE_END

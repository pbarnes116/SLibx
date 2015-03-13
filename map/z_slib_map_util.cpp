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

String PackageFilePath::makePackageFilePath(const MapTileLocation& location, const String& subFolderName, String* _packagePath /* = sl_null */, String* _filePath /* = sl_null */)
{
	String zoomFolderPath = "";
	sl_int32 tilesNum = 1 << location.level;
	sl_int32 blockX = (sl_int32)((location.x + 180.0) / 360.0 * tilesNum);
	sl_int32 blockY = (sl_int32)((location.y + 90.0) / 180.0 * tilesNum);
	sl_int32 fragmentNum = 1;
	if (location.level <= 18 && location.level > 10) {
		fragmentNum = 1 << (location.level - 10);
		zoomFolderPath = _SLT("P11-18");
	}
	else if (location.level <= 10 && location.level > 2) {
		fragmentNum = 1 << (location.level - 2);
		zoomFolderPath = _SLT("P03-10");
	}
	else {
		fragmentNum = 1 << (location.level);
		zoomFolderPath = _SLT("P00-02");
	}
	sl_int32 packageX = blockX / fragmentNum;
	sl_int32 packageY = blockY / fragmentNum;
	sl_int32 packageOffsetX = blockX % fragmentNum;
	sl_int32 packageOffsetY = blockY % fragmentNum;

	String filePath = String::fromInt32(packageX) + _SLT(".pkg");
	String pkgPath = zoomFolderPath + _SLT("/") + String::fromInt32(packageY);
	if (_packagePath) {
		*_packagePath = pkgPath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return pkgPath + _SLT("/") + filePath;
}

String PackageFilePath::makePackageFilePath(const LatLon& location, sl_int32 zoomLevel, const String& subFolderName, String* packagePath /* = sl_null */, String* filePath /* = sl_null */)
{
	MapTileLocation newLoc;
	newLoc.x = location.longitude;
	newLoc.y = location.latitude;
	newLoc.level = zoomLevel;
	return makePackageFilePath(newLoc, subFolderName, packagePath, filePath);
}

void PackageFilePath::getPackageFileOffsetXY(const MapTileLocation& location, sl_int32& blockOffsetX, sl_int32& blockOffsetY)
{
	sl_int32 tilesNum = 1 << location.level;
	sl_int32 blockX = (sl_int32)((location.x + 180.0f) / 360.0f * tilesNum);
	sl_int32 blockY = (sl_int32)((location.y + 90.0f) / 180.0f * tilesNum);
	sl_int32 fragmentNum = 1;
	if (location.level <= 18 && location.level > 10) {
		fragmentNum = 1 << (location.level - 10);
	}
	else if (location.level <= 10 && location.level > 2) {
		fragmentNum = 1 << (location.level - 2);
	}
	blockOffsetX = blockX % fragmentNum;
	blockOffsetY = blockY % fragmentNum;
}

void PackageFilePath::getPackageFileOffsetXY(const LatLon& location, sl_int32 zoomLevel, sl_int32& blockOffsetX, sl_int32& blockOffsetY)
{
	MapTileLocation newLoc;
	newLoc.x = location.longitude;
	newLoc.y = location.latitude;
	newLoc.level = zoomLevel;
	return getPackageFileOffsetXY(newLoc, blockOffsetX, blockOffsetY);
}

SLIB_MAP_NAMESPACE_END

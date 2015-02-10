#include "location.h"
SLIB_MAP_NAMESPACE_START

void MapLocation::convertToGeoLocation()
{
	sl_int32 startX = (pkgLocation.x << 8) + pkgLocation.offsetX;
	sl_int32 startY = (pkgLocation.y << 8) + pkgLocation.offsetY;
	if (pkgLocation.folderType == MapPackageTypeLevel11_18) {
		_zoom = 18;
	}
	else if (pkgLocation.folderType == MapPackageTypeLevel3_10) {
		_zoom = 10;
	}
	_lat = (double)startY / (double)(1 << _zoom) * 180.0 - 90.0;
	_lon = (double)startX / (double)(1 << _zoom) * 360.0 - 180.0;
}

void MapLocation::convertToPackageLocation()
{
	sl_int32 tilesNum = 1 << _zoom;
	sl_int32 blockX = (sl_int32)((_lon + 180.0) / 360.0 * tilesNum);
	sl_int32 blockY = (sl_int32)((_lat + 90.0) / 180.0 * tilesNum);
	sl_int32 fragmentNum = 1;
	if (_zoom <= 18 && _zoom > 10) {
		fragmentNum = 1 << (_zoom - 10);
		pkgLocation.folderType = MapPackageTypeLevel11_18;
	}
	else if (_zoom <= 10 && _zoom > 2) {
		fragmentNum = 1 << (_zoom - 2);
		pkgLocation.folderType = MapPackageTypeLevel3_10;
	}
	pkgLocation.x = blockX / fragmentNum;
	pkgLocation.y = blockY / fragmentNum;
	pkgLocation.offsetX = blockX % fragmentNum;
	pkgLocation.offsetY = blockY % fragmentNum;
}

sl_bool MapLocation::checkPositionContained(const MapLocation& areaStart, const MapLocation& areaEnd, const MapLocation position)
{
	if (position._lat <= Math::max(areaStart._lat, areaEnd._lat) && position._lat >= Math::min(areaStart._lat, areaEnd._lat)
		&& position._lon <= Math::max(areaStart._lon, areaEnd._lon) && position._lon >= Math::min(areaStart._lon, areaEnd._lon)) {
		return sl_true;
	}
	return false;
}

MapLocation MapLocation::checkLineIntersection(const MapLocation& line1Start, const MapLocation& line1End, const MapLocation& line2Start, const MapLocation& line2End)
{
	MapLocation crossPoint;
	double p0_x, p0_y, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;
	p0_x = line1Start._lon;
	p0_y = line1Start._lat;
	p1_x = line1End._lon;
	p1_y = line1End._lat;
	p2_x = line2Start._lon;
	p2_y = line2Start._lat;
	p3_x = line2End._lon;
	p3_y = line2End._lat;

	double s1_x, s1_y, s2_x, s2_y;
	s1_x = p1_x - p0_x;
	s1_y = p1_y - p0_y;
	s2_x = p3_x - p2_x;
	s2_y = p3_y - p2_y;

	double s, t;
	s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
	t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

	if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
		crossPoint._lon = p0_x + (t * s1_x);
		crossPoint._lat = p0_y + (t * s1_y);
	}

	return crossPoint;
}

MapLocation MapLocation::fromGoogleAddress(const String& googleAddr)
{
	sl_int32 n = googleAddr.getLength();
	if (n <= 0) {
		return MapLocation(0, 0, 0);
	}
	sl_int32 x = 0;
	sl_int32 y = 0;
	sl_int32 px = 1;
	sl_int32 py = 1;
	
	for (sl_int32 i = 0; i < n; i++) {
		sl_int32 q = googleAddr.getAt(i) - _SLT('0');
		if (q >= 4) {
			q -= 4;
		}
		sl_int32 tx = q % 2;
		x = x * 2 + tx;
		px *= 2;
		if (i != 1) {
			int ty = (q / 2) % 2;
			y = y * 2 + ty;
			py *= 2;
		}
	}
	double lon = (double)x * 360.0 / (double)px - 180.0;
	double lat = (double)y * 180.0 / (double)py - 90.0;
	return MapLocation(lat, lon, n);
}

SLIB_MAP_NAMESPACE_END
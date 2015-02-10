#ifndef CHECKHEADER_SLIB_MAP_LOCATOIN_HEADER
#define CHECKHEADER_SLIB_MAP_LOCATOIN_HEADER

#include "definition.h"
#include "../../slib/slib.h"
SLIB_MAP_NAMESPACE_START
class MapLocation
{
public:
	enum PackageFolderType{
		MapPackageTypeLevel11_18 = 1
		, MapPackageTypeLevel3_10 = 2
	};
	static MapLocation getCenterLocation(List<MapLocation> loc);
	static MapLocation checkLineIntersection(const MapLocation& line1Start, const MapLocation& line1End, const MapLocation& line2Start, const MapLocation& line2End);
	static sl_bool checkPositionContained(const MapLocation& areaStart, const MapLocation& areaEnd, const MapLocation position);
	static MapLocation fromGoogleAddress(const String& googleAddr);
public:
	double _lat;
	double _lon;
	sl_int32 _zoom;
	struct tagPackageLocation {
		PackageFolderType folderType;
		sl_int32 x;
		sl_int32 y;
		sl_int32 offsetX;
		sl_int32 offsetY;
	} pkgLocation;

private:
	void convertToPackageLocation();
	void convertToGeoLocation();

public:
	SLIB_INLINE MapLocation()
	{
		this->_lat = this->_lon = 0.f;
		this->_zoom = 0;
		Base::zeroMemory(&pkgLocation, sizeof(tagPackageLocation));
	}
	SLIB_INLINE MapLocation(double lat, double lon, sl_int32 zoom)
	{
		this->_lat = lat; this->_lon = lon; this->_zoom = zoom;
		convertToPackageLocation();
	}

	SLIB_INLINE MapLocation(tagPackageLocation pkgLocation)
	{
		this->pkgLocation = pkgLocation;
		convertToGeoLocation();
	}

	~MapLocation(){}
	SLIB_INLINE MapLocation& operator=(const MapLocation& other)
	{
		this->_lat = other._lat;
		this->_lon = other._lon;
		this->_zoom = other._zoom;
		Base::copyMemory(&pkgLocation, &(other.pkgLocation), sizeof(tagPackageLocation));
		return *this;
	}
	SLIB_INLINE sl_bool operator==(const MapLocation& other) const
	{
		if (this->_lat == other._lat && this->_lon == other._lon) {
			return sl_true;
		}
		return sl_false;
	}
	SLIB_INLINE sl_bool operator!=(const MapLocation& other) const
	{
		if (this->_lat != other._lat || this->_lon != other._lon) {
			return sl_true;
		}
		return sl_false;
	}
	SLIB_INLINE sl_bool isEmpty()
	{
		if (this->_lat == 0 && this->_lon == 0 && this->_zoom == 0) {
			return sl_true;
		} 
		return sl_false;
	}
	SLIB_INLINE sl_bool isNotEmpty()
	{
		return !isEmpty();
	}
};
SLIB_MAP_NAMESPACE_END
#endif

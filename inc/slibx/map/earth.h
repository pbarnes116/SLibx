#ifndef CHECKHEADER_SLIB_MAP_EARTH
#define CHECKHEADER_SLIB_MAP_EARTH

#include "definition.h"

#include <slib/math/geograph.h>

SLIB_MAP_NAMESPACE_BEGIN

class SLIB_EXPORT MapEarth
{
public:
	static const SphericalGlobe& getGlobe();

	SLIB_INLINE static double getRadius()
	{
		return Earth::getEquatorialRadius();
	}

	SLIB_INLINE static Vector3lf getCartesianPosition(double latitude, double longitude, double altitude)
	{
		return getGlobe().getCartesianPosition(latitude, longitude, altitude);
	}
	SLIB_INLINE static Vector3lf getCartesianPosition(const LatLon& latlon)
	{
		return getCartesianPosition(latlon.latitude, latlon.longitude, 0);
	}
	SLIB_INLINE static Vector3lf getCartesianPosition(const GeoLocation& location)
	{
		return getCartesianPosition(location.latitude, location.longitude, location.altitude);
	}

	SLIB_INLINE static GeoLocation getGeoLocation(double x, double y, double z)
	{
		return getGlobe().getGeoLocation(x, y, z);
	}
	SLIB_INLINE static GeoLocation getGeoLocation(const Vector3& pos)
	{
		return getGlobe().getGeoLocation(pos.x, pos.y, pos.z);
	}
};

SLIB_MAP_NAMESPACE_END

#endif

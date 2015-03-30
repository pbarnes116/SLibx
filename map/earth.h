#ifndef CHECKHEADER_SLIB_MAP_EARTH
#define CHECKHEADER_SLIB_MAP_EARTH

#include "definition.h"

#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

class MapEarth
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
};

SLIB_MAP_NAMESPACE_END

#endif

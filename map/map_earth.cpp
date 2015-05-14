#include "../../../inc/slibx/map/earth.h"

SLIB_MAP_NAMESPACE_BEGIN

const SphericalGlobe& MapEarth::getGlobe()
{
	static SphericalGlobe globe(getRadius());
	return globe;
}

SLIB_MAP_NAMESPACE_END

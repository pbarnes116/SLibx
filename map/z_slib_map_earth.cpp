#include "earth.h"

SLIB_MAP_NAMESPACE_START

const SphericalGlobe& MapEarth::getGlobe()
{
	static SphericalGlobe globe(getRadius());
	return globe;
}

SLIB_MAP_NAMESPACE_END

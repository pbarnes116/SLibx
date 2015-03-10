#ifndef CHECKHEADER_SLIB_MAP_TILE_DEM
#define CHECKHEADER_SLIB_MAP_TILE_DEM

#include "definition.h"

#include "../../slib/math/geograph.h"
#include "../../slib/core/array.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START

struct DEM_Vertex
{
	Vector3 position;
	Vector2 texCoord;
	sl_real altitude;
};

// Digital elevation model
class DEM : public Referable
{
public:
	sl_geo_val* dem;
	sl_uint32 N;
	Array<sl_geo_val> array;

public:
	SLIB_INLINE DEM()
	{
		N = 0;
		dem = sl_null;
	}
	
	sl_bool initialize(sl_uint32 N);
	sl_bool initializeFromFloatData(sl_uint32 N, const void* data, sl_size size);
	void makeMesh(Primitive& out, sl_uint32 M, const GeoRectangle& region, const Rectangle& rectDEM, const Rectangle& rectTexture);
};

SLIB_MAP_NAMESPACE_END

#endif

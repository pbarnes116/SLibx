#ifndef CHECKHEADER_SLIB_MAP_DEM
#define CHECKHEADER_SLIB_MAP_DEM

#include "definition.h"

#include <slib/math/geograph.h>
#include <slib/core/array.h>
#include <slib/render/engine.h>

SLIB_MAP_NAMESPACE_BEGIN

struct SLIB_EXPORT DEM_Vertex
{
	Vector3 position;
	sl_real altitude;
	Vector2 texCoord;
	Vector2 texCoordLayers[SLIB_SMAP_MAX_LAYERS_COUNT];
};

// Digital elevation model
class SLIB_EXPORT DEM : public Referable
{
public:
	float* dem;
	sl_uint32 N;
	SafeArray<float> array;

public:
	SLIB_INLINE DEM()
	{
		N = 0;
		dem = sl_null;
	}
	
	sl_bool initialize(sl_uint32 N);
	sl_bool initializeFromFloatData(sl_uint32 N, const void* data, sl_size size);

	void scaleDEM(float* output, sl_uint32 M, const Rectangle& rectDEM) const;

	void makeMeshFromGlobe(const Globe& globe, const Vector3lf& positionCenter, Primitive& out, sl_uint32 M
		, const GeoRectangle& region, const Rectangle& rectDEM
		, const Rectangle& rectTexture, const Rectangle rectTextureLayers[SLIB_SMAP_MAX_LAYERS_COUNT]) const;

	void makeMeshFromSphericalGlobe(const SphericalGlobe& globe, const Vector3lf& positionCenter, Primitive& out, sl_uint32 M
		, const GeoRectangle& region, const Rectangle& rectDEM
		, const Rectangle& rectTexture0, const Rectangle rectTextureLayers[SLIB_SMAP_MAX_LAYERS_COUNT]) const;

	float getAltitudeAt(float x, float y);

};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_GLOBE
#define CHECKHEADER_SLIB_MAP_GLOBE

#include "definition.h"

#include "environment.h"
#include "util.h"
#include "data.h"

#include "../../slib/core/object.h"
#include "../../slib/math/geograph.h"
#include "../../slib/core/map.h"

SLIB_MAP_NAMESPACE_START
class GlobeRenderer : public Object
{
public:
	GlobeRenderer();
	~GlobeRenderer();

protected:
	sl_uint32 m_nMaxLevel;
	sl_uint32 m_nY;
	sl_uint32 m_nX;

	SphericalGlobe m_globe;

public:
	void setLevelParamter(sl_uint32 nY, sl_uint32 nX, sl_uint32 nMaxLevel);
	SLIB_INLINE sl_uint32 getTileYCountOnLevel0()
	{
		return m_nY;
	}
	SLIB_INLINE sl_uint32 getTileXCountOnLevel0()
	{
		return m_nX;
	}
	SLIB_INLINE sl_uint32 getMaxTileLevels()
	{
		return m_nMaxLevel;
	}

	SLIB_INLINE double getRadius()
	{
		return m_globe.getRadius();
	}
	SLIB_INLINE void setRadius(double radius)
	{
		m_globe.setRadius(radius);
	}

public:
	void render(RenderEngine* engine, MapEnvironment* environment);
	void release();

protected:
	Ref<Thread> m_threadResource;
	Ref<Thread> m_threadControl;

	void _runThreadResource();
	void _runThreadControl();

protected:
	Ref<RenderEngine> m_engineResource;
	Ref<MapEnvironment> m_environment;
	
	void _prepareRendering(RenderEngine* engine, MapEnvironment* environment);

protected:
	class TileData : public Referable
	{
	public:
	};
	Map< MapTileLocationi, Ref<TileData> > m_mapResTiles;
	
};
SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_GLOBE
#define CHECKHEADER_SLIB_MAP_GLOBE

#include "definition.h"

#include "environment.h"
#include "util.h"
#include "data.h"
#include "dem.h"

#include "../../slib/core/object.h"
#include "../../slib/math/geograph.h"
#include "../../slib/core/map.h"
#include "../../slib/core/time.h"

#define SLIB_MAP_PICTURE_TILE_TYPE _SLT("picture")
#define SLIB_MAP_PICTURE_TILE_EXT _SLT(".jpg")
#define SLIB_MAP_DEM_TILE_TYPE _SLT("dem")
#define SLIB_MAP_DEM_TILE_EXT _SLT(".bil")
#define SLIB_MAP_GIS_LINE_TILE_TYPE _SLT("gis/line")
#define SLIB_MAP_GIS_LINE_TILE_EXT _SLT(".lin")
#define SLIB_MAP_GIS_POI_TILE_TYPE _SLT("gis/poi")
#define SLIB_MAP_GIS_POI_TILE_EXT _SLT(".poi")
#define SLIB_MAP_BUILDING_TILE_TYPE _SLT("real3d/facility_build")
#define SLIB_MAP_BUILDING_TILE_EXT _SLT(".dat")
#define SLIB_MAP_BUILDING_WORLD_TILE_TYPE _SLT("real3d/facility_build_world")
#define SLIB_MAP_BUILDING_WORLD_TILE_EXT _SLT(".dat")
#define SLIB_MAP_BRIDGE_TILE_TYPE _SLT("real3d/facility_bridge")
#define SLIB_MAP_BRIDGE_TILE_EXT _SLT(".dat")

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

class MapEarthRenderer : public Object
{
public:
	MapEarthRenderer();
	~MapEarthRenderer();

protected:
	sl_uint32 m_nMaxLevel;
	sl_uint32 m_nY;
	sl_uint32 m_nX;
	
	SLIB_PROPERTY_SIMPLE(sl_uint32, TileLifeMillseconds);

	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxPictureTilesCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxDEMTilesCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxRenderTilesCount);

	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoader>, DataLoader);

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

public:
	void initialize();
	void release();
	void render(RenderEngine* engine, MapEnvironment* environment);

public:
	LatLon getLatLonFromTileLocation(const MapTileLocationi& location);

protected:
	sl_bool m_flagInitialized;
	Ref<Thread> m_threadControl;
	Time m_timeLastThreadControl;
	Time m_timeCurrentThreadControl;
	sl_uint32 m_dtimeThreadControl;

	void _runThreadControl();
	void _runThreadControlStep();

protected:
	Ref<MapEnvironment> m_environment;
	void _prepareRendering(RenderEngine* engine, MapEnvironment* environment);

protected:
	class _PictureTileData : public Referable
	{
	public:
		MapTileLocationi location;
		Ref<Texture> texture;
		Time timeLastAccess;
	};
	class _SortPictureTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_PictureTileData>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_PictureTileData> > m_mapPictureTiles;
	Stack<MapTileLocationi> m_listPictureTilesRequest;
protected:
	Ref<_PictureTileData> _getPictureTile(const MapTileLocationi& location);
	Ref<_PictureTileData> _loadPictureTile(const MapTileLocationi& location);
	void _freeOldPictureTiles();
	void _loadZeroLevelPictureTiles();
	void _loadRequestedPictureTiles();
	void _requestPictureTile(const MapTileLocationi& location);

protected:
	class _DEMTileData : public Referable
	{
	public:
		MapTileLocationi location;
		DEM dem;
		Time timeLastAccess;
	};
	class _SortDEMTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_DEMTileData>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_DEMTileData> > m_mapDEMTiles;
	Stack<MapTileLocationi> m_listDEMTilesRequest;
protected:
	Ref<_DEMTileData> _getDEMTile(const MapTileLocationi& location);
	Ref<_DEMTileData> _loadDEMTile(const MapTileLocationi& location);
	void _freeOldDEMTiles();
	void _loadZeroLevelDEMTiles();
	void _loadRequestedDEMTiles();
	void _requestDEMTile(const MapTileLocationi& location);

	float _getAltitudeFromDEM(float x, float y, _DEMTileData* tile);

protected:
	class _RenderTile : public Referable
	{
	public:
		MapTileLocationi location;
		GeoRectangle region;
		Vector3 positions[4];
		Vector3 positionCenter;

		Ref<_PictureTileData> picture;
		Rectangle rectanglePicture;
		Ref<_DEMTileData> dem;
		Rectangle rectangleDEM;
	};

	Ref<RenderProgram3D> m_programSurfaceTile;
	Ref<RenderProgram3D> m_programLine;

	sl_real m_sizeTileMinimum;
	sl_uint32 m_nMaxRenderTileLevel;

protected:
	void _renderTiles(RenderEngine* engine);
	void _renderTile(RenderEngine* engine, _RenderTile* tile);
	Ref<_RenderTile> _getRenderTile(const MapTileLocationi& location, _RenderTile* parent, sl_uint32 ix, sl_uint32 iy);
	sl_bool _checkTileVisible(_RenderTile* tile);
	sl_bool _checkTileExpandable(_RenderTile* tile);

protected:
	class _RenderTileCache : public Referable
	{
	public:
		MapTileLocationi location;
		Ref<_PictureTileData> picture;
		Ref<_DEMTileData> dem;
		Primitive primitive;
		Time timeLastAccess;
	};
	class _SortRenderTileCacheByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_RenderTileCache>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_RenderTileCache> > m_mapRenderTileCaches;

protected:
	Ref<_RenderTileCache> _getRenderTileCache(const MapTileLocationi& location);
	void _saveRenderTileToCache(_RenderTile* tile, Primitive& primitive);
	void _freeOldRenderTileCaches();


};
SLIB_MAP_NAMESPACE_END

#endif

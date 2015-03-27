#ifndef CHECKHEADER_SLIB_MAP_GLOBE
#define CHECKHEADER_SLIB_MAP_GLOBE

#include "definition.h"

#include "environment.h"
#include "util.h"
#include "data.h"
#include "dem.h"

#include "data_vwbuilding.h"
#include "data_gis.h"

#include "../../slib/core/object.h"
#include "../../slib/math/geograph.h"
#include "../../slib/core/map.h"
#include "../../slib/core/time.h"
#include "../../slib/image/freetype.h"

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

class MapMarker : public Referable
{
public:
	GeoLocation location;
	String text;
	Color textColor;
	Ref<FreeType> textFont;
	sl_uint32 textFontSize;

	Size iconSize;
	Ref<Texture> iconTexture;
	Rectangle iconTextureRectangle; // texture whole coordinate

public:
	Ref<Texture> _textureText;

public:
	void invalidate()
	{
		_textureText.setNull();
	}
};

class MapPolygon : public Referable
{
public:
	String key;
	List<Vector3> points;
	Color color;
	float width;
};

class MapEarthRenderer : public Object
{
public:
	MapEarthRenderer();
	~MapEarthRenderer();

protected:	
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxLevel);
	SLIB_PROPERTY_SIMPLE(sl_uint32, CountX0);
	SLIB_PROPERTY_SIMPLE(sl_uint32, CountY0);

	SLIB_PROPERTY_SIMPLE(sl_uint32, TileLifeMillseconds);

	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxPictureTilesCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxDEMTilesCount);

	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxGISLineTilesCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxGISPoiTilesCount);

	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxBuildingTilesCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MinBuildingLevel);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxBuildingsCount);
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxDetailedBuildingsCount);

	SLIB_PROPERTY_SIMPLE(sl_uint32, MaxRenderTilesCount);

	SLIB_BOOLEAN_PROPERTY(ShowBuilding);
	SLIB_BOOLEAN_PROPERTY(ShowGISLine);
	SLIB_BOOLEAN_PROPERTY(ShowGISPoi);

	SLIB_PROPERTY_SIMPLE(Ref<FreeType>, FontForPOI);

	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoader>, DataLoader);

public:
	void initialize();
	void release();
	void render(RenderEngine* engine, MapEnvironment* environment);

public:
	LatLon getLatLonFromTileLocation(const MapTileLocationi& location);
	MapTileLocation getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon);
	Vector2 convertPointToScreen(const Vector3& point);

public:
	Map< String, Ref<MapMarker> > markers;
	Map< String, Ref<MapPolygon> > polygons;

protected:
	sl_bool m_flagInitialized;
	Ref<Thread> m_threadControl;
	Ref<Thread> m_threadData;
	Ref<Thread> m_threadDataEx;

	Time m_timeLastThreadControl;
	Time m_timeCurrentThreadControl;
	sl_uint32 m_dtimeThreadControl;

	void _runThreadControl();
	void _runThreadControlStep();

	void _runThreadData();
	void _runThreadDataEx();

	void _runThreadDataStep();
	void _runThreadDataBuildingStep();
	void _runThreadDataGISStep();

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
	class _DEMTileData : public Referable
	{
	public:
		MapTileLocationi location;
		DEM dem;
		Time timeLastAccess;
	};

protected:
	void _loadRenderingTilesData();
	void _loadZeroLevelTilesData();

	float _getAltitudeFromDEM(float x, float y, _DEMTileData* tile);
	float _getAltitudeFromDEM(const Vector2& pos, _DEMTileData* tile)
	{
		return _getAltitudeFromDEM(pos.x, pos.y, tile);
	}
	float _getAltitudeFromRenderingDEM(const LatLon& loc);

	/******************************************
		Picture Tile Resource Management
	******************************************/
protected:
	class _SortPictureTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_PictureTileData>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_PictureTileData> > m_mapPictureTiles;
protected:
	Ref<_PictureTileData> _getPictureTile(const MapTileLocationi& location);
	Ref<_PictureTileData> _loadPictureTile(const MapTileLocationi& location);
	void _freeOldPictureTiles();

	/******************************************
		DEM Tile Resource Management
	******************************************/
protected:
	class _SortDEMTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_DEMTileData>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_DEMTileData> > m_mapDEMTiles;
protected:
	Ref<_DEMTileData> _getDEMTile(const MapTileLocationi& location);
	Ref<_DEMTileData> _loadDEMTile(const MapTileLocationi& location);
	void _freeOldDEMTiles();

	/******************************************
		Building Data Management
	******************************************/
protected:
	class _BuildingTileData : public Referable
	{
	public:
		MapTileLocationi location;
		List< Ref<VW_Building_ObjectInfo> > objects;
		List< Ref<VW_Building_ObjectInfo> > bridges;
		Time timeLastAccess;
	};
	class _SortBuildingTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_BuildingTileData>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_BuildingTileData> > m_mapBuildingTiles;

	class _RenderBuilding : public Referable
	{
	public:
		String key;
		Ref<VW_Building_Object> object;
		Ref<VW_Building_ObjectInfo> info;
		Time timeLastAccess;
	};
	class _SortRenderBuildingByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_RenderBuilding>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< String, Ref<_RenderBuilding> > m_mapRenderBuildings;

	struct _BuildingTextureAddress
	{
		String key;
		sl_uint32 index;
	};
	class _BuildingTexture : public Referable
	{
	public:
		String tkey;
		Ref<Texture> texture;
		Time timeLastAccess;
	};
	class _SortBuildingTextureByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_BuildingTexture>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	
	Map< String, Ref<_BuildingTexture> > m_mapBuildingTextures;
	_BuildingTextureAddress m_requestedBuildingTexture;

protected:
	Ref<_BuildingTileData> _loadBuildingTile(const MapTileLocationi& location);
	void _freeOldBuildingTiles();
	Ref<_RenderBuilding> _loadRenderBuilding(VW_Building_ObjectInfo* info);
	void _freeOldRenderBuildings();
	Ref<_BuildingTexture> _getBuildingTexture(String key, sl_uint32 index);
	Ref<_BuildingTexture> _loadBuildingTexture(String key, sl_uint32 index);
	void _requestBuildingTexture(String key, sl_uint32 index);
	String _getBuildingTextureKey(String key, sl_uint32 index);
	void _freeOldBuildingTextures();

	/******************************************
		GIS Line Data Management
	******************************************/
protected:
	struct _GISShape
	{
		Color color;
		sl_real width;
		sl_uint32 nElements;
		Ref<VertexBuffer> vb;
		Ref<Texture> texture;
	};
	class _GISLineTile : public Referable
	{
	public:
		MapTileLocationi location;
		sl_bool flagLoaded;
		List<_GISShape> shapes;
		Time timeLastAccess;
	};
	class _SortGISLineTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_GISLineTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_GISLineTile> > m_mapGISLineTiles;

protected:
	Ref<_GISLineTile> _getGISLineTile(const MapTileLocationi& location);
	Ref<_GISLineTile> _loadGISLineTile(const MapTileLocationi& location);
	void _freeOldGISLineTiles();

	/******************************************
		GIS Poi Data Management
	******************************************/
public:
	SLIB_INLINE void setPoiInformation(Map<sl_int64, Variant> poiInfo)
	{
		m_gisPoiTileLoader.setPoiInformation(poiInfo);
	}
	SLIB_INLINE void setWayNames(Map<sl_int64, String> wayNames)
	{
		m_gisLineTileLoader.setWayNames(wayNames);
	}
protected:
	Map_GIS_Poi_TileLoader m_gisPoiTileLoader;
	Map_GIS_Line_TileLoader m_gisLineTileLoader;
	struct _GISPoi
	{
		LatLon location;
		MAP_GISPOI_TYPE type;
		Ref<Texture> texture;
	};
	class _GISPoiTile : public Referable
	{
	public:
		MapTileLocationi location;
		sl_bool flagLoaded;
		List<_GISPoi> pois;
		Time timeLastAccess;
	};
	class _SortGISPoiTileByAccessTime
	{
	public:
		SLIB_INLINE static Time key(Ref<_GISPoiTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	Map< MapTileLocationi, Ref<_GISPoiTile> > m_mapGISPoiTiles;

protected:
	Ref<_GISPoiTile> _getGISPoiTile(const MapTileLocationi& location);
	Ref<_GISPoiTile> _loadGISPoiTile(const MapTileLocationi& location);
	void _freeOldGISPoiTiles();

	/******************************************
		Rendering Management
	******************************************/
protected:
	class _RenderTile : public Referable
	{
	public:
		MapTileLocationi location;
		GeoRectangle region;
		Vector3 positions[4];
		Vector3 positionsWithDEM[4];
		Vector3 positionCenter;

		Ref<_PictureTileData> picture;
		Rectangle rectanglePicture;
		Ref<_DEMTileData> dem;
		Rectangle rectangleDEM;
	};

	Ref<RenderProgram3D> m_programSurfaceTile;
	Ref<RenderProgram3D> m_programBuilding;
	Ref<RenderProgram3D> m_programLine;

	sl_real m_sizeTileMinimum;
	sl_uint32 m_nMaxRenderTileLevel;

	List<MapTileLocationi> m_listRenderedTiles;

protected:
	Ref<_RenderTile> _getRenderTile(const MapTileLocationi& location, _RenderTile* parent, sl_uint32 ix, sl_uint32 iy);
	
	void _renderTiles(RenderEngine* engine);
	void _renderTile(RenderEngine* engine, _RenderTile* tile);
	sl_bool _checkTileVisible(_RenderTile* tile);
	sl_bool _checkTileExpandable(_RenderTile* tile);

	void _renderBuildings(RenderEngine* engine);
	void _renderBuilding(RenderEngine* engine, _RenderBuilding* building);

	void _renderGISLines(RenderEngine* engine);
	void _renderGISLine(RenderEngine* engine, _GISLineTile* line);

	void _renderGISPois(RenderEngine* engine);
	void _renderGISPoi(RenderEngine* engine, _GISPoiTile* poi);

	void _renderMarkers(RenderEngine* engine);
	void _renderMarker(RenderEngine* engine, MapMarker* marker);
	void _renderPolygons(RenderEngine* engine);
	void _renderPolygon(RenderEngine* engine, MapPolygon* polygon);

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

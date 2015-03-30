#ifndef CHECKHEADER_SLIB_MAP_EARTH_RENDERER
#define CHECKHEADER_SLIB_MAP_EARTH_RENDERER

#include "definition.h"

#include "camera.h"
#include "util.h"
#include "data.h"
#include "earth.h"
#include "object.h"

#include "tile_render.h"
#include "tile_picture.h"
#include "tile_dem.h"
#include "tile_building.h"
#include "tile_gis.h"

#include "../../slib/core/object.h"
#include "../../slib/core/map.h"
#include "../../slib/core/thread.h"
#include "../../slib/core/time.h"
#include "../../slib/image/freetype.h"

SLIB_MAP_NAMESPACE_START

class MapEarthRenderer : public Object
{
public:
	MapEarthRenderer();
	~MapEarthRenderer();

protected:
	SLIB_PROPERTY_INLINE(sl_uint32, MaxLevel);
	SLIB_PROPERTY_INLINE(sl_uint32, CountX0);
	SLIB_PROPERTY_INLINE(sl_uint32, CountY0);

	SLIB_PROPERTY_INLINE(sl_uint32, MinBuildingLevel);

	SLIB_BOOLEAN_PROPERTY_INLINE(ShowBuilding);
	SLIB_BOOLEAN_PROPERTY_INLINE(ShowGISLine);
	SLIB_BOOLEAN_PROPERTY_INLINE(ShowGISPoi);

public:
	void initialize();
	void release();
	void render(RenderEngine* engine);

	void setDataLoader(Ref<MapDataLoader> loader);

	SLIB_INLINE void setWayNames(Map<sl_int64, String> wayNames)
	{
		m_tilesGISLine->setWayNames(wayNames);
	}
	SLIB_INLINE void setPoiInformation(Map<sl_int64, Variant> poiInfo)
	{
		m_tilesGISPoi->setPoiInformation(poiInfo);
	}
	SLIB_INLINE void setFontForPOI(Ref<FreeType> font)
	{
		m_tilesGISPoi->setFontForPOI(font);
	}

	SLIB_INLINE const Ref<MapCamera>& getCamera() const
	{
		return m_camera;
	}
	SLIB_INLINE const Ref<MapDEMTileManager>& getDEMTiles() const
	{
		return m_tilesDEM;
	}

public:
	LatLon getLatLonFromTileLocation(const MapTileLocationi& location);
	MapTileLocation getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon);
	Vector2 convertPointToScreen(const Vector3& point);

public:
	Map< String, Ref<MapMarker> > markers;
	Map< String, Ref<MapPolygon> > polygons;

protected:
	sl_bool m_flagInitialized;
	sl_bool m_flagStartedRendering;
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
	Ref<MapDataLoader> m_dataLoader;

	Ref<MapRenderTileManager> m_tilesRender;
	Ref<MapPictureTileManager> m_tilesPicture;
	Ref<MapDEMTileManager> m_tilesDEM;
	Ref<MapBuildingTileManager> m_tilesBuilding;
	Ref<MapGISLineTileManager> m_tilesGISLine;
	Ref<MapGISPoiTileManager> m_tilesGISPoi;

	Ref<RenderProgram3D> m_programSurfaceTile;
	Ref<RenderProgram3D> m_programBuilding;
	Ref<RenderProgram3D> m_programLine;

	sl_real m_sizeTileMinimum;
	sl_uint32 m_nMaxRenderTileLevel;

	List<MapTileLocationi> m_listRenderedTiles;

protected:
	Ref<MapCamera> m_camera;

	sl_uint32 m_viewportWidth;
	sl_uint32 m_viewportHeight;
	Vector3 m_positionEye;
	Matrix4 m_transformView;
	Matrix4 m_transformViewInverse;
	Matrix4 m_transformProjection;
	Matrix4 m_transformViewProjection;
	ViewFrustum m_viewFrustum;

protected:
	void _prepareRendering(RenderEngine* engine);
	
	void _loadZeroLevelTiles();
	void _loadRenderingTilesData();
	void _initializeShaders();

	Ref<MapRenderTile> _getRenderTile(const MapTileLocationi& location);
	void _renderTiles(RenderEngine* engine);
	void _renderTile(RenderEngine* engine, MapRenderTile* tile);
	sl_bool _checkTileVisible(MapRenderTile* tile);
	sl_bool _checkTileExpandable(MapRenderTile* tile);

	void _renderBuildings(RenderEngine* engine);
	void _renderBuilding(RenderEngine* engine, MapBuilding* building);

	void _renderGISLines(RenderEngine* engine);
	void _renderGISLine(RenderEngine* engine, MapGISLineTile* line);

	void _renderGISPois(RenderEngine* engine);
	void _renderGISPoi(RenderEngine* engine, MapGISPoiTile* poi);

	void _renderMarkers(RenderEngine* engine);
	void _renderMarker(RenderEngine* engine, MapMarker* marker);
	void _renderPolygons(RenderEngine* engine);
	void _renderPolygon(RenderEngine* engine, MapPolygon* polygon);
};

SLIB_MAP_NAMESPACE_END

#endif

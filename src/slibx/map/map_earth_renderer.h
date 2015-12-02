#ifndef CHECKHEADER_SLIB_MAP_EARTH_RENDERER
#define CHECKHEADER_SLIB_MAP_EARTH_RENDERER

#include "../../../inc/slibx/map/definition.h"
#include "../../../inc/slibx/map/camera.h"
#include "../../../inc/slibx/map/earth.h"
#include "../../../inc/slibx/map/object.h"
#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slibx/map/data.h"

#include "map_tile_render.h"
#include "map_tile_picture.h"
#include "map_tile_dem.h"
#include "map_tile_layer.h"
#include "map_tile_building.h"
#include "map_tile_gis.h"

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/thread.h>
#include <slib/core/time.h>
#include <slib/graphics/freetype.h>

SLIB_MAP_NAMESPACE_BEGIN

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
	
protected:
	sl_bool m_flagShowLayer[SLIB_SMAP_MAX_LAYERS_COUNT];
	
public:
	SLIB_INLINE sl_bool isShowLayer(sl_uint32 layer)
	{
		if (layer < SLIB_SMAP_MAX_LAYERS_COUNT) {
			return m_flagShowLayer[layer];
		} else {
			return sl_false;
		}
	}
	SLIB_INLINE void setShowLayer(sl_uint32 layer, sl_bool flag)
	{
		if (layer < SLIB_SMAP_MAX_LAYERS_COUNT) {
			m_flagShowLayer[layer] = flag;
		}
	}

public:
	void initialize();
	void release();
	void render(RenderEngine* engine);

	void setDataLoader(Ref<MapDataLoader> loader);

	SLIB_INLINE void setWayNames(Map<sl_int64, String> wayNames)
	{
		m_tilesGISLine->setWayNames(wayNames);
	}
	SLIB_INLINE void setPoiInformation(Map<sl_int64, MapGISPoiInfo> poiInfo)
	{
		m_poiInfo = poiInfo;
	}
	SLIB_INLINE Ref<FreeType> getFontForPOI()
	{
		return m_tilesGISPoi->getFontForPOI();
	}
	SLIB_INLINE void setFontForPOI(Ref<FreeType> font)
	{
		m_tilesGISPoi->setFontForPOI(font);
	}
	SLIB_INLINE void setViewportSize(const Sizef& size)
	{
		m_tilesGISPoi->setViewportSize(size);
		m_tilesGISLine->setViewportSize(size);
	}

	SLIB_INLINE const Ref<MapCamera>& getCamera() const
	{
		return m_camera;
	}
	SLIB_INLINE const Ref<MapDEMTileManager>& getDEMTiles() const
	{
		return m_tilesDEM;
	}

	SLIB_INLINE const Matrix4lf& getProjectionMatrix() const
	{
		return m_transformProjection;
	}

	SLIB_INLINE sl_real getEyeSurfaceAltitude() const
	{
		return m_altitudeEyeSurface;
	}

public:
	LatLon getLatLonFromTileLocation(const MapTileLocationi& location);
	MapTileLocation getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon);
	Vector2 convertPointToScreen(const Vector3& point);
	sl_bool getLocationFromScreenPoint(GeoLocation& out, const Vector2& point);
	GeoLocation getLocationFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways = sl_false);
	sl_real getAltitudeFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways = sl_false);

public:
	Map< String, Ref<MapMarker> > markers;
	Map< String, Ref<MapIcon> > icons;
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

	sl_real m_altitudeEyeSurface;

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
	Ref<MapLayerTileManager> m_tilesLayer;
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

	Map<sl_int64, MapGISPoiInfo> m_poiInfo;
	
protected:
	Ref<MapCamera> m_camera;

	sl_uint32 m_viewportWidth;
	sl_uint32 m_viewportHeight;
	Vector3lf m_positionEye;
	Matrix4lf m_transformView;
	Matrix4lf m_transformViewInverse;
	Matrix4lf m_transformProjection;
	Matrix4lf m_transformViewProjection;
	ViewFrustumlf m_viewFrustum;

protected:
	void _prepareRendering(RenderEngine* engine);
	
	void _loadZeroLevelTiles();
	void _loadRenderingTilesData();
	void _initializeShaders();

	class _Tile : public Referable
	{
	public:
		MapTileLocationi location;
		GeoRectangle region;
		Vector3lf positions[4];
		Vector3lf positionsWithDEM[4];
		Vector3lf positionCenter;

		Ref<MapPictureTile> picture;
		Rectangle rectanglePicture;
		Ref<MapDEMTile> dem;
		Rectangle rectangleDEM;
		Ref<MapLayerTile> layers[SLIB_SMAP_MAX_LAYERS_COUNT];
		Rectangle rectangleLayers[SLIB_SMAP_MAX_LAYERS_COUNT];
	};

	Ref<_Tile> _getTile(const MapTileLocationi& location);
	void _renderTiles(RenderEngine* engine);
	void _renderTile(RenderEngine* engine, _Tile* tile);
	sl_bool _checkPointVisible(const Vector3& point);
	sl_bool _checkTileVisible(_Tile* tile);
	sl_bool _checkTileExpandable(_Tile* tile);

	void _renderBuildings(RenderEngine* engine);
	void _renderBuilding(RenderEngine* engine, MapBuilding* building);

	void _renderGISLines(RenderEngine* engine);
	void _renderGISLine(RenderEngine* engine, MapGISLineTile* line);

	void _renderGISPois(RenderEngine* engine);
	void _renderGISPoi(RenderEngine* engine, MapGISPoi* poi, const Ref<FreeType>& font);

	void _renderMarkers(RenderEngine* engine);
	void _renderMarker(RenderEngine* engine, MapMarker* marker);
	void _renderIcons(RenderEngine* engine);
	void _renderIcon(RenderEngine* engine, MapIcon* marker);
	void _renderPolygons(RenderEngine* engine);
	void _renderPolygon(RenderEngine* engine, MapPolygon* polygon);
};

SLIB_MAP_NAMESPACE_END

#endif

#include "map_earth_renderer.h"

#include <slib/render/opengl.h>

SLIB_MAP_NAMESPACE_BEGIN

MapEarthRenderer::MapEarthRenderer()
{
	m_flagInitialized = sl_false;
	m_flagStartedRendering = sl_false;

	setMaxLevel(15);
	setCountX0(10);
	setCountY0(5);

	setMinBuildingLevel(13);

	for (sl_uint32 layer = 0; layer < SLIB_SMAP_MAX_LAYERS_COUNT; layer++) {
		m_flagShowLayer[layer] = sl_false;
	}
	setShowBuilding(sl_true);
	setShowGISLine(sl_true);
	setShowGISPoi(sl_true);

	m_nMaxRenderTileLevel = 0;

	Ref<MapCamera> camera = new MapCamera;
	m_camera = camera;
	camera->setEyeLocation(GeoLocation(38, 126, 8000000));

	m_altitudeEyeSurface = 0;
	
	m_tilesRender = new MapRenderTileManager;
	m_tilesPicture = new MapPictureTileManager;
	m_tilesLayer = new MapLayerTileManager;
	m_tilesDEM = new MapDEMTileManager;
	m_tilesBuilding = new MapBuildingTileManager;
	m_tilesGISLine = new MapGISLineTileManager;
	m_tilesGISPoi = new MapGISPoiTileManager;
}

MapEarthRenderer::~MapEarthRenderer()
{
	release();
}

void MapEarthRenderer::initialize()
{
	m_flagInitialized = sl_true;

	_initializeShaders();

	_loadZeroLevelTiles();

	m_threadControl = Thread::start(SLIB_CALLBACK_CLASS(MapEarthRenderer, _runThreadControl, this));
	m_threadData = Thread::start(SLIB_CALLBACK_CLASS(MapEarthRenderer, _runThreadData, this));
	m_threadDataEx = Thread::start(SLIB_CALLBACK_CLASS(MapEarthRenderer, _runThreadDataEx, this));
}

void MapEarthRenderer::release()
{
	MutexLocker lock(getLocker());
	m_flagInitialized = sl_false;

	if (m_threadControl.isNotNull()) {
		m_threadControl->finish();
	}
	if (m_threadData.isNotNull()) {
		m_threadData->finish();
	}
	if (m_threadDataEx.isNotNull()) {
		m_threadDataEx->finish();
	}
	if (m_threadControl.isNotNull()) {
		m_threadControl->finishAndWait();
		m_threadControl.setNull();
	}
	if (m_threadData.isNotNull()) {
		m_threadData->finishAndWait();
		m_threadData.setNull();
	}
	if (m_threadDataEx.isNotNull()) {
		m_threadDataEx->finishAndWait();
		m_threadDataEx.setNull();
	}
}

SLIB_MAP_NAMESPACE_END

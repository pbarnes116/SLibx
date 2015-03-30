#include "earth_renderer.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

MapEarthRenderer::MapEarthRenderer()
{
	m_flagInitialized = sl_false;
	m_flagStartedRendering = sl_false;

	setMaxLevel(15);
	setCountX0(10);
	setCountY0(5);

	setMinBuildingLevel(13);

	setShowBuilding(sl_true);
	setShowGISLine(sl_true);
	setShowGISPoi(sl_true);

	m_nMaxRenderTileLevel = 0;

	m_camera = new MapCamera;
	m_camera->setEyeLocation(GeoLocation(38, 126, 8000000));
	
	m_tilesRender = new MapRenderTileManager;
	m_tilesPicture = new MapPictureTileManager;
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

LatLon MapEarthRenderer::getLatLonFromTileLocation(const MapTileLocationi& location)
{
	LatLon ret;
	sl_uint32 n = 1 << (location.level);
	sl_uint32 nx = n * getCountX0();
	sl_uint32 ny = n * getCountY0();
	ret.latitude = (location.y) * 180.0 / ny - 90.0;
	ret.longitude = (location.x) * 360.0 / nx - 180.0;
	return ret;
}

MapTileLocation MapEarthRenderer::getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon)
{
	MapTileLocation ret;
	sl_uint32 n = 1 << level;
	sl_uint32 nx = n * getCountX0();
	sl_uint32 ny = n * getCountY0();
	ret.level = level;
	ret.y = (90.0 + latLon.latitude) * ny / 180.0;
	ret.x = (180.0 + latLon.longitude) * nx / 360.0;
	return ret;
}

Vector2 MapEarthRenderer::convertPointToScreen(const Vector3& point)
{
	Vector3 posScreen = Transform3::projectToViewport(m_transformViewProjection, point);
	float x = (posScreen.x + 1.0f) * m_viewportWidth / 2.0f;
	float y = (1.0f - posScreen.y) * m_viewportHeight / 2.0f;
	return Vector2(x, y);
}
SLIB_MAP_NAMESPACE_END

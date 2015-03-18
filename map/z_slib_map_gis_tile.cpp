#include "gis.h"
#include "gis_tile.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

class _MapTile_GIS_Poi : public MapTile
{
public:
	List<GIS_Poi> pois;

	_MapTile_GIS_Poi()
	{
	}
};

class _MapTile_GIS_Line : public MapTile
{
public:
	List<GIS_Line> lines;

	_MapTile_GIS_Line()
	{
	}
};

MapTileManager_GIS_Poi::MapTileManager_GIS_Poi()
{
	m_programTile = new RenderProgram3D_PositionNormalTexture_Diffuse;
}

void MapTileManager_GIS_Poi::initialize()
{
	MapTileManager::initialize();
	_initializeTopTiles(0, 5, 10);
	setMaximumLevel(15);
}

Ref<MapTile> MapTileManager_GIS_Poi::createTile()
{
	return new _MapTile_GIS_Poi;
}

sl_bool MapTileManager_GIS_Poi::initializeTile(MapTile* _tile, MapTile* _parent)
{
	_MapTile_GIS_Poi* tile = (_MapTile_GIS_Poi*)_tile;

	Ref<MapDataLoaderPack> loader = getDataLoader();
	if (loader.isNull()) {
		return sl_false;
	}

	MapTileLocation loc;
	loc.level = tile->level;
	loc.x = tile->x;
	loc.y = tile->y;

	GIS_Poi_Tile t;
	if (t.load(loader->building.getObject(), _SLT("gis/pois"), loc)) {
		tile->pois = t.pois;
	}
	return sl_true;
}

void MapTileManager_GIS_Poi::freeTile(MapTile* _tile)
{
	_MapTile_GIS_Poi* tile = (_MapTile_GIS_Poi*)_tile;
	tile->pois.clear();
}

void MapTileManager_GIS_Poi::renderTile(MapTile* _tile, RenderEngine* engine, MapEnvironment* environment)
{
	_MapTile_GIS_Poi* tile = (_MapTile_GIS_Poi*)_tile;
	ListLocker<GIS_Poi> pois(tile->pois);
	for (sl_size poiIndex = 0; poiIndex < pois.count(); poiIndex++) {
		GIS_Poi& poi = pois[poiIndex];

		//draw poi
	}
}

void MapTileManager_GIS_Poi::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	m_programTile->setAmbientColor(Color(100, 100, 100));
	m_programTile->setViewMatrix(environment->transformView);
	m_programTile->setProjectionMatrix(environment->transformProjection);
	MapTileManager::renderTiles(engine, environment);
}


MapTileManager_GIS_Line::MapTileManager_GIS_Line()
{
	m_programTile = new RenderProgram3D_PositionNormalTexture_Diffuse;
}

void MapTileManager_GIS_Line::initialize()
{
	MapTileManager::initialize();
	_initializeTopTiles(0, 5, 10);
	setMaximumLevel(15);
}

Ref<MapTile> MapTileManager_GIS_Line::createTile()
{
	return new _MapTile_GIS_Line;
}

sl_bool MapTileManager_GIS_Line::initializeTile(MapTile* _tile, MapTile* _parent)
{
	_MapTile_GIS_Line* tile = (_MapTile_GIS_Line*)_tile;

	Ref<MapDataLoaderPack> loader = getDataLoader();
	if (loader.isNull()) {
		return sl_false;
	}

	MapTileLocation loc;
	loc.level = tile->level;
	loc.x = tile->x;
	loc.y = tile->y;

	GIS_Line_Tile t;
	if (t.load(loader->building.getObject(), _SLT("gis/lines"), loc)) {
		tile->lines = t.lines;
	}
	return sl_true;
}

void MapTileManager_GIS_Line::freeTile(MapTile* _tile)
{
	_MapTile_GIS_Line* tile = (_MapTile_GIS_Line*)_tile;
	tile->lines.clear();
}

void MapTileManager_GIS_Line::renderTile(MapTile* _tile, RenderEngine* engine, MapEnvironment* environment)
{
	_MapTile_GIS_Line* tile = (_MapTile_GIS_Line*)_tile;
	ListLocker<GIS_Line> lines(tile->lines);
	for (sl_size lineIndex = 0; lineIndex < lines.count(); lineIndex++) {
		GIS_Line& line = lines[lineIndex];

		//draw line
	}
}

void MapTileManager_GIS_Line::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	m_programTile->setAmbientColor(Color(100, 100, 100));
	m_programTile->setViewMatrix(environment->transformView);
	m_programTile->setProjectionMatrix(environment->transformProjection);
	MapTileManager::renderTiles(engine, environment);
}
SLIB_MAP_NAMESPACE_END

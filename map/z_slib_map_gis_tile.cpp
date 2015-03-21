#include "data_gis.h"
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

class _MapTile_GIS_Shape : public MapTile
{
public:
	Map<sl_int32, Ref<GIS_Shape>> shapes;

	_MapTile_GIS_Shape()
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
	if (t.load(loader->gis.getObject(), _SLT("gis/poi"), loc)) {
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


MapTileManager_GIS_Shape::MapTileManager_GIS_Shape()
{
	m_programTile = new RenderProgram3D_Position;
}

void MapTileManager_GIS_Shape::initialize()
{
	MapTileManager::initialize();
	_initializeTopTiles(0, 5, 10);
	setMaximumLevel(15);
}

Ref<MapTile> MapTileManager_GIS_Shape::createTile()
{
	return new _MapTile_GIS_Shape;
}

sl_bool MapTileManager_GIS_Shape::initializeTile(MapTile* _tile, MapTile* _parent)
{
	_MapTile_GIS_Shape* tile = (_MapTile_GIS_Shape*)_tile;

	Ref<MapDataLoaderPack> loader = getDataLoader();
	if (loader.isNull()) {
		return sl_false;
	}

	MapTileLocation loc;
	loc.level = tile->level;
	loc.x = tile->x;
	loc.y = tile->y;

	GIS_Line_Tile t;
	if (t.load(loader->gis.getObject(), _SLT("gis/line"), loc)) {
		tile->shapes = t.shapes;
	}
// 	else if (t.load(loader->gis.getObject(), _SLT("gis/line1"), loc)) {
// 		tile->shapes = t.shapes;
// 	}
	return sl_true;
}

void MapTileManager_GIS_Shape::freeTile(MapTile* _tile)
{
	_MapTile_GIS_Shape* tile = (_MapTile_GIS_Shape*)_tile;
	tile->shapes.clear();
}

void MapTileManager_GIS_Shape::renderTile(MapTile* _tile, RenderEngine* engine, MapEnvironment* environment)
{
	_MapTile_GIS_Shape* tile = (_MapTile_GIS_Shape*)_tile;

	Iterator<Pair<sl_int32, Ref<GIS_Shape>>> itemIter = tile->shapes.iterator();
	Pair<sl_int32, Ref<GIS_Shape>> pairValue;
	while (itemIter.next(&pairValue)) {
		Ref<GIS_Shape> shape = pairValue.value;
		ListLocker<GIS_Line> lines(shape->lines);
		SLIB_SCOPED_ARRAY(Vector3, pos, lines.count() * 2);
		for (sl_size lineIndex = 0; lineIndex < lines.count(); lineIndex++) {
			GIS_Line& line = lines[lineIndex];
			pos[lineIndex * 2] = Earth::getCartesianPosition(line.start);
			pos[lineIndex * 2 + 1] = Earth::getCartesianPosition(line.end);
		}
		m_programTile->setDiffuseColor(shape->clr);
		sl_bool flagShow = sl_true;
		if (flagShow && (sl_int32)(_tile->level) > shape->showMinLevel - 1) {
			Ref<VertexBuffer> vb = VertexBuffer::create(pos, sizeof(Vector3)*lines.count() * 2);
			engine->draw(m_programTile, lines.count() * 2, vb, Primitive::typeLines);
		}
	}
}

void MapTileManager_GIS_Shape::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	engine->setDepthTest(sl_false);
	m_programTile->setAmbientColor(Color(100, 100, 100));
	m_programTile->setViewMatrix(environment->transformView);
	m_programTile->setProjectionMatrix(environment->transformProjection);
	MapTileManager::renderTiles(engine, environment);
}
SLIB_MAP_NAMESPACE_END

#include "vw_building.h"
#include "vw_building_tile.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

class _MapTile_VW_Building : public MapTile
{
public:
	List<VW_Building> buildings;

	_MapTile_VW_Building()
	{
	}
};

MapTileManager_VWBuilding::MapTileManager_VWBuilding()
{
	m_programTile = new RenderProgram3D_PositionNormalTexture_Diffuse;
}

void MapTileManager_VWBuilding::initialize()
{
	MapTileManager::initialize();
	_initializeTopTiles(0, 5, 10);
	setMaximumLevel(15);
}

Ref<MapTile> MapTileManager_VWBuilding::createTile()
{
	return new _MapTile_VW_Building;
}

sl_bool MapTileManager_VWBuilding::initializeTile(MapTile* _tile, MapTile* _parent)
{
	_MapTile_VW_Building* tile = (_MapTile_VW_Building*)_tile;

	Ref<MapDataLoaderPack> loader = getDataLoader();
	if (loader.isNull()) {
		return sl_false;
	}

	MapTileLocation loc;
	loc.level = tile->level;
	loc.x = tile->x;
	loc.y = tile->y;

	VW_Building_Tile t;
	if (t.load(loader->building.getObject(), _SLT("real3d/facility_build"), loc)) {
		tile->buildings = t.buildings;
	}
	return sl_true;
}

void MapTileManager_VWBuilding::freeTile(MapTile* _tile)
{
	_MapTile_VW_Building* tile = (_MapTile_VW_Building*)_tile;
	tile->buildings.clear();
}

void MapTileManager_VWBuilding::renderTile(MapTile* _tile, RenderEngine* engine, MapEnvironment* environment)
{
	_MapTile_VW_Building* tile = (_MapTile_VW_Building*)_tile;
	ListLocker<VW_Building> buildings(tile->buildings);
	for (sl_size iBuilding = 0; iBuilding < buildings.count(); iBuilding++) {
		VW_Building& building = buildings[iBuilding];
		// world matrix
		{
			GeoLocation loc;
			loc.longitude = building.longitude;
			loc.latitude = building.latitude;
			loc.altitude = building.altitude + 6;
			Vector3 pos = Earth::getCartesianPosition(loc);
			Vector3 z = pos.getNormalized();
			Vector3 x = Vector3(0, 1, 0).cross(z).getNormalized();
			Vector3 y = z.cross(x);
			Matrix4 m;
			m.setRow0(Vector4(0, 0, -1, 0));
			m.setRow1(Vector4(-1, 0, 0, 0));
			m.setRow2(Vector4(0, 1, 0, 0));
			m.setRow3(Vector4(pos, 1));
			m_programTile->setModelMatrix(m);
		}
		ListLocker<VW_Building_Mesh> meshes(building.meshes);
		for (sl_size iMesh = 0; iMesh < meshes.count(); iMesh++) {
			VW_Building_Mesh& mesh = meshes[iMesh];
			m_programTile->setTexture(mesh.texture);
			engine->draw(m_programTile, mesh.countElements, mesh.vb, mesh.ib);
		}
	}
}

void MapTileManager_VWBuilding::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	m_programTile->setAmbientColor(Color(100, 100, 100));
	m_programTile->setViewMatrix(environment->transformView);
	m_programTile->setProjectionMatrix(environment->transformProjection);
	MapTileManager::renderTiles(engine, environment);
}
SLIB_MAP_NAMESPACE_END

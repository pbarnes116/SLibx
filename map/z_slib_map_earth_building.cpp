#include "earth.h"
#include "dem.h"
#include "data_config.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::_runThreadDataBuildingStep()
{
	struct _MapEarthRenderer_BuildingInfo
	{
		Ref<VW_Building_ObjectInfo> info;
		sl_real distance;
	};
	struct _MapEarthRenderer_SortBuildingInfo
	{
	public:
		SLIB_INLINE static sl_real key(_MapEarthRenderer_BuildingInfo& info)
		{
			return info.distance;
		}
	};
	if (isShowBuilding()) {
		
		Vector3 eye = m_environment->positionEye;

		List< _MapEarthRenderer_BuildingInfo > infos;

		// load building infors
		{
			sl_uint32 nMinBuildingLevel = getMinBuildingLevel();
			ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
			for (sl_size i = 0; i < list.count(); i++) {
				MapTileLocationi loc = list[i];
				Ref<_BuildingTileData> tile;
				if (loc.level >= nMinBuildingLevel) {
					do {
						if (Thread::isStoppingCurrent()) {
							return;
						}
						tile = _loadBuildingTile(loc);
						ListLocker< Ref<VW_Building_ObjectInfo> > objects(tile->objects);
						for (sl_size k = 0; k < objects.count(); k++) {
							Ref<VW_Building_ObjectInfo>& info = objects[k];
							if (info.isNotNull()) {
								if (m_environment->viewFrustum.containsBox(info->bound)) {
									Ref<_RenderBuilding> rb;
									m_mapRenderBuildings.get(info->key, &rb);
									if (rb.isNull()) {
										_MapEarthRenderer_BuildingInfo o;
										o.info = info;
										if (info->flagBridge) {
											o.distance = 0;
										} else {
											o.distance = (eye - (Vector3)(info->bound.center())).getLength2p();
										}
										infos.add(o);
									} else {
										rb->timeLastAccess = m_timeCurrentThreadControl;
									}
								}
							}
						}
						loc.level--;
						loc.x >>= 1;
						loc.y >>= 1;
					} while (loc.level + 1 > nMinBuildingLevel);
				}
			}
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// select objects
		{
			infos = infos.sort<_MapEarthRenderer_SortBuildingInfo, sl_real>(sl_true);
			infos.setCount(Math::min(10, (sl_int32)(infos.count())));
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// load objects
		{
			ListLocker<_MapEarthRenderer_BuildingInfo> list(infos);
			for (sl_size i = 0; i < list.count(); i++) {
				_loadRenderBuilding(list[i].info);
			}
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// load building textures
		{
			_BuildingTextureAddress addr = m_requestedBuildingTexture;
			if (addr.key.isNotEmpty()) {
				_loadBuildingTexture(addr.key, addr.index);
				m_requestedBuildingTexture.key.setNull();
			}
		}
	}
	if (Thread::isStoppingCurrent()) {
		return;
	}
	_freeOldBuildingTiles();
	if (Thread::isStoppingCurrent()) {
		return;
	}
	_freeOldRenderBuildings();
	if (Thread::isStoppingCurrent()) {
		return;
	}
	_freeOldBuildingTextures();
}

Ref<MapEarthRenderer::_BuildingTileData> MapEarthRenderer::_loadBuildingTile(const MapTileLocationi& location)
{
	Ref<_BuildingTileData> tile;
	m_mapBuildingTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		return tile;
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {

		List< Ref<VW_Building_ObjectInfo> > objects;
		objects.add(VW_Building::readTile(loader, SLIB_MAP_BUILDING_TILE_TYPE, location, sl_false));
		objects.add(VW_Building::readTile(loader, SLIB_MAP_BUILDING_WORLD_TILE_TYPE, location, sl_false));
		objects.add(VW_Building::readTile(loader, SLIB_MAP_BRIDGE_TILE_TYPE, location, sl_true));

		tile = new _BuildingTileData();
		if (tile.isNotNull()) {
			tile->location = location;
			tile->objects = objects;
			tile->timeLastAccess = m_timeCurrentThreadControl;
			m_mapBuildingTiles.put(location, tile);
			return tile;
		}
	}
	return Ref<_BuildingTileData>::null();
}

void MapEarthRenderer::_freeOldBuildingTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxBuildingTilesCount();
	List< Ref<_BuildingTileData> > tiles;
	{
		ListLocker< Ref<_BuildingTileData> > t(m_mapBuildingTiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_BuildingTileData>& tile = t[i];
			if (tile.isNotNull()) {
				if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_mapBuildingTiles.remove(tile->location);
				}
			}
		}
	}
	tiles = tiles.sort<_SortBuildingTileByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_BuildingTileData> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_BuildingTileData>& tile = t[i];
			m_mapBuildingTiles.remove(tile->location);
		}
	}
}

Ref<MapEarthRenderer::_RenderBuilding> MapEarthRenderer::_loadRenderBuilding(VW_Building_ObjectInfo* info)
{
	Ref<_RenderBuilding> building;
	m_mapRenderBuildings.get(info->key, &building);
	if (building.isNotNull()) {
		building->timeLastAccess = m_timeCurrentThreadControl;
		return building;
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {

		Ref<VW_Building_Object> vo = VW_Building::readObject(loader, info);
		if (vo.isNotNull()) {
			building = new _RenderBuilding;
			if (building.isNotNull()) {
				building->key = info->key;
				building->object = vo;
				building->info = info;
				building->timeLastAccess = m_timeCurrentThreadControl;
				m_mapRenderBuildings.put(info->key, building);
				return building;
			}
		}
	}
	return Ref<_RenderBuilding>::null();
}

void MapEarthRenderer::_freeOldRenderBuildings()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxBuildingsCount() * 2;
	List< Ref<_RenderBuilding> > buildings;
	{
		ListLocker< Ref<_RenderBuilding> > t(m_mapRenderBuildings.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_RenderBuilding>& building = t[i];
			if (building.isNotNull()) {
				if ((m_timeCurrentThreadControl - building->timeLastAccess).getMillisecondsCount() < timeLimit) {
					buildings.add(building);
				} else {
					m_mapRenderBuildings.remove(building->key);
				}
			}
		}
	}
	buildings = buildings.sort<_SortRenderBuildingByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_RenderBuilding> > t(buildings);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_RenderBuilding>& building = t[i];
			m_mapRenderBuildings.remove(building->key);
		}
	}
}

Ref<MapEarthRenderer::_BuildingTexture> MapEarthRenderer::_getBuildingTexture(String key, sl_uint32 index)
{
	String tkey = _getBuildingTextureKey(key, index);
	Ref<_BuildingTexture> bt;
	m_mapBuildingTextures.get(tkey, &bt);
	if (bt.isNotNull()) {
		bt->timeLastAccess = m_timeCurrentThreadControl;
	}
	return bt;
}

Ref<MapEarthRenderer::_BuildingTexture> MapEarthRenderer::_loadBuildingTexture(String key, sl_uint32 index)
{
	String tkey = _getBuildingTextureKey(key, index);
	Ref<_BuildingTexture> bt;
	m_mapBuildingTextures.get(tkey, &bt);
	if (bt.isNotNull()) {
		bt->timeLastAccess = m_timeCurrentThreadControl;
		return bt;
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {

		Ref<_RenderBuilding> rb;
		m_mapRenderBuildings.get(key, &rb);

		if (rb.isNotNull()) {
			VW_Building_Mesh mesh;
			if (rb->object->meshes.getItem(index, &mesh)) {

				Memory mem = loader->loadData(rb->info->dataType, rb->info->location, SLIB_MAP_BUILDING_PACKAGE_DIMENSION, _SLT("/") + mesh.textureFileName);
				if (mem.isNotEmpty()) {
					Ref<Texture> texture = Texture::loadFromMemory(mem);
					bt = new _BuildingTexture;
					if (bt.isNotNull()) {
						bt->tkey = tkey;
						bt->texture = texture;
						bt->timeLastAccess = m_timeCurrentThreadControl;
						m_mapBuildingTextures.put(tkey, bt);
						return bt;
					}
				}				
			}
			
		}
	}
	return Ref<_BuildingTexture>::null();
}

void MapEarthRenderer::_requestBuildingTexture(String key, sl_uint32 index)
{
	m_requestedBuildingTexture.key = key;
	m_requestedBuildingTexture.index = index;
}

String MapEarthRenderer::_getBuildingTextureKey(String key, sl_uint32 index)
{
	return key + _SLT("_") + index;
}

void MapEarthRenderer::_freeOldBuildingTextures()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxDetailedBuildingsCount() * 2;
	List< Ref<_BuildingTexture> > buildings;
	{
		ListLocker< Ref<_BuildingTexture> > t(m_mapBuildingTextures.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_BuildingTexture>& building = t[i];
			if (building.isNotNull()) {
				if ((m_timeCurrentThreadControl - building->timeLastAccess).getMillisecondsCount() < timeLimit) {
					buildings.add(building);
				} else {
					m_mapBuildingTextures.remove(building->tkey);
				}
			}
		}
	}
	buildings = buildings.sort<_SortBuildingTextureByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_BuildingTexture> > t(buildings);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_BuildingTexture>& building = t[i];
			m_mapBuildingTextures.remove(building->tkey);
		}
	}
}

SLIB_MAP_NAMESPACE_END

#include "map_tile_gis.h"
#include "map_tile_config.h"
#include "map_data_config.h"

SLIB_MAP_NAMESPACE_BEGIN

MapGISLineTileManager::MapGISLineTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_GIS_LINE_TILES_COUNT);
}

Ref<MapGISLineTile> MapGISLineTileManager::getTile(const MapTileLocationi& location)
{
	Ref<MapGISLineTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->flagLoaded) {
			return tile;
		}
	}
	return Ref<MapGISLineTile>::null();
}

Ref<MapGISLineTile> MapGISLineTileManager::loadTile(const MapTileLocationi& location)
{
	Ref<MapGISLineTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->flagLoaded) {
			return tile;
		} else {
			return Ref<MapGISLineTile>::null();
		}
	}
	sl_real screenRatio = getViewportSize().width / 1280;
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Map< sl_int32, Ref<MapGISShapeData> > shapes = m_dataLoader.loadTile(loader, SLIB_MAP_GIS_LINE_TILE_TYPE, location);
		tile = new MapGISLineTile();
		if (tile.isNotNull()) {
			tile->location = location;
			tile->timeLastAccess = Time::now();
			tile->flagLoaded = sl_true;
			{
				ListLocker< Ref<MapGISShapeData> > ls(shapes.values());
				for (sl_size i = 0; i < ls.count(); i++) {
					Ref<MapGISShapeData> s = ls[i];
					if (s.isNotNull()) {
						if (s->showMinLevel <= location.level) {							
							Ref<MapGISShape> o = new MapGISShape;
							if (o.isNotNull()) {
								o->color = s->clr;
								o->width = s->width * screenRatio;
								o->lines = s->lines;
								tile->shapes.add(o);
							}
						}
					}
				}
			}
			m_tiles.put(location, tile);
			return Ref<MapGISLineTile>::null();
		}
	}
	return Ref<MapGISLineTile>::null();
}

class _MapGISLineTileManager_Compare
{

};
void MapGISLineTileManager::freeOldTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	List< Ref<MapGISLineTile> > tiles;
	{
		ListLocker< Ref<MapGISLineTile> > t(m_tiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<MapGISLineTile>& tile = t[i];
			if (tile.isNotNull()) {
				if ((now - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_tiles.remove(tile->location);
				}
			}
		}
	}
	class _Compare
	{
	public:
		SLIB_INLINE static int compare(const Ref<MapGISLineTile>& a, const Ref<MapGISLineTile>& b)
		{
			return Compare<Time>::compare(b->timeLastAccess, a->timeLastAccess);
		}
	};
	tiles.sortBy<_Compare>();
	{
		ListLocker< Ref<MapGISLineTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapGISLineTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}

MapGISPoiTileManager::MapGISPoiTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_GIS_POI_TILES_COUNT);
}

void MapGISPoi::init(Map<sl_int64, MapGISPoiInfo>& info)
{
	if (_flagInit) {
		return;
	}
	_flagInit = sl_true;
	MapGISPoiInfo poiInfo;
	if (!(info.get(id, &poiInfo))) {
		return;
	}
	type = (MapGISPoiData::Type)(poiInfo.type);
	text = poiInfo.name;
	if (type != MapGISPoiData::typeNone) {
		showMinLevel = 15;
		clr = Color::White;
		fontSize = 16;
		if (type == MapGISPoiData::typeNaturalLake || type == MapGISPoiData::typeNaturalMountain || type == MapGISPoiData::typeNaturalRiver) {
			showMinLevel = 5;
			clr = Color::White;
			fontSize = 18;
		}
		if (type == MapGISPoiData::typePlaceCountry) {
			showMinLevel = 4;
			clr = Color::Red;
			fontSize = 26;
		} else if (type == MapGISPoiData::typePlaceState) {
			showMinLevel = 5;
			clr = Color::Yellow;
			fontSize = 24;
		} else if (type == MapGISPoiData::typePlaceCity) {
			showMinLevel = 7;
			clr = Color::YellowGreen;
			fontSize = 22;
		} else if (type == MapGISPoiData::typePlaceTown) {
			showMinLevel = 10;
			clr = Color::LightCyan;
			fontSize = 20;
		} else if (type == MapGISPoiData::typePlaceVilliage) {
			showMinLevel = 13;
			clr = Color::White;
			fontSize = 18;
		}
	} else {
		showMinLevel = 16;
	}

}

Ref<MapGISPoiTile> MapGISPoiTileManager::getTile(const MapTileLocationi& location)
{
	Ref<MapGISPoiTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->flagLoaded) {
			return tile;
		}
	}
	return Ref<MapGISPoiTile>::null();
}

Ref<MapGISPoiTile> MapGISPoiTileManager::loadTile(const MapTileLocationi& location)
{
	
	Ref<MapGISPoiTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->flagLoaded) {
			return tile;
		} else {
			return Ref<MapGISPoiTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		List<MapGISPoiData> pois = m_dataLoader.loadTile(loader, SLIB_MAP_GIS_POI_TILE_TYPE, location);
		pois.add(m_dataLoader.loadTile(loader, SLIB_MAP_GIS_SPECIAL_POI_TILE_TYPE, location));
		pois.add(m_dataLoader.loadTile(loader, SLIB_MAP_GIS_NK_POI_TILE_TYPE, location));
		tile = new MapGISPoiTile();
		if (tile.isNotNull()) {
			tile->location = location;
			tile->timeLastAccess = Time::now();
			tile->flagLoaded = sl_true;
			{
				ListLocker<MapGISPoiData> list(pois);
				for (sl_size i = 0; i < list.count(); i++) {
					Ref<MapGISPoi> p = new MapGISPoi;
					if (p.isNotNull()) {
						p->location = list[i].location;
						p->level = location.level;
						p->id = list[i].id;
						tile->pois.add(p);
					}
				}
			}
			m_tiles.put(location, tile);
			return Ref<MapGISPoiTile>::null();
		}
	}
	return Ref<MapGISPoiTile>::null();
}

void MapGISPoiTileManager::freeOldTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	List< Ref<MapGISPoiTile> > tiles;
	{
		ListLocker< Ref<MapGISPoiTile> > t(m_tiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<MapGISPoiTile>& tile = t[i];
			if (tile.isNotNull()) {
				if ((now - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_tiles.remove(tile->location);
				}
			}
		}
	}	
	class _Compare
	{
	public:
		SLIB_INLINE static int compare(const Ref<MapGISPoiTile>& a, const Ref<MapGISPoiTile>& b)
		{
			return Compare<Time>::compare(b->timeLastAccess, a->timeLastAccess);
		}
	};
	tiles.sortBy<_Compare>();
	{
		ListLocker< Ref<MapGISPoiTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapGISPoiTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}
SLIB_MAP_NAMESPACE_END

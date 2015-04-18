#include "map_tile_gis.h"
#include "map_tile_config.h"
#include "map_data_config.h"

SLIB_MAP_NAMESPACE_START

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
							MapGISShape o;
							o.color = s->clr;
							o.width = s->width * screenRatio;
							o.lines = s->lines;
							tile->shapes.add(o);
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

void MapGISLineTileManager::freeOldTiles()
{
	class SortTile
	{
	public:
		SLIB_INLINE static Time key(Ref<MapGISLineTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};

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
	tiles = tiles.sort<SortTile, Time>(sl_false);
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

void MapGISPoi::initPoi()
{
	MAP_GIS_POI_TYPE type = _type;
	if (type != POITypeNone) {
		showMinLevel = 15;
		clr = Color::LightGoldenrodYellow;
		fontSize = 13;
		if (type == NaturalLake || type == NaturalMountain || type == NaturalRiver) {
			showMinLevel = 5;
			clr = Color::White;
			fontSize = 16;
		}
		if (type == PlaceCountry) {
			showMinLevel = 5;
			clr = Color::Red;
			fontSize = 26;
		} else if (type == PlaceState) {
			showMinLevel = 6;
			clr = Color::Yellow;
			fontSize = 24;
		} else if (type == PlaceCity) {
			showMinLevel = 7;
			clr = Color::YellowGreen;
			fontSize = 20;
		} else if (type == PlaceTown) {
			showMinLevel = 11;
			clr = Color::Orange;
			fontSize = 18;
		} else if (type == PlaceVilliage) {
			showMinLevel = 13;
			clr = Color::Orange;
			fontSize = 15;
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
		if (location.level == 5 && location.y == 113 && location.x >269) {
			int i = 1;
		}
		List<MapGISPoiData> pois = m_dataLoader.loadTile(loader, SLIB_MAP_GIS_POI_TILE_TYPE, location);
		pois.add(m_dataLoader.loadTile(loader, SLIB_MAP_GIS_SPECIAL_POI_TILE_TYPE, location));
		tile = new MapGISPoiTile();
		if (tile.isNotNull()) {
			tile->location = location;
			tile->timeLastAccess = Time::now();
			tile->flagLoaded = sl_true;
			{
				ListLocker<MapGISPoiData> list(pois);
				for (sl_size i = 0; i < list.count(); i++) {
					MapGISPoi p;
					p.location = list[i].location;
					p.id = list[i].id;
					tile->pois.add(p);
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
	class SortTile
	{
	public:
		SLIB_INLINE static Time key(Ref<MapGISPoiTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};

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
	tiles = tiles.sort<SortTile, Time>(sl_false);
	{
		ListLocker< Ref<MapGISPoiTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapGISPoiTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}
SLIB_MAP_NAMESPACE_END

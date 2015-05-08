#include "map_tile_dem.h"
#include "map_tile_config.h"
#include "map_data_config.h"

SLIB_MAP_NAMESPACE_START

MapDEMTileManager::MapDEMTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_DEM_TILES_COUNT);
}

Ref<MapDEMTile> MapDEMTileManager::getTile(const MapTileLocationi& location)
{
	Ref<MapDEMTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->dem.N != 0) {
			return tile;
		}
	}
	return Ref<MapDEMTile>::null();
}

Ref<MapDEMTile> MapDEMTileManager::getTileHierarchically(const MapTileLocationi& location, Rectangle* _rectangle)
{
	sl_int32 level = location.level;
	sl_int32 x = location.x;
	sl_int32 y = location.y;
	Rectangle& rectangle = *_rectangle;
	if (_rectangle) {
		rectangle.left = 0;
		rectangle.top = 0;
		rectangle.right = 1;
		rectangle.bottom = 1;
	}
	do {
		Ref<MapDEMTile> dem;
		dem = getTile(MapTileLocationi(level, y, x));
		if (dem.isNotNull()) {
			return dem;
		}
		if (level >= 0 && _rectangle) {
			rectangle.left /= 2;
			rectangle.top /= 2;
			rectangle.right /= 2;
			rectangle.bottom /= 2;
			if ((x & 1) == 1) {
				rectangle.left += 0.5f;
				rectangle.right += 0.5f;
			}
			if ((y & 1) == 0) {
				rectangle.top += 0.5f;
				rectangle.bottom += 0.5f;
			}
		}
		level--;
		y >>= 1;
		x >>= 1;
	} while (level >= 0);
	return Ref<MapDEMTile>::null();
}

Ref<MapDEMTile> MapDEMTileManager::loadTile(const MapTileLocationi& location)
{
	Ref<MapDEMTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->dem.N != 0) {
			return tile;
		} else {
			return Ref<MapDEMTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Memory mem = loader->loadData(SLIB_MAP_DEM_TILE_TYPE, location, SLIB_MAP_DEM_PACKAGE_DIMENSION, SLIB_MAP_DEM_TILE_EXT);
		if (mem.getSize() == 16900) {
			tile = new MapDEMTile();
			if (tile.isNotNull()) {
				tile->dem.initializeFromFloatData(65, mem.getBuf(), mem.getSize());
				tile->location = location;
				tile->timeLastAccess = Time::now();
				m_tiles.put(location, tile);
				return tile;
			}
		} else {
			tile = new MapDEMTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->timeLastAccess = Time::now();
				m_tiles.put(location, tile);
				return Ref<MapDEMTile>::null();
			}
		}
	}
	return Ref<MapDEMTile>::null();
}

Ref<MapDEMTile> MapDEMTileManager::loadTileHierarchically(const MapTileLocationi& location, Rectangle* _rectangle)
{
	sl_int32 level = location.level;
	sl_int32 x = location.x;
	sl_int32 y = location.y;
	Rectangle& rectangle = *_rectangle;
	if (_rectangle) {
		rectangle.left = 0;
		rectangle.top = 0;
		rectangle.right = 1;
		rectangle.bottom = 1;
	}
	do {
		Ref<MapDEMTile> dem;
		dem = loadTile(MapTileLocationi(level, y, x));
		if (dem.isNotNull()) {
			return dem;
		}
		if (level >= 0 && _rectangle) {
			rectangle.left /= 2;
			rectangle.top /= 2;
			rectangle.right /= 2;
			rectangle.bottom /= 2;
			if ((x & 1) == 1) {
				rectangle.left += 0.5f;
				rectangle.right += 0.5f;
			}
			if ((y & 1) == 0) {
				rectangle.top += 0.5f;
				rectangle.bottom += 0.5f;
			}
		}
		level--;
		y >>= 1;
		x >>= 1;
	} while (level >= 0);
	return Ref<MapDEMTile>::null();
}

void MapDEMTileManager::freeOldTiles()
{
	class SortTile
	{
	public:
		SLIB_INLINE static Time key(Ref<MapDEMTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};

	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	List< Ref<MapDEMTile> > tiles;
	{
		ListLocker< Ref<MapDEMTile> > t(m_tiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<MapDEMTile>& tile = t[i];
			if (tile.isNotNull()) {
				if (tile->location.level != 0) {
					if ((now - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
						tiles.add(tile);
					} else {
						m_tiles.remove(tile->location);
					}
				}
			}
		}
	}
	tiles = tiles.sortBy<SortTile, Time>(sl_false);
	{
		ListLocker< Ref<MapDEMTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapDEMTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}

float MapDEMTileManager::getAltitudeFromDEM(float x, float y, MapDEMTile* tile)
{
	if (tile) {
		return tile->dem.getAltitudeAt(x, y);
	} else {
		return 0;
	}
}

float MapDEMTileManager::getAltitude(const MapTileLocation& location)
{
	sl_int32 level = location.level;
	sl_real x = (sl_real)(location.x);
	sl_real y = (sl_real)(location.y);
	sl_int32 iy = (sl_int32)(y);
	sl_int32 ix = (sl_int32)(x);
	sl_real fy = (float)(y - iy);
	sl_real fx = (float)(x - ix);
	Ref<MapDEMTile> dem;
	dem = getTile(MapTileLocationi(level, iy, ix));
	if (dem.isNotNull()) {
		return getAltitudeFromDEM(fx, 1 - fy, dem);
	}
	return 0;
}

float MapDEMTileManager::getAltitudeHierarchically(const MapTileLocation& location)
{
	sl_int32 level = location.level;
	sl_real x = (sl_real)(location.x);
	sl_real y = (sl_real)(location.y);
	do {
		sl_int32 iy = (sl_int32)(y);
		sl_int32 ix = (sl_int32)(x);
		sl_real fy = (float)(y - iy);
		sl_real fx = (float)(x - ix);
		Ref<MapDEMTile> dem;
		dem = getTile(MapTileLocationi(level, iy, ix));
		if (dem.isNotNull()) {
			return getAltitudeFromDEM(fx, 1 - fy, dem);
		}
		level--;
		y /= 2;
		x /= 2;
	} while (level >= 0);
	return 0;
}

float MapDEMTileManager::readAltitude(const MapTileLocation& location)
{
	sl_int32 level = location.level;
	sl_real x = (sl_real)(location.x);
	sl_real y = (sl_real)(location.y);
	sl_int32 iy = (sl_int32)(y);
	sl_int32 ix = (sl_int32)(x);
	sl_real fy = (float)(y - iy);
	sl_real fx = (float)(x - ix);
	Ref<MapDEMTile> dem;
	dem = loadTile(MapTileLocationi(level, iy, ix));
	if (dem.isNotNull()) {
		return getAltitudeFromDEM(fx, 1 - fy, dem);
	}
	return 0;
}

float MapDEMTileManager::readAltitudeHierarchically(const MapTileLocation& location)
{
	sl_int32 level = location.level;
	sl_real x = (sl_real)(location.x);
	sl_real y = (sl_real)(location.y);
	do {
		sl_int32 iy = (sl_int32)(y);
		sl_int32 ix = (sl_int32)(x);
		sl_real fy = (float)(y - iy);
		sl_real fx = (float)(x - ix);
		Ref<MapDEMTile> dem;
		dem = loadTile(MapTileLocationi(level, iy, ix));
		if (dem.isNotNull()) {
			return getAltitudeFromDEM(fx, 1 - fy, dem);
		}
		level--;
		y /= 2;
		x /= 2;
	} while (level >= 0);
	return 0;
}

SLIB_MAP_NAMESPACE_END

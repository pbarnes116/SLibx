#include "tile_render.h"
#include "tile_config.h"

SLIB_MAP_NAMESPACE_START

MapRenderTileManager::MapRenderTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_RENDER_TILES_COUNT);
}

Ref<MapRenderTile> MapRenderTileManager::getTile(const MapTileLocationi& location)
{
	Ref<MapRenderTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		return tile;
	}
	return Ref<MapRenderTile>::null();
}

void MapRenderTileManager::saveTile(const MapTileLocationi& location, const Ref<MapRenderTile>& tile)
{
	m_tiles.put(tile->location, tile);
}

void MapRenderTileManager::freeOldTiles()
{
	class SortTile
	{
	public:
		SLIB_INLINE static Time key(Ref<MapRenderTile>& tile)
		{
			return tile->timeLastAccess;
		}
	};
	
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	List< Ref<MapRenderTile> > tiles;
	{
		ListLocker< Ref<MapRenderTile> > t(m_tiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<MapRenderTile>& tile = t[i];
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
		ListLocker< Ref<MapRenderTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapRenderTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}

SLIB_MAP_NAMESPACE_END


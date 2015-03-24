#include "earth.h"
#include "dem.h"

SLIB_MAP_NAMESPACE_START

Ref<MapEarthRenderer::_PictureTileData> MapEarthRenderer::_getPictureTile(const MapTileLocationi& location)
{
	Ref<_PictureTileData> tile;
	m_mapPictureTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		return tile;
	}
	return Ref<_PictureTileData>::null();
}

Ref<MapEarthRenderer::_PictureTileData> MapEarthRenderer::_loadPictureTile(const MapTileLocationi& location)
{
	Ref<_PictureTileData> tile = _getPictureTile(location);
	if (tile.isNotNull()) {
		return tile;
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Memory mem = loader->loadData(SLIB_MAP_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_TILE_EXT);
		if (mem.isNotEmpty()) {
			Ref<Texture> texture = Texture::create(Image::loadFromMemory(mem));
			if (texture.isNotNull()) {
				tile = new _PictureTileData();
				if (tile.isNotNull()) {
					tile->location = location;
					tile->texture = texture;
					tile->timeLastAccess = m_timeCurrentThreadControl;
					m_mapPictureTiles.put(location, tile);
					return tile;
				}
			}
		}
	}
	return Ref<_PictureTileData>::null();
}

void MapEarthRenderer::_freeOldPictureTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxPictureTilesCount();
	List< Ref<_PictureTileData> > tiles;
	{
		ListLocker< Ref<_PictureTileData> > t(m_mapPictureTiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_PictureTileData>& tile = t[i];
			if (tile.isNotNull()) {
				if (tile->location.level != 0) {
					if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
						tiles.add(tile);
					} else {
						m_mapPictureTiles.remove(tile->location);
					}
				}
			}
		}
	}
	tiles = tiles.sort<_SortPictureTileByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_PictureTileData> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_PictureTileData>& tile = t[i];
			m_mapPictureTiles.remove(tile->location);
		}
	}
}

void MapEarthRenderer::_loadZeroLevelPictureTiles()
{
	for (sl_uint32 iy = 0; iy < m_nY; iy++) {
		for (sl_uint32 ix = 0; ix < m_nX; ix++) {
			_loadPictureTile(MapTileLocationi(0, iy, ix));
		}
	}
}

void MapEarthRenderer::_loadRequestedPictureTiles()
{
	MapTileLocationi loc;
	while (m_listPictureTilesRequest.pop(&loc) && !Thread::isStoppingCurrent()) {
		Ref<_PictureTileData> tile;
		if (loc.level != 0) {
			do {
				tile = _loadPictureTile(loc);
				loc.level--;
				loc.x >>= 1;
				loc.y >>= 1;
			} while (tile.isNull() && loc.level > 0);
		}
	}
}

void MapEarthRenderer::_requestPictureTile(const MapTileLocationi& location)
{
	if (m_listPictureTilesRequest.count() > getMaxPictureTilesCount()) {
		m_listPictureTilesRequest.clear();
	}
	m_listPictureTilesRequest.push(location);
}



Ref<MapEarthRenderer::_DEMTileData> MapEarthRenderer::_getDEMTile(const MapTileLocationi& location)
{
	Ref<_DEMTileData> tile;
	m_mapDEMTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		return tile;
	}
	return Ref<_DEMTileData>::null();
}

Ref<MapEarthRenderer::_DEMTileData> MapEarthRenderer::_loadDEMTile(const MapTileLocationi& location)
{
	Ref<_DEMTileData> tile = _getDEMTile(location);
	if (tile.isNotNull()) {
		return tile;
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Memory mem = loader->loadData(SLIB_MAP_DEM_TILE_TYPE, location, SLIB_MAP_DEM_TILE_EXT);
		if (mem.getSize() == 16900) {
			tile = new _DEMTileData();
			if (tile.isNotNull()) {
				tile->dem.initializeFromFloatData(65, mem.getBuf(), mem.getSize());
				tile->location = location;
				tile->timeLastAccess = m_timeCurrentThreadControl;
				m_mapDEMTiles.put(location, tile);
				return tile;
			}
		}
	}
	return Ref<_DEMTileData>::null();
}

void MapEarthRenderer::_freeOldDEMTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxDEMTilesCount();
	List< Ref<_DEMTileData> > tiles;
	{
		ListLocker< Ref<_DEMTileData> > t(m_mapDEMTiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_DEMTileData>& tile = t[i];
			if (tile.isNotNull()) {
				if (tile->location.level != 0) {
					if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
						tiles.add(tile);
					} else {
						m_mapDEMTiles.remove(tile->location);
					}
				}
			}
		}
	}
	tiles = tiles.sort<_SortDEMTileByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_DEMTileData> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_DEMTileData>& tile = t[i];
			m_mapDEMTiles.remove(tile->location);
		}
	}
}

void MapEarthRenderer::_loadZeroLevelDEMTiles()
{
	for (sl_uint32 iy = 0; iy < m_nY; iy++) {
		for (sl_uint32 ix = 0; ix < m_nX; ix++) {
			_loadDEMTile(MapTileLocationi(0, iy, ix));
		}
	}
}

void MapEarthRenderer::_loadRequestedDEMTiles()
{
	MapTileLocationi loc;
	while (m_listDEMTilesRequest.pop(&loc) && !Thread::isStoppingCurrent()) {
		if (loc.level != 0) {
			Ref<_DEMTileData> tile;
			do {
				tile = _loadDEMTile(loc);
				loc.level--;
				loc.x >>= 1;
				loc.y >>= 1;
			} while (tile.isNull() && loc.level > 0);
		}
	}
}

void MapEarthRenderer::_requestDEMTile(const MapTileLocationi& location)
{
	if (m_listDEMTilesRequest.count() > getMaxDEMTilesCount()) {
		m_listDEMTilesRequest.clear();
	}
	m_listDEMTilesRequest.push(location);
}

float MapEarthRenderer::_getAltitudeFromDEM(float x, float y, _DEMTileData* tile)
{
	if (tile) {
		return tile->dem.getAltitudeAt(x, y);
	} else {
		return 0;
	}
}


Ref<MapEarthRenderer::_RenderTileCache> MapEarthRenderer::_getRenderTileCache(const MapTileLocationi& location)
{
	Ref<_RenderTileCache> tile;
	m_mapRenderTileCaches.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		return tile;
	}
	return Ref<_RenderTileCache>::null();
}

void MapEarthRenderer::_saveRenderTileToCache(_RenderTile* tile, Primitive& primitive)
{
	Ref<_RenderTileCache> cache = _getRenderTileCache(tile->location);
	if (cache.isNull()) {
		cache = new _RenderTileCache;
		if (cache.isNull()) {
			return;
		}
		m_mapRenderTileCaches.put(tile->location, cache);
	}
	cache->location = tile->location;
	cache->picture = tile->picture;
	cache->dem = tile->dem;
	cache->primitive = primitive;
	cache->timeLastAccess = m_timeCurrentThreadControl;
}

void MapEarthRenderer::_freeOldRenderTileCaches()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxRenderTilesCount();
	List< Ref<_RenderTileCache> > tiles;
	{
		ListLocker< Ref<_RenderTileCache> > t(m_mapRenderTileCaches.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_RenderTileCache>& tile = t[i];
			if (tile.isNotNull()) {
				if (tile->location.level != 0) {
					if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
						tiles.add(tile);
					} else {
						m_mapRenderTileCaches.remove(tile->location);
					}
				}
			}
		}
	}
	tiles = tiles.sort<_SortRenderTileCacheByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_RenderTileCache> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_RenderTileCache>& tile = t[i];
			m_mapRenderTileCaches.remove(tile->location);
		}
	}
}

SLIB_MAP_NAMESPACE_END

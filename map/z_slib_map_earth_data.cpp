#include "earth.h"
#include "dem.h"
#include "data_config.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::_runThreadData()
{
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		if (m_environment.isNotNull()) {
			_runThreadDataStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now - m_timeLastThreadControl).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
	}
}

void MapEarthRenderer::_runThreadDataEx()
{
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		if (m_environment.isNotNull()) {
			_runThreadDataBuildingStep();
			_runThreadDataGISStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now - m_timeLastThreadControl).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
	}
}

void MapEarthRenderer::_runThreadDataStep()
{
	_loadRenderingTilesData();

	_freeOldPictureTiles();
	_freeOldDEMTiles();

	_freeOldRenderTileCaches();
}

void MapEarthRenderer::_loadRenderingTilesData()
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		// load picture
		{
			MapTileLocationi loc = list[i];
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
		// load dem
		{
			MapTileLocationi loc = list[i];
			Ref<_DEMTileData> tile;
			if (loc.level != 0) {
				do {
					tile = _loadDEMTile(loc);
					loc.level--;
					loc.x >>= 1;
					loc.y >>= 1;
				} while (tile.isNull() && loc.level > 0);
			}
		}
	}
}

void MapEarthRenderer::_loadZeroLevelTilesData()
{
	for (sl_uint32 iy = 0; iy < _getCountY0(); iy++) {
		for (sl_uint32 ix = 0; ix < _getCountX0(); ix++) {
			_loadPictureTile(MapTileLocationi(0, iy, ix));
			_loadDEMTile(MapTileLocationi(0, iy, ix));
		}
	}
}

float MapEarthRenderer::_getAltitudeFromDEM(float x, float y, _DEMTileData* tile)
{
	if (tile) {
		return tile->dem.getAltitudeAt(x, y);
	} else {
		return 0;
	}
}

float MapEarthRenderer::_getAltitudeFromRenderingDEM(const LatLon& latLon)
{
	sl_int32 _level = _getMaxLevel() - 3;
	sl_int32 level = _level;
	do {
		MapTileLocation loc = getTileLocationFromLatLon(level, latLon);
		sl_int32 iy = (sl_int32)(loc.y);
		sl_int32 ix = (sl_int32)(loc.x);
		sl_real fy = (float)(loc.y - iy);
		sl_real fx = (float)(loc.x - ix);
		sl_real e = 0.1f;
		sl_real d = 0.9f;
		Ref<_DEMTileData> dem;
		dem = _getDEMTile(MapTileLocationi(level, iy, ix));
		if (dem.isNotNull()) {
			return _getAltitudeFromDEM(fx, fy, dem);
		}
		if (fx < e) {
			dem = _getDEMTile(MapTileLocationi(level, iy, ix - 1));
			if (dem.isNotNull()) {
				return _getAltitudeFromDEM(1, fy, dem);
			}
			if (fy < e) {
				dem = _getDEMTile(MapTileLocationi(level, iy - 1, ix - 1));
				if (dem.isNotNull()) {
					return _getAltitudeFromDEM(1, 1, dem);
				}
			}
			if (fy > d) {
				dem = _getDEMTile(MapTileLocationi(level, iy + 1, ix - 1));
				if (dem.isNotNull()) {
					return _getAltitudeFromDEM(1, 0, dem);
				}
			}
		}
		if (fx > d) {
			dem = _getDEMTile(MapTileLocationi(level, iy, ix + 1));
			if (dem.isNotNull()) {
				return _getAltitudeFromDEM(0, fy, dem);
			}
			if (fy < e) {
				dem = _getDEMTile(MapTileLocationi(level, iy - 1, ix + 1));
				if (dem.isNotNull()) {
					return _getAltitudeFromDEM(0, 1, dem);
				}
			}
			if (fy > d) {
				dem = _getDEMTile(MapTileLocationi(level, iy + 1, ix + 1));
				if (dem.isNotNull()) {
					return _getAltitudeFromDEM(0, 0, dem);
				}
			}
		}
		if (fy < e) {
			dem = _getDEMTile(MapTileLocationi(level, iy - 1, ix));
			if (dem.isNotNull()) {
				return _getAltitudeFromDEM(fx, 1, dem);
			}
		}
		if (fy > d) {
			dem = _getDEMTile(MapTileLocationi(level, iy + 1, ix));
			if (dem.isNotNull()) {
				return _getAltitudeFromDEM(fx, 0, dem);
			}
		}
		level--;
	} while (level >= 0);
	return 0;
}

Ref<MapEarthRenderer::_PictureTileData> MapEarthRenderer::_getPictureTile(const MapTileLocationi& location)
{
	Ref<_PictureTileData> tile;
	m_mapPictureTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->texture.isNotNull()) {
			return tile;
		}
	}
	return Ref<_PictureTileData>::null();
}

Ref<MapEarthRenderer::_PictureTileData> MapEarthRenderer::_loadPictureTile(const MapTileLocationi& location)
{
	Ref<_PictureTileData> tile;
	m_mapPictureTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->texture.isNotNull()) {
			return tile;
		} else {
			return Ref<_PictureTileData>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Memory mem = loader->loadData(SLIB_MAP_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_PICTURE_TILE_EXT);
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
		} else {
			tile = new _PictureTileData();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->texture = Ref<Texture>::null();
				tile->timeLastAccess = m_timeCurrentThreadControl;
				m_mapPictureTiles.put(location, tile);
				return Ref<_PictureTileData>::null();
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

Ref<MapEarthRenderer::_DEMTileData> MapEarthRenderer::_getDEMTile(const MapTileLocationi& location)
{
	Ref<_DEMTileData> tile;
	m_mapDEMTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->dem.N != 0) {
			return tile;
		}
	}
	return Ref<_DEMTileData>::null();
}

Ref<MapEarthRenderer::_DEMTileData> MapEarthRenderer::_loadDEMTile(const MapTileLocationi& location)
{
	Ref<_DEMTileData> tile;
	m_mapDEMTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->dem.N != 0) {
			return tile;
		} else {
			return Ref<_DEMTileData>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Memory mem = loader->loadData(SLIB_MAP_DEM_TILE_TYPE, location, SLIB_MAP_DEM_PACKAGE_DIMENSION, SLIB_MAP_DEM_TILE_EXT);
		if (mem.getSize() == 16900) {
			tile = new _DEMTileData();
			if (tile.isNotNull()) {
				tile->dem.initializeFromFloatData(65, mem.getBuf(), mem.getSize());
				tile->location = location;
				tile->timeLastAccess = m_timeCurrentThreadControl;
				m_mapDEMTiles.put(location, tile);
				return tile;
			}
		} else {
			tile = new _DEMTileData();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->timeLastAccess = m_timeCurrentThreadControl;
				m_mapDEMTiles.put(location, tile);
				return Ref<_DEMTileData>::null();
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
				if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_mapRenderTileCaches.remove(tile->location);
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

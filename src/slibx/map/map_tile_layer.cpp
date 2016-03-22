#include "map_tile_layer.h"
#include "map_tile_config.h"
#include "map_data_config.h"

#include <slib/graphics/image.h>

SLIB_MAP_NAMESPACE_BEGIN

MapLayerTileManager::MapLayerTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_PICTURE_TILES_COUNT);
}

MapLayerTileManager::~MapLayerTileManager()
{
}

Ref<MapLayerTile> MapLayerTileManager::getTile(sl_uint32 layer, const MapTileLocationi& location)
{
	if (layer >= SLIB_SMAP_MAX_LAYERS_COUNT) {
		return Ref<MapLayerTile>::null();
	}
	Ref<MapLayerTile> tile;
	m_tiles[layer].get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->texture.isNotNull()) {
			return tile;
		}
	}
	return Ref<MapLayerTile>::null();
}

Ref<MapLayerTile> MapLayerTileManager::getTileHierarchically(sl_uint32 layer, const MapTileLocationi& location, Rectangle* _rectangle)
{
	if (layer >= SLIB_SMAP_MAX_LAYERS_COUNT) {
		return Ref<MapLayerTile>::null();
	}
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
		Ref<MapLayerTile> tile;
		tile = getTile(layer, MapTileLocationi(level, y, x));
		if (tile.isNotNull()) {
			return tile;
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
	return Ref<MapLayerTile>::null();
}

class _MapLayerTitleManager_GoogleEarthTileLoader : public IMapTileDataLoader
{
public:
	Ref<MapDataLoader> dataLoader;
	
	Memory loadData(const MapTileLocationi& location, const String& subPath)
	{
		Memory ret = dataLoader->loadData("gm/gge", location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, subPath);
		if (ret.isNull()) {
			ret = dataLoader->loadData("gm/gg", location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, subPath);
		}
		return ret;
	}
};

Ref<MapLayerTile> MapLayerTileManager::loadTile(sl_uint32 layer, const MapTileLocationi& location)
{
	if (layer >= SLIB_SMAP_MAX_LAYERS_COUNT) {
		return Ref<MapLayerTile>::null();
	}
	Ref<MapLayerTile> tile;
	m_tiles[layer].get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->texture.isNotNull()) {
			return tile;
		} else {
			return Ref<MapLayerTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Ref<Image> image;
		if (layer == 0) {
			Memory mem = loader->loadData(SLIB_MAP_VCA_DAUM_MAP_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_VCA_DAUM_MAP_TILE_EXT);
			if (mem.isNotEmpty()) {
				image = Image::loadFromMemory(mem);
			}
		} else if (layer == 1) {
			Memory mem = loader->loadData(SLIB_MAP_VCA_SEA_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_VCA_SEA_TILE_EXT);
			if (mem.isNotEmpty()) {
				image = Image::loadFromMemory(mem);
			}
		} else if (layer == 2) {
			if (location.level < 10) {
				image = Image::create(16, 16);
				if (image.isNotNull()) {
					image->fillColor(Color::zero());
				}
			} else {
				_MapLayerTitleManager_GoogleEarthTileLoader tl;
				tl.dataLoader = loader;
				sl_bool flagAlpha;
				sl_int32 n = 1 << (location.level + 3);
				double lat0 = location.y * 36.0 / (1 << location.level) - 90.0;
				double lon0 = location.x * 36.0 / (1 << location.level) - 180.0;
				double lat1 = (location.y + 1) * 36.0 / (1 << location.level) - 90.0;
				double lon1 = (location.x + 1) * 36.0 / (1 << location.level) - 180.0;
				double Y0 = (lat0 + 90) / 180 * (n / 2);
				double X0 = (lon0 + 180) / 360 * n;
				double Y1 = (lat1 + 90) / 180 * (n / 2);
				double X1 = (lon1 + 180) / 360 * n;
				image = MapPictureUtil::capturePictureFromTiles(&tl, 256, 256, location.level + 3, X0, Y0, X1, Y1, sl_true, flagAlpha);
			}
		}
		Ref<Texture> texture = Texture::create(image);
		if (texture.isNotNull()) {
			tile = new MapLayerTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->texture = texture;
				tile->timeLastAccess = Time::now();
				m_tiles[layer].put(location, tile);
				return tile;
			}
		} else {
			tile = new MapLayerTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->texture = Ref<Texture>::null();
				tile->timeLastAccess = Time::now();
				m_tiles[layer].put(location, tile);
				return Ref<MapLayerTile>::null();
			}
		}
	}
	return Ref<MapLayerTile>::null();
}

Ref<MapLayerTile> MapLayerTileManager::loadTileHierarchically(sl_uint32 layer, const MapTileLocationi& location, Rectangle* _rectangle)
{
	if (layer >= SLIB_SMAP_MAX_LAYERS_COUNT) {
		return Ref<MapLayerTile>::null();
	}
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
	Ref<MapLayerTile> ret;
	do {
		Ref<MapLayerTile> tile;
		tile = loadTile(layer, MapTileLocationi(level, y, x));
		if (ret.isNull()) {
			if (tile.isNotNull()) {
				ret = tile;
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
		}
		level--;
		y >>= 1;
		x >>= 1;
	} while (level >= 0);
	return ret;
}


class _MapLayerTileManager_Compare
{
public:
	SLIB_INLINE static int compare(const Ref<MapLayerTile>& a, const Ref<MapLayerTile>& b)
	{
		return Compare<Time>::compare(b->timeLastAccess, a->timeLastAccess);
	}
};
void MapLayerTileManager::freeOldTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	for (sl_uint32 layer = 0; layer < SLIB_SMAP_MAX_LAYERS_COUNT; layer++){
		List< Ref<MapLayerTile> > tiles;
		ListLocker< Ref<MapLayerTile> > t(m_tiles[layer].values());
		for (sl_size i = 0; i < t.count; i++) {
			Ref<MapLayerTile>& tile = t[i];
			if (tile.isNotNull()) {
				if (tile->location.level != 0) {
					if ((now - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
						tiles.add(tile);
					} else {
						m_tiles[layer].remove(tile->location);
					}
				}
			}
		}
		tiles.sortBy<_MapLayerTileManager_Compare>();
		{
			ListLocker< Ref<MapLayerTile> > t(tiles);
			for (sl_size i = tileLimit; i < t.count; i++) {
				Ref<MapLayerTile>& tile = t[i];
				m_tiles[layer].remove(tile->location);
			}
		}
	}
}

SLIB_MAP_NAMESPACE_END


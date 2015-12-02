#include "map_tile_picture.h"
#include "map_tile_config.h"
#include "map_data_config.h"

#include <slib/graphics/image.h>

SLIB_MAP_NAMESPACE_BEGIN

MapPictureTileManager::MapPictureTileManager()
{
	setTileLifeMillseconds(SLIB_MAP_TILE_LIFE_MILLISECONDS);
	setMaxTilesCount(SLIB_MAP_MAX_PICTURE_TILES_COUNT);
}

Ref<MapPictureTile> MapPictureTileManager::getTile(const MapTileLocationi& location)
{
	Ref<MapPictureTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->texture.isNotNull()) {
			return tile;
		}
	}
	return Ref<MapPictureTile>::null();
}

Ref<MapPictureTile> MapPictureTileManager::getTileHierarchically(const MapTileLocationi& location, Rectangle* _rectangle)
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
		Ref<MapPictureTile> tile;
		tile = getTile(MapTileLocationi(level, y, x));
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
	return Ref<MapPictureTile>::null();
}

Ref<MapPictureTile> MapPictureTileManager::loadTile(const MapTileLocationi& location)
{
	Ref<MapPictureTile> tile;
	m_tiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = Time::now();
		if (tile->texture.isNotNull()) {
			return tile;
		} else {
			return Ref<MapPictureTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		ImageFileType type = imageFileTypeUnknown;
		Ref<Image> image;
		// first load x picture
		{
			Memory mem = loader->loadData(SLIB_MAP_PICTUREX_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_PICTUREX_TILE_EXT);
			if (mem.isEmpty()) {
				mem = loader->loadData(SLIB_MAP_PICTUREX_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_PICTUREX_TILE_EXT);
			}
			if (mem.isNotEmpty()) {
				type = Image::getFileType(mem);
				image = Image::loadFromMemory(mem);
			}
		}
		if (image.isNull() || type == imageFileTypePNG) {
			Ref<Image> imagex = image;
			image.setNull();
			Memory mem;
			if (location.level >= 14) {
				mem = loader->loadData(SLIB_MAP_VCA_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_VCA_PICTURE_TILE_EXT);
				if (mem.isEmpty()) {
					mem = loader->loadData(SLIB_MAP_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_PICTURE_TILE_EXT);
				}
			} else {
				mem = loader->loadData(SLIB_MAP_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_PICTURE_TILE_EXT);
				if (mem.isEmpty()) {
					mem = loader->loadData(SLIB_MAP_VCA_PICTURE_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_VCA_PICTURE_TILE_EXT);
				}
			}
			if (mem.isEmpty()) {
				mem = loader->loadData(SLIB_MAP_VCA_GEARTH_TILE_TYPE, location, SLIB_MAP_PICTURE_PACKAGE_DIMENSION, SLIB_MAP_VCA_GEARTH_TILE_EXT);
			}
			if (mem.isNotEmpty()) {
				image = Image::loadFromMemory(mem);
				if (imagex.isNotNull()) {
					image->drawImage(
						0, 0, image->getWidth(), image->getHeight()
						, imagex, 0, 0, imagex->getWidth(), imagex->getHeight(), blendSrcAlpha);
				}
			}
		}
		Ref<Texture> texture = Texture::create(image);
		if (texture.isNotNull()) {
			tile = new MapPictureTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->texture = texture;
				tile->timeLastAccess = Time::now();
				m_tiles.put(location, tile);
				return tile;
			}
		} else {
			tile = new MapPictureTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->texture = Ref<Texture>::null();
				tile->timeLastAccess = Time::now();
				m_tiles.put(location, tile);
				return Ref<MapPictureTile>::null();
			}
		}
	}
	return Ref<MapPictureTile>::null();
}

Ref<MapPictureTile> MapPictureTileManager::loadTileHierarchically(const MapTileLocationi& location, Rectangle* _rectangle)
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
	Ref<MapPictureTile> ret;
	do {
		Ref<MapPictureTile> tile;
		tile = loadTile(MapTileLocationi(level, y, x));
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

void MapPictureTileManager::freeOldTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxTilesCount();
	Time now = Time::now();

	List< Ref<MapPictureTile> > tiles;
	{
		ListLocker< Ref<MapPictureTile> > t(m_tiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<MapPictureTile>& tile = t[i];
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

	class _Compare
	{
	public:
		SLIB_INLINE static int compare(const Ref<MapPictureTile>& a, const Ref<MapPictureTile>& b)
		{
			return Compare<Time>::compare(b->timeLastAccess, a->timeLastAccess);
		}
	};
	tiles.sortBy<_Compare>();
	{
		ListLocker< Ref<MapPictureTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<MapPictureTile>& tile = t[i];
			m_tiles.remove(tile->location);
		}
	}
}

SLIB_MAP_NAMESPACE_END


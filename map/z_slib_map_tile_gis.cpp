#include "tile_gis.h"
#include "tile_config.h"
#include "data_config.h"

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
							o.width = s->width;
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
	Ref<FreeType> font = getFontForPOI();
	if (font.isNull()) {
		return Ref<MapGISPoiTile>::null();
	}
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
					p.type = list[i].type;
					if (list[i].showMinLevel <= (sl_int32)(location.level)) {
						String text = list[i].name;
						if (text.length() > 50) {
							text = text.substring(0, 50);
						}
						font->setSize(list[i].fontSize);
						Sizei size = font->getStringExtent(text);
						Ref<Image> image = Image::create(size.width + 5, size.height + 5);
						if (image.isNotNull()) {
							font->drawString(image, 1, size.height + 1, text, Color::Black);
							font->drawString(image, 2, size.height + 2, text, list[i].clr);
							Ref<Texture> texture = Texture::create(image);
							if (texture.isNotNull()) {
								p.texture = texture;
								tile->pois.add(p);
							}
						}
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

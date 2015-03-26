#include "earth.h"
#include "data_config.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::_runThreadDataGISStep()
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		// load gis line
		{
			MapTileLocationi loc = list[i];
			_loadGISLineTile(loc);
		}
		// load gis poi
		{
			MapTileLocationi loc = list[i];
			_loadGISPoiTile(loc);
		}
	}

	_freeOldGISLineTiles();
	_freeOldGISPoiTiles();

}

Ref<MapEarthRenderer::_GISLineTile> MapEarthRenderer::_getGISLineTile(const MapTileLocationi& location)
{
	Ref<_GISLineTile> tile;
	m_mapGISLineTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->flagLoaded) {
			return tile;
		}
	}
	return Ref<_GISLineTile>::null();
}

Ref<MapEarthRenderer::_GISLineTile> MapEarthRenderer::_loadGISLineTile(const MapTileLocationi& location)
{
	Ref<_GISLineTile> tile;
	m_mapGISLineTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->flagLoaded) {
			return tile;
		} else {
			return Ref<_GISLineTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		Map_GIS_Line_Tile line;
		if (line.load(loader, SLIB_MAP_GIS_LINE_TILE_TYPE, location)) {
			tile = new _GISLineTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->timeLastAccess = m_timeCurrentThreadControl;
				tile->flagLoaded = sl_true;
				tile->data = line;
				{
					ListLocker< Ref<Map_GIS_Shape> > ls(line.shapes.values());
					for (sl_size i = 0; i < ls.count(); i++) {
						Ref<Map_GIS_Shape> s = ls[i];
						if (s.isNotNull()) {
							if (s->showMinLevel <= location.level) {
								ListLocker<Map_GIS_Line> ll(s->lines);
								SLIB_SCOPED_ARRAY(Vector3, v, ll.count() * 2);
								for (sl_size k = 0; k < ll.count(); k++) {
									float sa = 0; // _getAltitudeFromRenderingDEM(ll[k].start);
									v[k * 2] = MapEarth::getCartesianPosition(GeoLocation(ll[k].start, sa));
									float ea = 0; // _getAltitudeFromRenderingDEM(ll[k].end);
									v[k * 2 + 1] = MapEarth::getCartesianPosition(GeoLocation(ll[k].end, ea));
								}
								Ref<VertexBuffer> vb = VertexBuffer::create(v, ll.count() * 2 * sizeof(Vector3));
								if (vb.isNotNull()) {
									_GISShape o;
									o.nElements = ll.count() * 2;
									o.vb = vb;
									o.color = s->clr;
									tile->shapes.add(o);
								}
							}
						}
					}
				}
				m_mapGISLineTiles.put(location, tile);
				return Ref<_GISLineTile>::null();
			}
		} else {
			tile = new _GISLineTile();
			if (tile.isNotNull()) {
				tile->location = location;
				tile->timeLastAccess = m_timeCurrentThreadControl;
				tile->flagLoaded = sl_false;
				m_mapGISLineTiles.put(location, tile);
				return Ref<_GISLineTile>::null();
			}
		}
	}
	return Ref<_GISLineTile>::null();
}

void MapEarthRenderer::_freeOldGISLineTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxGISLineTilesCount();
	List< Ref<_GISLineTile> > tiles;
	{
		ListLocker< Ref<_GISLineTile> > t(m_mapGISLineTiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_GISLineTile>& tile = t[i];
			if (tile.isNotNull()) {
				if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_mapGISLineTiles.remove(tile->location);
				}
			}
		}
	}
	tiles = tiles.sort<_SortGISLineTileByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_GISLineTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_GISLineTile>& tile = t[i];
			m_mapGISLineTiles.remove(tile->location);
		}
	}
}



Ref<MapEarthRenderer::_GISPoiTile> MapEarthRenderer::_getGISPoiTile(const MapTileLocationi& location)
{
	Ref<_GISPoiTile> tile;
	m_mapGISPoiTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->flagLoaded) {
			return tile;
		}
	}
	return Ref<_GISPoiTile>::null();
}

Ref<MapEarthRenderer::_GISPoiTile> MapEarthRenderer::_loadGISPoiTile(const MapTileLocationi& location)
{
	Ref<FreeType> font = _getFontForPOI();
	if (font.isNull()) {
		return Ref<_GISPoiTile>::null();
	}
	Ref<_GISPoiTile> tile;
	m_mapGISPoiTiles.get(location, &tile);
	if (tile.isNotNull()) {
		tile->timeLastAccess = m_timeCurrentThreadControl;
		if (tile->flagLoaded) {
			return tile;
		} else {
			return Ref<_GISPoiTile>::null();
		}
	}
	Ref<MapDataLoader> loader = getDataLoader();
	if (loader.isNotNull()) {
		List<Map_GIS_Poi> pois = m_gisPoiTileLoader.loadTile(loader, SLIB_MAP_GIS_POI_TILE_TYPE, location);
		tile = new _GISPoiTile();
		if (tile.isNotNull()) {
			tile->location = location;
			tile->timeLastAccess = m_timeCurrentThreadControl;
			tile->flagLoaded = sl_true;
			{
				ListLocker<Map_GIS_Poi> list(pois);
				for (sl_size i = 0; i < list.count(); i++) {
					_GISPoi p;
					p.location = list[i].location;
					p.type = list[i].type;
					if (p.type == PlaceState || p.type == PlaceCountry) {
						String text = list[i].name;
						if (text.length() > 50) {
							text = text.substring(0, 50);
						}
						Sizei size = font->getStringExtent(text);
						Ref<Image> image = Image::create(size.width + 5, size.height + 5);
						if (image.isNotNull()) {
							font->drawString(image, 2, size.height + 2, text, Color::red());
							Ref<Texture> texture = Texture::create(image);
							if (texture.isNotNull()) {
								p.texture = texture;
								tile->pois.add(p);
							}
						}
					}
					
				}
			}
			m_mapGISPoiTiles.put(location, tile);
			return Ref<_GISPoiTile>::null();
		}
	}
	return Ref<_GISPoiTile>::null();
}

void MapEarthRenderer::_freeOldGISPoiTiles()
{
	sl_int64 timeLimit = getTileLifeMillseconds();
	sl_uint32 tileLimit = getMaxGISPoiTilesCount();
	List< Ref<_GISPoiTile> > tiles;
	{
		ListLocker< Ref<_GISPoiTile> > t(m_mapGISPoiTiles.values());
		for (sl_size i = 0; i < t.count(); i++) {
			Ref<_GISPoiTile>& tile = t[i];
			if (tile.isNotNull()) {
				if ((m_timeCurrentThreadControl - tile->timeLastAccess).getMillisecondsCount() < timeLimit) {
					tiles.add(tile);
				} else {
					m_mapGISPoiTiles.remove(tile->location);
				}
			}
		}
	}
	tiles = tiles.sort<_SortGISPoiTileByAccessTime, Time>(sl_false);
	{
		ListLocker< Ref<_GISPoiTile> > t(tiles);
		for (sl_size i = tileLimit; i < t.count(); i++) {
			Ref<_GISPoiTile>& tile = t[i];
			m_mapGISPoiTiles.remove(tile->location);
		}
	}
}
SLIB_MAP_NAMESPACE_END

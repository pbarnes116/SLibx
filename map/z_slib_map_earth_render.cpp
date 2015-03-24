
#include "earth.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::render(RenderEngine* engine, MapEnvironment* environment)
{
	if (!m_flagInitialized) {
		return;
	}

	_prepareRendering(engine, environment);

	_renderTiles(engine);
}

void MapEarthRenderer::_renderTile(RenderEngine* engine, _RenderTile* tile)
{
	m_programSurfaceTile->setTexture(tile->picture->texture);

	sl_bool flagUseCache = sl_false;
	Ref<_RenderTileCache> cache = _getRenderTileCache(tile->location);
	if (cache.isNotNull()) {
		if (cache->picture == tile->picture && cache->dem == tile->dem) {
			flagUseCache = sl_true;
		}
	}
	if (flagUseCache) {
		engine->draw(m_programSurfaceTile, &(cache->primitive));
	} else {
		Primitive primitive;
		if (tile->dem.isNotNull()) {
			tile->dem->dem.makeMeshFromSphericalGlobe(MapEarth::getGlobe(), primitive, 17, tile->region, tile->rectangleDEM, tile->rectanglePicture);
		} else {
			DEM dem;
			dem.makeMeshFromSphericalGlobe(MapEarth::getGlobe(), primitive, 17, tile->region, Rectangle(0, 0, 1, 1), tile->rectanglePicture);
		}
		engine->draw(m_programSurfaceTile, &(primitive));
		_saveRenderTileToCache(tile, primitive);
	}
}

void MapEarthRenderer::_prepareRendering(RenderEngine* engine, MapEnvironment* environment)
{
	m_environment = environment;

	m_programSurfaceTile->setViewMatrix(environment->transformView);
	m_programSurfaceTile->setProjectionMatrix(environment->transformProjection);

	m_programLine->setViewMatrix(environment->transformView);
	m_programLine->setProjectionMatrix(environment->transformProjection);

	m_sizeTileMinimum = (65536.0f / m_environment->viewportWidth / m_environment->viewportWidth) * 4;
}

void MapEarthRenderer::_renderTiles(RenderEngine* engine)
{
	List< Ref<_RenderTile> > listRender;
	List< Ref<_RenderTile> > listCurrent;
	for (sl_uint32 y = 0; y < 5; y++) {
		for (sl_uint32 x = 0; x < 10; x++) {
			Ref<_RenderTile> tile = _getRenderTile(MapTileLocationi(0, y, x), sl_null, 0, 0);
			if (tile.isNotNull()) {
				listCurrent.add(tile);
			}
		}
	}
	for (sl_uint32 level = 0; level <= m_nMaxLevel; level++) {
		List< Ref<_RenderTile> > listExpand;
		// find expand
		{
			sl_bool flagExpand = sl_true;
			if (level == m_nMaxLevel) {
				flagExpand = sl_false;
			}
			ListLocker< Ref<_RenderTile> > current(listCurrent);
			for (sl_size i = 0; i < current.count(); i++) {
				Ref<_RenderTile> tile = current[i];
				if (_checkTileVisible(tile)) {
					if (flagExpand && _checkTileExpandable(tile)) {
						listExpand.add(tile);
					} else {
						listRender.add(tile);
					}
				}
			}
		}
		// expand
		{
			listCurrent = List< Ref<_RenderTile> >::null();
			ListLocker< Ref<_RenderTile> > expand(listExpand);
			for (sl_size i = 0; i < expand.count(); i++) {
				Ref<_RenderTile> tile = expand[i];
				for (sl_uint32 iy = 0; iy < 2; iy++) {
					for (sl_uint32 ix = 0; ix < 2; ix++) {
						sl_uint32 ty = (tile->location.y << 1) + iy;
						sl_uint32 tx = (tile->location.x << 1) + ix;
						Ref<_RenderTile> tileNew = _getRenderTile(MapTileLocationi(level + 1, ty, tx), tile, ix, iy);
						if (tileNew.isNotNull()) {
							listCurrent.add(tileNew);
						}
					}
				}
			}
		}
	}

	sl_uint32 nLevelMax = 0;
	// prepare render surface tiles
	{
		ListLocker< Ref<_RenderTile> > list(listRender);
		for (sl_size i = 0; i < list.count(); i++) {
			const Ref<_RenderTile>& tile = list[i];
			if (tile->location.level > nLevelMax) {
				nLevelMax = tile->location.level;
			}
			_renderTile(engine, tile);
		}
	}
	m_nMaxRenderTileLevel = nLevelMax;
}

Ref<MapEarthRenderer::_RenderTile> MapEarthRenderer::_getRenderTile(const MapTileLocationi& location, _RenderTile* parent, sl_uint32 ix, sl_uint32 iy)
{
	Ref<_RenderTile> ret;
	Ref<_PictureTileData> tilePicture = _getPictureTile(location);
	Rectangle rectanglePicture;
	if (tilePicture.isNull()) {
		_requestPictureTile(location);
		if (parent) {
			tilePicture = parent->picture;
			sl_real hw = parent->rectanglePicture.getWidth() * 0.5f;
			sl_real hh = parent->rectanglePicture.getHeight() * 0.5f;
			rectanglePicture.left = parent->rectanglePicture.left + hw * ix;
			rectanglePicture.top = parent->rectanglePicture.top + hh * (1 - iy);
			rectanglePicture.right = rectanglePicture.left + hw;
			rectanglePicture.bottom = rectanglePicture.top + hh;
		} else {
			return ret;
		}
	} else {
		rectanglePicture = Rectangle(0, 0, 1, 1);
	}

	Ref<_DEMTileData> tileDEM = _getDEMTile(location);
	Rectangle rectangleDEM;
	if (tileDEM.isNull()) {
		_requestDEMTile(location);
		if (parent) {
			tileDEM = parent->dem;
			sl_real hw = parent->rectangleDEM.getWidth() * 0.5f;
			sl_real hh = parent->rectangleDEM.getHeight() * 0.5f;
			rectangleDEM.left = parent->rectangleDEM.left + hw * ix;
			rectangleDEM.top = parent->rectangleDEM.top + hh * (1 - iy);
			rectangleDEM.right = rectangleDEM.left + hw;
			rectangleDEM.bottom = rectangleDEM.top + hh;
		} else {
			rectangleDEM = Rectangle(0, 0, 1, 1);
		}
	} else {
		rectangleDEM = Rectangle(0, 0, 1, 1);
	}

	SLIB_NEW_REF(_RenderTile, tile);
	if (tile) {
		tile->picture = tilePicture;
		tile->rectanglePicture = rectanglePicture;
		tile->dem = tileDEM;
		tile->rectangleDEM = rectangleDEM;

		tile->location = location;
		tile->region.bottomLeft = getLatLonFromTileLocation(location);
		tile->region.topRight = getLatLonFromTileLocation(MapTileLocationi(location.level, location.y + 1, location.x + 1));

		LatLon loc;
		const GeoRectangle& rectangle = tile->region;
		// bottom-left
		loc.latitude = rectangle.bottomLeft.latitude;
		loc.longitude = rectangle.bottomLeft.longitude;
		tile->positions[0] = MapEarth::getCartesianPosition(loc);
		// bottom-right
		loc.latitude = rectangle.bottomLeft.latitude;
		loc.longitude = rectangle.topRight.longitude;
		tile->positions[1] = MapEarth::getCartesianPosition(loc);
		// top-left
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.bottomLeft.longitude;
		tile->positions[2] = MapEarth::getCartesianPosition(loc);
		// top-right
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.topRight.longitude;
		tile->positions[3] = MapEarth::getCartesianPosition(loc);
		// center
		loc.latitude = (rectangle.bottomLeft.latitude + rectangle.topRight.latitude) / 2;
		loc.longitude = (rectangle.topRight.longitude + rectangle.bottomLeft.longitude) / 2;
		tile->positionCenter = MapEarth::getCartesianPosition(loc);
	}
	return tile;
}

sl_bool MapEarthRenderer::_checkTileVisible(_RenderTile* tile)
{
	// check normal
	{
		Vector3 normal = m_environment->transformView.transformDirection(tile->positionCenter);
		normal.normalize();
		if (tile->location.level == 0) {
			if (normal.z > 0.2f) {
				return sl_false;
			}
		} else if (tile->location.level == 1) {
			if (normal.z > 0.1f) {
				return sl_false;
			}
		} else if (tile->location.level == 2) {
			if (normal.z > 0.05f) {
				return sl_false;
			}
		} else {
			if (normal.z > 0.0f) {
				return sl_false;
			}
		}
	}
	// check frustum
	if (m_environment->viewFrustum.containsFacets(tile->positions, 4)) {
		return sl_true;
	}
	return sl_false;
}

sl_bool MapEarthRenderer::_checkTileExpandable(_RenderTile* tile)
{
	MapEnvironment* environment = m_environment;
	
	// check size
	Vector3 ptBL = environment->transformView.transformPosition(tile->positions[0]);
	Vector3 ptBR = environment->transformView.transformPosition(tile->positions[1]);
	Vector3 ptTL = environment->transformView.transformPosition(tile->positions[2]);
	sl_uint32 nBehind = 0;
	if (Math::isLessThanEpsilon(ptBL.z)) {
		nBehind++;
	}
	if (Math::isLessThanEpsilon(ptBR.z)) {
		nBehind++;
	}
	if (Math::isLessThanEpsilon(ptTL.z)) {
		nBehind++;
	}
	if (nBehind == 3) {
		return sl_false;
	}
	if (nBehind != 0) {
		return sl_true;
	} else {
		Triangle2 t;
		t.point1.x = ptBL.x / ptBL.z;
		t.point1.y = ptBL.y / ptBL.z;
		t.point2.x = ptBR.x / ptBR.z;
		t.point2.y = ptBR.y / ptBR.z;
		t.point3.x = ptTL.x / ptTL.z;
		t.point3.y = ptTL.y / ptTL.z;
		sl_real size = Math::abs(t.getSize());
		if (size * 2 > m_sizeTileMinimum * 1.5f) {
			return sl_true;
		}
	}
	return sl_false;
}

SLIB_MAP_NAMESPACE_END

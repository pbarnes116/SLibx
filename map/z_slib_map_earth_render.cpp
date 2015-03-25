
#include "earth.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::render(RenderEngine* engine, MapEnvironment* environment)
{
	if (!m_flagInitialized) {
		return;
	}

	_prepareRendering(engine, environment);

	_renderTiles(engine);

	if (isShowGISLine()) {
		engine->setDepthTest(sl_false);
		_renderGISLines(engine);
		engine->setDepthTest(sl_true);
	}
	if (isShowBuilding()) {
		engine->clearDepth();
		_renderBuildings(engine);
	}
	if (isShowGISPoi()) {
		engine->setBlending(sl_true);
		engine->setDepthTest(sl_false);
		_renderGISPois(engine);
		engine->setBlending(sl_false);
		engine->setDepthTest(sl_true);
	}
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

void MapEarthRenderer::_renderBuilding(RenderEngine* engine, _RenderBuilding* building)
{
	m_programBuilding->setAmbientColor(Color(120, 120, 120));
	m_programBuilding->setDiffuseColor(Color(120, 120, 120));
	m_programBuilding->setModelMatrix(VW_Building::getModelTransformMatrixForMesh(building->info->bound.center()));
	ListLocker<VW_Building_Mesh> meshes(building->object->meshes);
	for (sl_size i = 0; i < meshes.count(); i++) {
		VW_Building_Mesh& mesh = meshes[i];
		Ref<_BuildingTexture> bt = _getBuildingTexture(building->info->key, i);
		if (bt.isNotNull()) {
			m_programBuilding->setTexture(bt->texture);
		} else {
			m_programBuilding->setTexture(mesh.textureThumbnail);
		}
		engine->draw(m_programBuilding, mesh.countElements, mesh.vb, mesh.ib);
	}
}

void MapEarthRenderer::_renderGISLine(RenderEngine* engine, _GISLineTile* tile)
{
	ListLocker<_GISShape> list(tile->shapes);
	for (sl_size i = 0; i < list.count(); i++) {
		_GISShape& s = list[i];
		m_programLine->setDiffuseColor(s.color);
		engine->draw(m_programLine, s.nElements, s.vb, Primitive::typeLines);
	}
}

void MapEarthRenderer::_renderGISPoi(RenderEngine* engine, _GISPoiTile* tile)
{
	ListLocker<_GISPoi> list(tile->pois);
	for (sl_size i = 0; i < list.count(); i++) {
		_GISPoi& s = list[i];
		float altitude = _getAltitudeFromRenderingDEM(s.location);
		altitude = 0;
		Vector3 pos = MapEarth::getCartesianPosition(GeoLocation(s.location, altitude));
		if (m_environment->viewFrustum.containsPoint(pos)) {
			Vector3 posScreen = Transform3::projectToScreenPoint(m_environment->transformViewProjection, pos);
			float x = (posScreen.x + 1.0f) * m_environment->viewportWidth / 2.0f;
			float y = (1.0f - posScreen.y) * m_environment->viewportHeight / 2.0f;
			float w = (float)(s.texture->getWidth());
			float h = (float)(s.texture->getHeight());
			engine->drawTexture2D(x - w / 2, y - h / 2, w, h, s.texture);
		}
	}
}

void MapEarthRenderer::_renderMarker(RenderEngine* engine, MapMarker* marker)
{
	
}

void MapEarthRenderer::_renderPolygon(RenderEngine* engine, MapPolygon* polygon)
{
	
}

void MapEarthRenderer::_prepareRendering(RenderEngine* engine, MapEnvironment* environment)
{
	m_environment = environment;

	m_programSurfaceTile->setViewMatrix(environment->transformView);
	m_programSurfaceTile->setProjectionMatrix(environment->transformProjection);

	m_programBuilding->setViewMatrix(environment->transformView);
	m_programBuilding->setProjectionMatrix(environment->transformProjection);

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
	for (sl_uint32 level = 0; level <= _getMaxLevel(); level++) {
		List< Ref<_RenderTile> > listExpand;
		// find expand
		{
			sl_bool flagExpand = sl_true;
			if (level == _getMaxLevel()) {
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
	
	List<MapTileLocationi> listRenderedTiles;
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
			listRenderedTiles.add(tile->location);
		}
	}
	m_nMaxRenderTileLevel = nLevelMax;
	m_listRenderedTiles = listRenderedTiles;
}

Ref<MapEarthRenderer::_RenderTile> MapEarthRenderer::_getRenderTile(const MapTileLocationi& location, _RenderTile* parent, sl_uint32 ix, sl_uint32 iy)
{
	Ref<_RenderTile> ret;
	Ref<_PictureTileData> tilePicture = _getPictureTile(location);
	Rectangle rectanglePicture;
	if (tilePicture.isNull()) {
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

		GeoLocation loc;
		const GeoRectangle& rectangle = tile->region;
		// bottom-left
		loc.latitude = rectangle.bottomLeft.latitude;
		loc.longitude = rectangle.bottomLeft.longitude;
		loc.altitude = 0;
		tile->positions[0] = MapEarth::getCartesianPosition(loc);
		loc.altitude = _getAltitudeFromRenderingDEM(loc.getLatLon());
		tile->positionsWithDEM[0] = MapEarth::getCartesianPosition(loc);
		// bottom-right
		loc.latitude = rectangle.bottomLeft.latitude;
		loc.longitude = rectangle.topRight.longitude;
		loc.altitude = 0;
		tile->positions[1] = MapEarth::getCartesianPosition(loc);
		loc.altitude = _getAltitudeFromRenderingDEM(loc.getLatLon());
		tile->positionsWithDEM[1] = MapEarth::getCartesianPosition(loc);
		// top-left
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.bottomLeft.longitude;
		loc.altitude = 0;
		tile->positions[2] = MapEarth::getCartesianPosition(loc);
		loc.altitude = _getAltitudeFromRenderingDEM(loc.getLatLon());
		tile->positionsWithDEM[2] = MapEarth::getCartesianPosition(loc);
		// top-right
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.topRight.longitude;
		loc.altitude = 0;
		tile->positions[3] = MapEarth::getCartesianPosition(loc);
		loc.altitude = _getAltitudeFromRenderingDEM(loc.getLatLon());
		tile->positionsWithDEM[3] = MapEarth::getCartesianPosition(loc);
		// center
		loc.latitude = (rectangle.bottomLeft.latitude + rectangle.topRight.latitude) / 2;
		loc.longitude = (rectangle.topRight.longitude + rectangle.bottomLeft.longitude) / 2;
		loc.altitude = 0;
		tile->positionCenter = MapEarth::getCartesianPosition(loc);
	}
	return tile;
}

sl_bool MapEarthRenderer::_checkTileVisible(_RenderTile* tile)
{
	// check distance
	{
		sl_real e2 = m_environment->positionEye.getLength2p();
		sl_real r2 = (sl_real)(MapEarth::getRadius());
		r2 *= r2;
		if (e2 < r2 * 4) {
			sl_real p2 = (m_environment->positionEye - tile->positionCenter).getLength2p();
			if (p2 > e2 + r2) {
				return sl_false;
			}
		}
	}
	// check normal
	{
		Vector3 normal = m_environment->transformView.transformDirection(tile->positionCenter);
		normal.normalize();
		if (normal.z > 0.2f) {
			return sl_false;
		}
	}
	// check frustum
	if (m_environment->viewFrustum.containsFacets(tile->positions, 4)
		|| m_environment->viewFrustum.containsFacets(tile->positionsWithDEM, 4)) {
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


void MapEarthRenderer::_renderBuildings(RenderEngine* engine)
{
	struct _MapEarthRenderer_Building
	{
		Ref<MapEarthRenderer::_RenderBuilding> object;
		sl_real distance;
	};
	struct _MapEarthRenderer_SortBuilding
	{
	public:
		SLIB_INLINE static sl_real key(_MapEarthRenderer_Building& b)
		{
			return b.distance;
		}
	};
	Vector3 eye = m_environment->positionEye;
	List<_MapEarthRenderer_Building> buildings;
	{
		ListLocker< Ref<_RenderBuilding> > list(m_mapRenderBuildings.values());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<_RenderBuilding> building = list[i];
			if (building.isNotNull()) {
				if (m_environment->viewFrustum.containsBox(building->info->bound)) {
					_MapEarthRenderer_Building b;
					if (building->info->flagBridge) {
						b.distance = 0;
					} else {
						b.distance = (eye - (Vector3)(building->info->bound.center())).getLength2p();
					}
					b.object = building;
					buildings.add(b);
				}
			}
		}
	}
	buildings = buildings.sort<_MapEarthRenderer_SortBuilding, sl_real>(sl_true);
	buildings.setCount(Math::min(getMaxBuildingsCount(), (sl_uint32)(buildings.count())));
	{
		sl_bool flagRequestTexture = sl_false;
		sl_uint32 maxt = _getMaxDetailedBuildingsCount();
		sl_uint32 it = 0;
		ListLocker<_MapEarthRenderer_Building> list(buildings);
		for (sl_size i = 0; i < list.count(); i++) {
			if (it < maxt && !flagRequestTexture) {
				sl_size kn = list[i].object->object->meshes.count();
				for (sl_size k = 0; k < kn; k++) {
					if (it < maxt) {
						if (!flagRequestTexture) {
							String key = list[i].object->info->key;
							if (_getBuildingTexture(key, k).isNull()) {
								_requestBuildingTexture(key, k);
								flagRequestTexture = sl_true;
								break;
							}
						}
						it++;
					}
				}
			}
			_renderBuilding(engine, list[i].object);
		}
	}
}

void MapEarthRenderer::_renderGISLines(RenderEngine* engine)
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<_GISLineTile> tile = _getGISLineTile(list[i]);
		if (tile.isNotNull()) {
			_renderGISLine(engine, tile);
		}
	}
}

void MapEarthRenderer::_renderGISPois(RenderEngine* engine)
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<_GISPoiTile> tile = _getGISPoiTile(list[i]);
		if (tile.isNotNull()) {
			_renderGISPoi(engine, tile);
		}
	}
}

void MapEarthRenderer::_renderMarkers(RenderEngine* engine)
{
	ListLocker< Ref<MapMarker> > list(markers.values());
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<MapMarker> marker = list[i];
		if (marker.isNotNull()) {
			_renderMarker(engine, marker);
		}
	}
}

void MapEarthRenderer::_renderPolygons(RenderEngine* engine)
{
	ListLocker< Ref<MapPolygon> > list(polygons.values());
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<MapPolygon> polygon = list[i];
		if (polygon.isNotNull()) {
			_renderPolygon(engine, polygon);
		}
	}
}

SLIB_MAP_NAMESPACE_END

#include "map_earth_renderer.h"

#include "../../../inc/slib/render/opengl.h"

SLIB_MAP_NAMESPACE_BEGIN

#define MAX_LEVEL 25
#define MAX_RENDER_POIS 1000

void MapEarthRenderer::render(RenderEngine* engine)
{
	if (!m_flagInitialized) {
		return;
	}

	_prepareRendering(engine);

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

	{
		engine->setBlending(sl_true);
		engine->setDepthTest(sl_false);
		_renderPolygons(engine);
		_renderMarkers(engine);
		_renderIcons(engine);
		engine->setBlending(sl_false);
		engine->setDepthTest(sl_true);
	}
}

void MapEarthRenderer::_renderTile(RenderEngine* engine, _Tile* tile)
{
	m_programSurfaceTile->setTexture(tile->picture->texture);

	sl_bool flagUseCache = sl_false;
	Ref<MapRenderTile> cache = m_tilesRender->getTile(tile->location);
	if (cache.isNotNull()) {
		if (cache->picture == tile->picture && cache->dem == tile->dem) {
			flagUseCache = sl_true;
		}
	}

	m_programSurfaceTile->setViewMatrix(Transform3lf::getTranslationMatrix(tile->positionCenter) * m_transformView);
	if (flagUseCache) {
		engine->draw(m_programSurfaceTile, &(cache->primitive));
	} else {
		Primitive primitive;
		if (tile->dem.isNotNull()) {
			tile->dem->dem.makeMeshFromSphericalGlobe(MapEarth::getGlobe(), tile->positionCenter, primitive, 17, tile->region, tile->rectangleDEM, tile->rectanglePicture);
		} else {
			DEM dem;
			dem.makeMeshFromSphericalGlobe(MapEarth::getGlobe(), tile->positionCenter, primitive, 17, tile->region, Rectangle(0, 0, 1, 1), tile->rectanglePicture);
		}

		engine->draw(m_programSurfaceTile, &(primitive));

		cache = new MapRenderTile;
		cache->location = tile->location;
		cache->picture = tile->picture;
		cache->dem = tile->dem;
		cache->primitive = primitive;
		m_tilesRender->saveTile(tile->location, cache);
	}
}

void MapEarthRenderer::_renderBuilding(RenderEngine* engine, MapBuilding* building)
{
	m_programBuilding->setAmbientColor(Color(120, 120, 120));
	m_programBuilding->setDiffuseColor(Color(120, 120, 120));
	m_programBuilding->setViewMatrix(VW_Building::getModelTransformMatrixForMesh(building->info->bound.center()) * m_transformView);
	ListLocker<VW_Building_Mesh> meshes(building->object->meshes);
	for (sl_size i = 0; i < meshes.count(); i++) {
		VW_Building_Mesh& mesh = meshes[i];
		Ref<Texture> bt = m_tilesBuilding->getDetailedTexture(building->info->key, (sl_uint32)i);
		if (bt.isNotNull()) {
			m_programBuilding->setTexture(bt);
		} else {
			m_programBuilding->setTexture(mesh.textureThumbnail);
		}
		engine->draw(m_programBuilding, mesh.countElements, mesh.vb, mesh.ib);
	}
}

void MapEarthRenderer::_renderGISLine(RenderEngine* engine, MapGISLineTile* tile)
{
	Ref<MapDEMTile> dem = m_tilesDEM->getTileHierarchically(tile->location, sl_null);
	ListLocker< Ref<MapGISShape> > list(tile->shapes);
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<MapGISShape>& s = list[i];
		ListLocker<MapGISLineData> lines(s->lines);
		sl_uint32 n = (sl_uint32)(lines.count());
		if (n > 0) {
			m_programLine->setDiffuseColor(s->color);
			engine->setLineWidth(s->width);
			Ref<VertexBuffer> vb;
			if (s->vb.isNotNull() && dem == s->demTileRef) {
				vb = s->vb;
			} else {
				SLIB_SCOPED_ARRAY(Vector3, pos, n * 2);
				for (sl_uint32 i = 0; i < n; i++) {
					if (dem.isNull()) {
						pos[i * 2] = MapEarth::getCartesianPosition(lines[i].start);
						pos[i * 2 + 1] = MapEarth::getCartesianPosition(lines[i].end);
					} else {
						MapTileLocation location;
						sl_real altitude;
						location = getTileLocationFromLatLon(dem->location.level, lines[i].start);
						altitude = MapDEMTileManager::getAltitudeFromDEM(
							(sl_real)(location.x - dem->location.x)
							, 1 - (sl_real)(location.y - dem->location.y)
							, dem.get());
						pos[i * 2] = MapEarth::getCartesianPosition(GeoLocation(lines[i].start, altitude));
						location = getTileLocationFromLatLon(dem->location.level, lines[i].end);
						altitude = MapDEMTileManager::getAltitudeFromDEM(
							(sl_real)(location.x - dem->location.x)
							, 1 - (sl_real)(location.y - dem->location.y)
							, dem.get());
						pos[i * 2 + 1] = MapEarth::getCartesianPosition(GeoLocation(lines[i].end, altitude));
					}
				}
				s->demTileRef = dem;
				s->vb = VertexBuffer::create(pos, n * 2 * sizeof(Vector3));
				vb = s->vb;
			}
			if (vb.isNotNull()) {
				engine->draw(m_programLine, n * 2, vb, Primitive::typeLines);
			}
		}
	}
}

void MapEarthRenderer::_renderGISPoi(RenderEngine* engine, MapGISPoi* poi, const Ref<FreeType>& font)
{
	sl_real screenRatio = (sl_real)m_viewportWidth / 1280;

	MapGISPoi& s = *poi;
	if (s.texture.isNull()) {
		String text = s.text;
		if (text.length() > 50) {
			text = text.substring(0, 50);
		}
		font->setSize((sl_uint32)(s.fontSize * screenRatio * 1.5f));
		Sizei size = font->getStringExtent(text);
		Ref<Image> image = Image::create(size.width + 6, size.height + 8);
		if (image.isNotNull()) {
			font->strokeString(image, 3, size.height + 2, text, Color::Black, 2);
			font->drawString(image, 3, size.height + 2, text, s.clr);
			Ref<Texture> texture = Texture::create(image);
			if (texture.isNotNull()) {
				s.texture = texture;
			}
		}
	}
	if (s.texture.isNotNull()) {
		MapTileLocation location = getTileLocationFromLatLon(s.level, s.location);
		float altitude = m_tilesDEM->getAltitudeHierarchically(location);
		Vector3 pos = MapEarth::getCartesianPosition(GeoLocation(s.location, altitude));
		if (m_viewFrustum.containsPoint(pos)) {
			Vector2 ps = convertPointToScreen(pos);
			float w = (float)(s.texture->getWidth());
			float h = (float)(s.texture->getHeight());
			engine->drawTexture2D(
				engine->screenToViewport(ps.x - w / 2, ps.y - h / 2, w, h)
				, s.texture);
		}
	}
}

void MapEarthRenderer::_renderMarker(RenderEngine* engine, MapMarker* marker)
{
	sl_real screenRatio = (sl_real)m_viewportWidth / 1280;
	Vector3 pos = MapEarth::getCartesianPosition(getLocationFromLatLon(marker->location.getLatLon()));
	if (_checkPointVisible(pos)) {
		Vector2 ps = convertPointToScreen(pos);
		if (marker->iconTexture.isNotNull()) {
			Rectangle rectangle = Rectangle(Point(ps.x - marker->iconSize.width / 2, ps.y - marker->iconSize.height), marker->iconSize);
			engine->drawTexture2D(
				engine->screenToViewport(rectangle)
				, marker->iconTexture
				, marker->iconTextureRectangle);
		}
		if (marker->text.isNotEmpty() && marker->textFont.isNotNull()) {
			if (marker->_textureText.isNull()) {
				String text = marker->text;
				if (text.length() > 50) {
					text = text.substring(0, 50);
				}
				marker->textFont->setSize((sl_uint32)(marker->textFontSize * screenRatio));
				Sizei size = marker->textFont->getStringExtent(text);
				Ref<Image> image = Image::create(size.width + 8, size.height + 8);
				if (image.isNotNull()) {
					marker->textFont->strokeString(image, 3, size.height + 2, text, Color::Black, 2);
					marker->textFont->drawString(image, 3, size.height + 2, text, marker->textColor);
					Ref<Texture> texture = Texture::create(image);
					if (texture.isNotNull()) {
						marker->_textureText = texture;
					}
				}
			}
			if (marker->_textureText.isNotNull()) {
				Rectangle rectangle = Rectangle(
					Point(ps.x - marker->_textureText->getWidth() / 2, ps.y)
					,Size((float)(marker->_textureText->getWidth()), (float)(marker->_textureText->getHeight()))
					);
				engine->drawTexture2D(
					engine->screenToViewport(rectangle)
					, marker->_textureText);
			}
		}
	}	
}

void MapEarthRenderer::_renderIcon(RenderEngine* engine, MapIcon* icon)
{
	Vector3 pos = MapEarth::getCartesianPosition(getLocationFromLatLon(icon->location.getLatLon()));
	if (_checkPointVisible(pos)) {
		Vector2 ps = convertPointToScreen(pos);
		if (icon->iconTexture.isNotNull()) {
			Matrix3 transform = Transform2::getTranslationMatrix(-0.5f, -0.5f)
				* Transform2::getScalingMatrix(icon->iconSize.x, icon->iconSize.y)
				* Transform2::getRotationMatrix(Math::getRadianFromDegrees(icon->rotationAngle))
				* Transform2::getTranslationMatrix(ps.x, ps.y)
				* Transform2::getScalingMatrix(2.0f / m_viewportWidth, -2.0f / m_viewportHeight)
				* Transform2::getTranslationMatrix(-1, 1);
			engine->drawTexture2D(transform, icon->iconTexture, icon->iconTextureRectangle);
		}
	}
}

void MapEarthRenderer::_renderPolygon(RenderEngine* engine, MapPolygon* polygon)
{
	List<GeoLocation> points = polygon->points;
	sl_size n = points.count();
	if (n <= 1) {
		return;
	}
	SLIB_SCOPED_ARRAY(Vector3, pos, n);
	sl_bool flagVisible = sl_false;
	for (sl_size i = 0; i < n; i++) {
		pos[i] = MapEarth::getCartesianPosition(getLocationFromLatLon(points[i].getLatLon()));
		Vector3lf normal = m_transformView.transformDirection(pos[i]);
		normal.normalize();
		if (normal.z <= 0) {
			flagVisible = sl_true;
		}
	}
	if (!flagVisible) {
		return;
	}
	engine->setLineWidth(polygon->width);
	m_programLine->setDiffuseColor(polygon->color);
	Ref<VertexBuffer> vb = VertexBuffer::create(pos, n*sizeof(Vector3));
	engine->draw(m_programLine, (sl_uint32)n, vb, Primitive::typeLineStrip);
	engine->setLineWidth(1);
}

void MapEarthRenderer::_prepareRendering(RenderEngine* engine)
{
	// update projection matrix
	{
		m_viewportWidth = engine->getViewportWidth();
		m_viewportHeight = engine->getViewportHeight();
		double dist = m_camera->getEyeLocation().altitude + 0.1;
		double zNear, zFar;
		if (dist < 5000) {
			zNear = dist / 50;
			zFar = dist * 20 + 1000;
		} else {
			zNear = dist / 5;
			zFar = dist + MapEarth::getRadius() * 2;
		}
		m_transformProjection = Transform3lf::getPerspectiveProjectionFovYMatrix(SLIB_PI / 3, (double)m_viewportWidth / m_viewportHeight, zNear, zFar);
	}
	m_transformView = m_camera->getViewMatrix();
	m_transformViewInverse = m_transformView.inverse();
	m_transformViewProjection = m_transformView * m_transformProjection;
	m_viewFrustum = ViewFrustumlf::fromMVP(m_transformViewProjection);
	m_positionEye = m_camera->getEyeCartesianPosition();

	m_programSurfaceTile->setProjectionMatrix(m_transformProjection);

	m_programBuilding->setProjectionMatrix(m_transformProjection);

	m_programLine->setViewMatrix(m_transformView);
	m_programLine->setProjectionMatrix(m_transformProjection);

	sl_real _w = (sl_real)m_viewportWidth;
	if (_w > 1280) {
		_w = 1280;
	}
	m_sizeTileMinimum = (65536.0f / _w / _w) * 4;

	m_flagStartedRendering = sl_true;
}

void MapEarthRenderer::_renderTiles(RenderEngine* engine)
{
	List< Ref<_Tile> > listRender;
	List< Ref<_Tile> > listCurrent;
	for (sl_uint32 y = 0; y < 5; y++) {
		for (sl_uint32 x = 0; x < 10; x++) {
			Ref<_Tile> tile = _getTile(MapTileLocationi(0, y, x));
			if (tile.isNotNull()) {
				listCurrent.add(tile);
			}
		}
	}
	for (sl_uint32 level = 0; level <= getMaxLevel(); level++) {
		List< Ref<_Tile> > listExpand;
		// find expand
		{
			sl_bool flagExpand = sl_true;
			if (level == getMaxLevel()) {
				flagExpand = sl_false;
			}
			ListLocker< Ref<_Tile> > current(listCurrent);
			for (sl_size i = 0; i < current.count(); i++) {
				Ref<_Tile> tile = current[i];
				if (_checkTileVisible(tile.get())) {
					if (flagExpand && _checkTileExpandable(tile.get())) {
						listExpand.add(tile);
					} else {
						listRender.add(tile);
					}
				}
			}
		}
		// expand
		{
			listCurrent = List< Ref<_Tile> >::null();
			ListLocker< Ref<_Tile> > expand(listExpand);
			for (sl_size i = 0; i < expand.count(); i++) {
				Ref<_Tile> tile = expand[i];
				for (sl_uint32 iy = 0; iy < 2; iy++) {
					for (sl_uint32 ix = 0; ix < 2; ix++) {
						sl_uint32 ty = (tile->location.y << 1) + iy;
						sl_uint32 tx = (tile->location.x << 1) + ix;
						Ref<_Tile> tileNew = _getTile(MapTileLocationi(level + 1, ty, tx));
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
		ListLocker< Ref<_Tile> > list(listRender);
		for (sl_size i = 0; i < list.count(); i++) {
			const Ref<_Tile>& tile = list[i];
			if (tile->location.level > nLevelMax) {
				nLevelMax = tile->location.level;
			}
			_renderTile(engine, tile.get());
			listRenderedTiles.add(tile->location);
		}
	}
	m_nMaxRenderTileLevel = nLevelMax;
	m_listRenderedTiles = listRenderedTiles;
}

Ref<MapEarthRenderer::_Tile> MapEarthRenderer::_getTile(const MapTileLocationi& location)
{
	Ref<_Tile> ret;
	Rectangle rectanglePicture;
	Ref<MapPictureTile> tilePicture = m_tilesPicture->getTileHierarchically(location, &rectanglePicture);
	if (tilePicture.isNull()) {
		return ret;
	}

	Rectangle rectangleDEM;
	Ref<MapDEMTile> tileDEM = m_tilesDEM->getTileHierarchically(location, &rectangleDEM);

	SLIB_NEW_REF(_Tile, tile);
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
		loc.altitude = MapDEMTileManager::getAltitudeFromDEM(rectangleDEM.left, rectangleDEM.bottom, tileDEM.get());
		tile->positionsWithDEM[0] = MapEarth::getCartesianPosition(loc);
		// bottom-right
		loc.latitude = rectangle.bottomLeft.latitude;
		loc.longitude = rectangle.topRight.longitude;
		loc.altitude = 0;
		tile->positions[1] = MapEarth::getCartesianPosition(loc);
		loc.altitude = MapDEMTileManager::getAltitudeFromDEM(rectangleDEM.right, rectangleDEM.bottom, tileDEM.get());
		tile->positionsWithDEM[1] = MapEarth::getCartesianPosition(loc);
		// top-left
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.bottomLeft.longitude;
		loc.altitude = 0;
		tile->positions[2] = MapEarth::getCartesianPosition(loc);
		loc.altitude = MapDEMTileManager::getAltitudeFromDEM(rectangleDEM.left, rectangleDEM.top, tileDEM.get());
		tile->positionsWithDEM[2] = MapEarth::getCartesianPosition(loc);
		// top-right
		loc.latitude = rectangle.topRight.latitude;
		loc.longitude = rectangle.topRight.longitude;
		loc.altitude = 0;
		tile->positions[3] = MapEarth::getCartesianPosition(loc);
		loc.altitude = MapDEMTileManager::getAltitudeFromDEM(rectangleDEM.right, rectangleDEM.top, tileDEM.get());
		tile->positionsWithDEM[3] = MapEarth::getCartesianPosition(loc);
		// center
		loc.latitude = (rectangle.bottomLeft.latitude + rectangle.topRight.latitude) / 2;
		loc.longitude = (rectangle.topRight.longitude + rectangle.bottomLeft.longitude) / 2;
		loc.altitude = 0;
		tile->positionCenter = MapEarth::getCartesianPosition(loc);
	}
	return tile;
}

sl_bool MapEarthRenderer::_checkPointVisible(const Vector3& point)
{
	// check distance
	double e2 = m_positionEye.getLength2p();
	double r2 = MapEarth::getRadius();
	r2 *= r2;
	double p2 = (m_positionEye - point).getLength2p();
	if (p2 > e2 - r2) {
		return sl_false;
	}
	// check frustum
	if (!m_viewFrustum.containsPoint(point)) {
		return sl_false;
	}
	return sl_true;
}

sl_bool MapEarthRenderer::_checkTileVisible(_Tile* tile)
{
	// check distance
	{
		double e2 = m_positionEye.getLength2p();
		double r2 = MapEarth::getRadius();
		r2 *= r2;
		if (e2 < r2 * 4) {
			double p2 = (m_positionEye - tile->positionCenter).getLength2p();
			if (p2 > e2 + r2) {
				return sl_false;
			}
		}
	}
	// check normal
	{
		sl_bool flagVisible = sl_false;
		for (sl_uint32 i = 0; i < 4; i++) {
			Vector3lf normal = m_transformView.transformDirection(tile->positions[i]);
			normal.normalize();
			if (normal.z <= 0) {
				flagVisible = sl_true;
				break;
			}
		}
		if (!flagVisible) {
			return sl_false;
		}
	}
	// check frustum
	if (m_viewFrustum.containsFacets(tile->positions, 4)
		|| m_viewFrustum.containsFacets(tile->positionsWithDEM, 4)) {
		return sl_true;
	}
	return sl_false;
}

sl_bool MapEarthRenderer::_checkTileExpandable(_Tile* tile)
{
	// check size
	Vector3 ptBL = m_transformView.transformPosition(tile->positions[0]);
	Vector3 ptBR = m_transformView.transformPosition(tile->positions[1]);
	Vector3 ptTL = m_transformView.transformPosition(tile->positions[2]);
	Vector3 ptTR = m_transformView.transformPosition(tile->positions[3]);
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
	if (Math::isLessThanEpsilon(ptTR.z)) {
		nBehind++;
	}
	if (nBehind == 4) {
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
		t.point1.x = ptBL.x / ptTR.z;
		t.point1.y = ptBL.y / ptTR.z;
		size += Math::abs(t.getSize());
		if (size > m_sizeTileMinimum * 1.5f) {
			return sl_true;
		}
	}
	return sl_false;
}

void MapEarthRenderer::_renderBuildings(RenderEngine* engine)
{
	struct _Building
	{
		Ref<MapBuilding> object;
		sl_real distance;
	};
	Vector3 eye = m_positionEye;
	List<_Building> buildings;
	{
		ListLocker< Ref<MapBuilding> > list(m_tilesBuilding->getBuildings());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<MapBuilding> building = list[i];
			if (building.isNotNull()) {
				if (m_viewFrustum.containsBox(building->info->bound)) {
					_Building b;
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
	class _Compare
	{
	public:
		SLIB_INLINE static int compare(const _Building& a, const _Building& b)
		{
			return Compare<sl_real>::compare(a.distance, b.distance);
		}
	};
	buildings.sortBy<_Compare>();
	buildings.setCount(Math::min(m_tilesBuilding->getMaxBuildingsCount(), (sl_uint32)(buildings.count())));
	{
		sl_bool flagRequestTexture = sl_false;
		sl_uint32 maxt = m_tilesBuilding->getMaxDetailedBuildingsCount();
		sl_uint32 it = 0;
		ListLocker<_Building> list(buildings);
		for (sl_size i = 0; i < list.count(); i++) {
			if (it < maxt && !flagRequestTexture) {
				sl_size kn = list[i].object->object->meshes.count();
				for (sl_size k = 0; k < kn; k++) {
					if (it < maxt) {
						if (!flagRequestTexture) {
							String key = list[i].object->info->key;
							if (m_tilesBuilding->getDetailedTexture(key, (sl_uint32)k).isNull()) {
								m_tilesBuilding->requestDetailedTexture(key, (sl_uint32)k);
								flagRequestTexture = sl_true;
								break;
							}
						}
						it++;
					}
				}
			}
			_renderBuilding(engine, list[i].object.get());
		}
	}
}

void MapEarthRenderer::_renderGISLines(RenderEngine* engine)
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		Ref<MapGISLineTile> tile = m_tilesGISLine->getTile(list[i]);
		if (tile.isNotNull()) {
			_renderGISLine(engine, tile.get());
		}
	}
}

void MapEarthRenderer::_renderGISPois(RenderEngine* engine)
{
	Ref<FreeType> font = getFontForPOI();
	if (font.isNull()) {
		return;
	}
	List< Ref<MapGISPoi> > pois[MAX_LEVEL];
	{
		ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<MapGISPoiTile> tile = m_tilesGISPoi->getTile(list[i]);
			if (tile.isNotNull()) {
				ListLocker< Ref<MapGISPoi> > list(tile->pois);
				for (sl_size k = 0; k < list.count(); k++) {
					Ref<MapGISPoi> poi = list[k];
					poi->init(m_poiInfo);
					if (poi->type != MapGISPoiData::typeNone && poi->text.isNotEmpty() && poi->showMinLevel <= (sl_int32)(poi->level)) {
						if (poi->showMinLevel < MAX_LEVEL) {
							pois[poi->showMinLevel].add(poi);
						}
					}
				}
			}
		}
	}
	int n = 0;
	for (sl_int32 m = MAX_LEVEL - 1; m >= 0; m--) {
		ListLocker< Ref<MapGISPoi> > list(pois[m]);
		for (sl_size i = 0; i < list.count(); i++) {
			_renderGISPoi(engine, list[i].get(), font);
			n++;
			if (n > MAX_RENDER_POIS) {
				return;
			}
		}
	}
}

void MapEarthRenderer::_renderMarkers(RenderEngine* engine)
{
	{
		ListLocker< Ref<MapMarker> > list(markers.values());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<MapMarker> marker = list[i];
			if (marker.isNotNull() && marker->flagVisible) {
				_renderMarker(engine, marker.get());
			}
		}
	}
}

void MapEarthRenderer::_renderIcons(RenderEngine* engine)
{
	{
		ListLocker< Ref<MapIcon> > list(icons.values());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<MapIcon> icon = list[i];
			if (icon.isNotNull() && icon->flagVisible) {
				_renderIcon(engine, icon.get());
			}
		}
	}
}

void MapEarthRenderer::_renderPolygons(RenderEngine* engine)
{
	{
		ListLocker< Ref<MapPolygon> > list(polygons.values());
		for (sl_size i = 0; i < list.count(); i++) {
			Ref<MapPolygon> polygon = list[i];
			if (polygon.isNotNull()) {
				_renderPolygon(engine, polygon.get());
			}
		}
	}
}


class _MapEarth_RenderProgram_SurfaceTile : public RenderProgram3D
{
public:
	_MapEarth_RenderProgram_SurfaceTile() {}
	~_MapEarth_RenderProgram_SurfaceTile() {}

	class MapInfo_GL : public Info_GL
	{
	public:
		sl_int32 attrAltitude;	// float
	};

public:

	Ref<RenderProgramInfo> create(RenderEngine* engine)
	{
		Ref<RenderProgramInfo> ret;
		RenderEngine::EngineType type = engine->getEngineType();
		if (type == RenderEngine::OPENGL_ES || type == RenderEngine::OPENGL) {
			ret = new MapInfo_GL;
		}
		return ret;
	}

	sl_bool onInit(RenderEngine* _engine, RenderProgramInfo* _info)
	{
		RenderProgram3D::onInit(_engine, _info);

		RenderEngine::EngineType type = _engine->getEngineType();
		if (type == RenderEngine::OPENGL_ES || type == RenderEngine::OPENGL) {
			GLRenderEngine* engine = (GLRenderEngine*)_engine;
			MapInfo_GL* info = (MapInfo_GL*)_info;
			sl_uint32 program = info->program_GL;
			info->attrAltitude = engine->getAttributeLocation(program, "a_Altitude");
			return sl_true;
		}
		return sl_false;
	}

	sl_bool onPreRender(RenderEngine* _engine, RenderProgramInfo* _info, Primitive* primitive)
	{
		RenderEngine::EngineType type = _engine->getEngineType();
		if (type == RenderEngine::OPENGL_ES || type == RenderEngine::OPENGL) {
			GLRenderEngine* engine = (GLRenderEngine*)_engine;
			MapInfo_GL* info = (MapInfo_GL*)_info;
			SLIB_RENDER_GL_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(engine, info->attrPosition, DEM_Vertex, position);
			SLIB_RENDER_GL_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(engine, info->attrTexCoord, DEM_Vertex, texCoord);
			SLIB_RENDER_GL_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(engine, info->attrAltitude, DEM_Vertex, altitude);
			return sl_true;
		}
		return sl_false;
	}

	void onPostRender(RenderEngine* _engine, RenderProgramInfo* _info, Primitive* primitive)
	{
		RenderEngine::EngineType type = _engine->getEngineType();
		if (type == RenderEngine::OPENGL_ES || type == RenderEngine::OPENGL) {
			GLRenderEngine* engine = (GLRenderEngine*)_engine;
			MapInfo_GL* info = (MapInfo_GL*)_info;
			engine->disableVertexArrayAttribute(info->attrPosition);
			engine->disableVertexArrayAttribute(info->attrTexCoord);
			engine->disableVertexArrayAttribute(info->attrAltitude);
		}
	}

	String getGLSLVertexShader(RenderEngine* engine)
	{
		String source;
		source = SLIB_STRINGIFY(
			uniform mat4 u_Transform;
			attribute vec3 a_Position;
			attribute vec2 a_TexCoord;
			attribute float a_Altitude;
			varying vec2 v_TexCoord;
			varying float v_Altitude;
			void main() {
				vec4 P = vec4(a_Position, 1.0) * u_Transform;
				gl_Position = P;
				v_TexCoord = a_TexCoord;
				v_Altitude = a_Altitude;
			}
		);
		return source;
	}

	String getGLSLFragmentShader(RenderEngine* engine)
	{
		String source;
		source = SLIB_STRINGIFY(
			uniform sampler2D u_Texture;
			varying vec2 v_TexCoord;
			void main() {
				vec4 colorTexture = texture2D(u_Texture, v_TexCoord);
				gl_FragColor = colorTexture;
			}
		);
		return source;
	}
};

class _MapEarth_RenderProgram_SurfaceTile_TestTextureColor : public _MapEarth_RenderProgram_SurfaceTile
{
public:
	_MapEarth_RenderProgram_SurfaceTile_TestTextureColor() {}
	~_MapEarth_RenderProgram_SurfaceTile_TestTextureColor() {}

	String getGLSLFragmentShader(RenderEngine* engine)
	{
		String source;
		source = SLIB_STRINGIFY(
			varying vec2 v_TexCoord;
			void main() {
				gl_FragColor = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
			}
		);
		return source;
	}
};

void MapEarthRenderer::_initializeShaders()
{
	m_programSurfaceTile = new _MapEarth_RenderProgram_SurfaceTile;
	m_programBuilding = new RenderProgram3D_PositionNormalTexture_Diffuse;
	m_programLine = new RenderProgram3D_Position;
}

SLIB_MAP_NAMESPACE_END

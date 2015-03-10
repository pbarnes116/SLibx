#include "tile.h"

#include "../../slib/core/log.h"

SLIB_MAP_NAMESPACE_START
MapTile::MapTile()
{
	flagCanRender = sl_false;
}

MapTileManager::MapTileManager()
{
	initializeTileLifeMillseconds(5000);
	initializeMaximumLevel(10);
}

MapTileManager::~MapTileManager()
{
	release();
}

void MapTileManager::initialize()
{
	m_threadManage = Thread::start(SLIB_CALLBACK_PARAM1(&MapTileManager::_threadManageTile, (WeakRef<MapTileManager>)this));
}

void MapTileManager::release()
{
	Ref<Thread> thread = m_threadManage;
	if (thread.isNotNull()) {
		m_threadManage.setNull();
		thread->finishAndWait();
	}
}

void MapTileManager::_initializeTopTiles(sl_uint32 level, sl_uint32 nY, sl_uint32 nX)
{
	m_levelTop = level;
	m_nYTop = nY;
	m_nXTop = nX;
	Array2D< Ref<MapTile> > tiles = Array2D< Ref<MapTile> >::create(nX, nY);
	if (tiles.isNotEmpty()) {
		for (sl_uint32 i = 0; i < nY; i++) {
			for (sl_uint32 j = 0; j < nX; j++) {
				Ref<MapTile> tile = _createTile(level, i, j, 0, 0);
				if (tile.isNotNull()) {
					tiles.item(j, i) = tile;
				}
			}
		}
	}
	m_tilesTop = tiles;
}

void MapTileManager::_setTileLocation(MapTile* tile, sl_uint32 _level, sl_uint32 _y, sl_uint32 _x)
{
	tile->level = _level;
	tile->y = _y;
	tile->x = _x;

	tile->rect.bottomLeft = getLatLonFromTileLocation(_level, _y, _x);
	tile->rect.topRight = getLatLonFromTileLocation(_level, _y + 1, _x + 1);

	GeoLocation loc;
	loc.altitude = 0;
	// bottom-left
	loc.latitude = tile->rect.bottomLeft.latitude;
	loc.longitude = tile->rect.bottomLeft.longitude;
	tile->positions[0] = loc.convertToPosition();
	// bottom-right
	loc.latitude = tile->rect.bottomLeft.latitude;
	loc.longitude = tile->rect.topRight.longitude;
	tile->positions[1] = loc.convertToPosition();
	// top-left
	loc.latitude = tile->rect.topRight.latitude;
	loc.longitude = tile->rect.bottomLeft.longitude;
	tile->positions[2] = loc.convertToPosition();
	// top-right
	loc.latitude = tile->rect.topRight.latitude;
	loc.longitude = tile->rect.topRight.longitude;
	tile->positions[3] = loc.convertToPosition();
	// center
	loc.latitude = (tile->rect.bottomLeft.latitude + tile->rect.topRight.latitude) / 2;
	loc.longitude = (tile->rect.topRight.longitude + tile->rect.bottomLeft.longitude) / 2;
	tile->positionCenter = loc.convertToPosition();
}

void MapTileManager::_updateRenderState(MapEnvironment* environment)
{
	m_timeCurrent = Time::now();
	m_environment = environment;
}

sl_bool MapTileManager::_checkTileVisibleInViewFrustum(MapTile* tile)
{
	sl_bool flag = ViewFrustum::fromMVP(m_environment->transformViewProjection).containsFacets(tile->positions, 4);
	return flag;
}

void MapTileManager::_threadManageTile(WeakRef<MapTileManager> _manager)
{
	while (!Thread::isStoppingCurrent()) {
		{
			Ref<MapTileManager> manager = _manager.lock();
			if (manager.isNotNull()) {
				manager->_runThreadManageTileStep();
			} else {
				return;
			}
		}
		Thread::sleep(20);
	}
}

void MapTileManager::_runThreadManageTileStep()
{
	m_timeCurrent = Time::now();
	if (m_environment.isNotNull()) {
		ArrayInfo2D< Ref<MapTile> > tiles;
		if (m_tilesTop.getInfo(tiles)) {
			for (sl_size y = 0; y < tiles.height; y++) {
				for (sl_size x = 0; x < tiles.width; x++) {
					Ref<MapTile> tile = tiles.item(x, y);
					if (tile.isNotNull()) {
						_initializeTile(tile, sl_null);
					}
				}
			}
		}
		if ((m_timeCurrent - m_timeLastRenderRequest).getMillisecondsCount() > 1000) {
			m_environment->requestRender();
			m_timeLastRenderRequest = m_timeCurrent;
		}
	}
}

LatLon MapTileManager::getLatLonFromTileLocation(sl_uint32 level, sl_uint32 y, sl_uint32 x)
{
	LatLon ret;
	sl_uint32 n = 1 << (level - m_levelTop);
	ret.latitude = y * 180.0 / m_nYTop / n - 90.0;
	ret.longitude = x * 360.0 / m_nXTop / n - 180.0;
	return ret;
}

Ref<MapTile> MapTileManager::_createTile(sl_uint32 level, sl_uint32 y, sl_uint32 x, sl_uint32 iy, sl_uint32 ix)
{
	Ref<MapTile> tile = createTile();
	if (tile.isNotNull()) {
		tile->iy = iy;
		tile->ix = ix;
		_setTileLocation(tile, level, y, x);
	}
	return tile;
}

sl_bool MapTileManager::_initializeTile(MapTile* tile, MapTile* parent)
{
	if (!tile->flagCanRender) {
		if (!initializeTile(tile, parent)) {
			return sl_false;
		}
		tile->flagCanRender = sl_true;
		tile->timeLastAccess = m_timeCurrent;
	}
	sl_uint32 lifeTile = getTileLifeMillseconds();
	if ((m_timeCurrent - tile->timeLastAccess).getMillisecondsCount() > (sl_int64)lifeTile) {
		for (sl_uint32 i = 0; i < 2; i++) {
			for (sl_uint32 j = 0; j < 2; j++) {
				tile->children[i][j].setNull();
			}
		}
		if (parent) {
			_freeTile(tile);
			tile->flagCanRender = sl_false;
		}
	} else {
		for (sl_uint32 i = 0; i < 2; i++) {
			for (sl_uint32 j = 0; j < 2; j++) {
				Ref<MapTile> child = tile->children[i][j];
				if (child.isNotNull()) {
					_initializeTile(child, tile);
				}
			}
		}
	}
	return sl_true;
}

void MapTileManager::_freeTile(MapTile* tile)
{
	freeTile(tile);
}

void MapTileManager::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	_updateRenderState(environment);
	ArrayInfo2D< Ref<MapTile> > tiles;
	if (m_tilesTop.getInfo(tiles)) {
		for (sl_size y = 0; y < tiles.height; y++) {
			for (sl_size x = 0; x < tiles.width; x++) {
				Ref<MapTile> tile = tiles.item(x, y);
				if (tile.isNotNull()) {
					_renderTile(tile, engine, environment);
				}
			}
		}
	}
}

void MapTileManager::_renderTile(MapTile* tile, RenderEngine* engine, MapEnvironment* environment)
{
	if (!(tile->flagCanRender)) {
		return;
	}
	// check normal
	{
		Vector3 normal = environment->transformView.transformDirection(tile->positionCenter);
		normal.normalize();
		if (normal.z > 0.1f) {
			return;
		}
	}
	// check frustum
	if (!_checkTileVisibleInViewFrustum(tile)) {
		return;
	}

	tile->timeLastAccess = m_timeCurrent;

	sl_bool flagExtend = sl_false;
	// check size
	if (tile->level < getMaximumLevel()) {

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
			return;
		}
		if (nBehind != 0) {
			flagExtend = sl_true;
		} else {
			Triangle2 t;
			t.point1.x = ptBL.x / ptBL.z;
			t.point1.y = ptBL.y / ptBL.z;
			t.point2.x = ptBR.x / ptBR.z;
			t.point2.y = ptBR.y / ptBR.z;
			t.point3.x = ptTL.x / ptTL.z;
			t.point3.y = ptTL.y / ptTL.z;
			sl_real size = Math::abs(t.getSize());
			if (size > 0.2 * 0.2) {
				flagExtend = sl_true;
			}
		}
	}
	Ref<MapTile> children[2][2];
	if (flagExtend) {
		for (sl_uint32 i = 0; i < 2; i++) {
			for (sl_uint32 j = 0; j < 2; j++) {
				Ref<MapTile> child = tile->children[i][j];
				if (child.isNotNull()) {
					children[i][j] = child;
					child->timeLastAccess = m_timeCurrent;
				} else {
					child = _createTile(tile->level + 1, tile->y * 2 + i, tile->x * 2 + j, i, j);
					if (child.isNotNull()) {
						tile->children[i][j] = child;
						children[i][j] = child;
						child->timeLastAccess = m_timeCurrent;
					} else {
						flagExtend = sl_false;
					}
				}
			}
		}
	}
	if (flagExtend) {
		for (sl_uint32 i = 0; i < 2; i++) {
			for (sl_uint32 j = 0; j < 2; j++) {
				Ref<MapTile> child = children[i][j];
				if (! child->flagCanRender) {
					flagExtend = sl_false;
				}
			}
		}
	}
	if (flagExtend) {
		for (sl_uint32 i = 0; i < 2; i++) {
			for (sl_uint32 j = 0; j < 2; j++) {
				Ref<MapTile> child = children[i][j];
				_renderTile(child, engine, environment);
			}
		}
	} else {
		if (engine) {
			if (tile->flagCanRender) {
				renderTile(tile, engine, environment);
			}
		}
	}
}

SLIB_MAP_NAMESPACE_END

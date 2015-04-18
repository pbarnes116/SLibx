#include "map_earth_renderer.h"

SLIB_MAP_NAMESPACE_START

void MapEarthRenderer::setDataLoader(Ref<MapDataLoader> loader)
{
	m_tilesPicture->setDataLoader(loader);
	m_tilesDEM->setDataLoader(loader);
	m_tilesBuilding->setDataLoader(loader);
	m_tilesGISLine->setDataLoader(loader);
	m_tilesGISPoi->setDataLoader(loader);
}

void MapEarthRenderer::_runThreadData()
{
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		if (m_flagStartedRendering) {
			_runThreadDataStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now2- now).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
	}
}

void MapEarthRenderer::_runThreadDataEx()
{
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		if (m_flagStartedRendering) {
			_runThreadDataBuildingStep();
			_runThreadDataGISStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now2 - now).getMillisecondsCount(), _SI64(0), _SI64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
	}
}

void MapEarthRenderer::_runThreadDataStep()
{
	_loadRenderingTilesData();

	m_tilesPicture->freeOldTiles();
	m_tilesDEM->freeOldTiles();

	m_tilesRender->freeOldTiles();
}

void MapEarthRenderer::_loadRenderingTilesData()
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		MapTileLocationi loc = list[i];
		m_tilesPicture->loadTileHierarchically(loc);
		m_tilesDEM->loadTileHierarchically(loc);
	}
}

void MapEarthRenderer::_loadZeroLevelTiles()
{
	for (sl_uint32 iy = 0; iy < getCountY0(); iy++) {
		for (sl_uint32 ix = 0; ix < getCountX0(); ix++) {
			m_tilesPicture->loadTile(MapTileLocationi(0, iy, ix));
			m_tilesDEM->loadTile(MapTileLocationi(0, iy, ix));
		}
	}
}

void MapEarthRenderer::_runThreadDataBuildingStep()
{
	struct _BuildingInfo
	{
		Ref<VW_Building_ObjectInfo> info;
		sl_real distance;
	};
	struct _SortBuildingInfo
	{
	public:
		SLIB_INLINE static sl_real key(_BuildingInfo& info)
		{
			return info.distance;
		}
	};
	if (isShowBuilding()) {

		Vector3 eye = m_positionEye;

		List<_BuildingInfo> infos;

		// load building infors
		{
			sl_uint32 nMinBuildingLevel = getMinBuildingLevel();
			ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
			for (sl_size i = 0; i < list.count(); i++) {
				MapTileLocationi loc = list[i];
				if (loc.level >= nMinBuildingLevel) {
					do {
						if (Thread::isStoppingCurrent()) {
							return;
						}
						Ref<MapBuildingTile> tile = m_tilesBuilding->loadTile(loc);
						ListLocker< Ref<VW_Building_ObjectInfo> > objects(tile->objects);
						for (sl_size k = 0; k < objects.count(); k++) {
							Ref<VW_Building_ObjectInfo>& info = objects[k];
							if (info.isNotNull()) {
								if (m_viewFrustum.containsBox(info->bound)) {
									Ref<MapBuilding> rb = m_tilesBuilding->getBuilding(info->key);
									if (rb.isNull()) {
										_BuildingInfo o;
										o.info = info;
										if (info->flagBridge) {
											o.distance = 0;
										} else {
											o.distance = (eye - (Vector3)(info->bound.center())).getLength2p();
										}
										infos.add(o);
									}
								}
							}
						}
						loc.level--;
						loc.x >>= 1;
						loc.y >>= 1;
					} while (loc.level + 1 > nMinBuildingLevel);
				}
			}
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// select objects
		{
			infos = infos.sort<_SortBuildingInfo, sl_real>(sl_true);
			infos.setCount(Math::min(10, (sl_int32)(infos.count())));
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// load objects
		{
			ListLocker<_BuildingInfo> list(infos);
			for (sl_size i = 0; i < list.count(); i++) {
				m_tilesBuilding->loadBuilding(list[i].info);
			}
		}
		if (Thread::isStoppingCurrent()) {
			return;
		}
		// load building textures
		{
			m_tilesBuilding->loadRequestedDetailedTextures();
		}
	}
	if (Thread::isStoppingCurrent()) {
		return;
	}
	m_tilesBuilding->freeOldTiles();
	if (Thread::isStoppingCurrent()) {
		return;
	}
	m_tilesBuilding->freeOldBuildings();
	if (Thread::isStoppingCurrent()) {
		return;
	}
	m_tilesBuilding->freeOldDetailedTextures();
}

void MapEarthRenderer::_runThreadDataGISStep()
{
	ListLocker<MapTileLocationi> list(m_listRenderedTiles.duplicate());
	for (sl_size i = 0; i < list.count(); i++) {
		MapTileLocationi loc = list[i];
		m_tilesGISLine->loadTile(loc);
		m_tilesGISPoi->loadTile(loc);
	}

	m_tilesGISLine->freeOldTiles();
	m_tilesGISPoi->freeOldTiles();

}

SLIB_MAP_NAMESPACE_END

#ifndef CHECKHEADER_SLIB_MAP_TILE_BUILDING
#define CHECKHEADER_SLIB_MAP_TILE_BUILDING

#include "../../../inc/slibx/map/definition.h"

#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slibx/map/data.h"
#include "../../../inc/slibx/map/data_building.h"

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/time.h>

SLIB_MAP_NAMESPACE_BEGIN

class MapBuildingTile : public Referable
{
public:
	MapTileLocationi location;
	List< Ref<VW_Building_ObjectInfo> > objects;
	List< Ref<VW_Building_ObjectInfo> > bridges;
	Time timeLastAccess;
};

class MapBuilding : public Referable
{
public:
	String key;
	Ref<VW_Building_Object> object;
	Ref<VW_Building_ObjectInfo> info;
	Time timeLastAccess;
};

class MapBuildingTileManager : public Object
{
public:
	MapBuildingTileManager();
	~MapBuildingTileManager() {}

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxBuildingsCount);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxDetailedBuildingsCount);

protected:
	Map< MapTileLocationi, Ref<MapBuildingTile> > m_tiles;
	Map< String, Ref<MapBuilding> > m_buildings;

	struct _BuildingTextureAddress
	{
		String key;
		sl_uint32 index;
	};
	class _BuildingTexture : public Referable
	{
	public:
		String tkey;
		Ref<Texture> texture;
		Time timeLastAccess;
	};
	Map< String, Ref<_BuildingTexture> > m_detailedTextures;
	_BuildingTextureAddress m_requestedDetailedTexture;

public:
	Ref<MapBuildingTile> getTile(const MapTileLocationi& location);
	Ref<MapBuildingTile> loadTile(const MapTileLocationi& location);
	void freeOldTiles();

	SLIB_INLINE List< Ref<MapBuilding> > getBuildings()
	{
		return m_buildings.values();
	}
	Ref<MapBuilding> getBuilding(String key);
	Ref<MapBuilding> loadBuilding(VW_Building_ObjectInfo* info);
	void freeOldBuildings();

	void loadRequestedDetailedTextures();
	sl_bool getDetailedTexture(String key, sl_uint32 index, Ref<Texture>& texture);
	Ref<Texture> loadDetailedTexture(String key, sl_uint32 index);
	void requestDetailedTexture(String key, sl_uint32 index);
	void freeOldDetailedTextures();

protected:
	String _getDetailedTextureKey(String key, sl_uint32 index);

};
SLIB_MAP_NAMESPACE_END

#endif

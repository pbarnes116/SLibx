#ifndef CHECKHEADER_SLIB_MAP_TILE_RENDER
#define CHECKHEADER_SLIB_MAP_TILE_RENDER

#include "../../../inc/slibx/map/definition.h"

#include "map_tile_picture.h"
#include "map_tile_dem.h"

SLIB_MAP_NAMESPACE_START

class MapRenderTile : public Referable
{
public:
	MapTileLocationi location;

	Ref<MapPictureTile> picture;
	Ref<MapDEMTile> dem;
	
	Primitive primitive;
	
	Time timeLastAccess;
};

class MapRenderTileManager : public Object
{
public:
	MapRenderTileManager();
	~MapRenderTileManager() {}

public:
	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

protected:
	Map< MapTileLocationi, Ref<MapRenderTile> > m_tiles;

public:
	Ref<MapRenderTile> getTile(const MapTileLocationi& location);
	void saveTile(const MapTileLocationi& location, const Ref<MapRenderTile>& tile);

	void freeOldTiles();

};
SLIB_MAP_NAMESPACE_END

#endif

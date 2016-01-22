#ifndef CHECKHEADER_SLIB_MAP_TILE_RENDER
#define CHECKHEADER_SLIB_MAP_TILE_RENDER

#include "../../../inc/slibx/map/definition.h"

#include "map_tile_picture.h"
#include "map_tile_dem.h"
#include "map_tile_layer.h"

SLIB_MAP_NAMESPACE_BEGIN

class MapRenderTile : public Referable
{
public:
	MapTileLocationi location;

	Ref<MapPictureTile> picture;
	Ref<MapDEMTile> dem;
	Ref<MapLayerTile> layers[SLIB_SMAP_MAX_LAYERS_COUNT];
	
	Primitive primitive;
	
	Time timeLastAccess;
};

class MapRenderTileManager : public Object
{
public:
	MapRenderTileManager();
	~MapRenderTileManager() {}

public:
	SLIB_PROPERTY(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY(sl_uint32, MaxTilesCount);

protected:
	HashMap< MapTileLocationi, Ref<MapRenderTile> > m_tiles;

public:
	Ref<MapRenderTile> getTile(const MapTileLocationi& location);
	void saveTile(const MapTileLocationi& location, const Ref<MapRenderTile>& tile);

	void freeOldTiles();

};
SLIB_MAP_NAMESPACE_END

#endif

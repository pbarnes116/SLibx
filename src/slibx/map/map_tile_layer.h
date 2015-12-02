#ifndef CHECKHEADER_SLIB_MAP_TILE_LAYER
#define CHECKHEADER_SLIB_MAP_TILE_LAYER

#include "../../../inc/slibx/map/definition.h"
#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slibx/map/data.h"

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/time.h>
#include <slib/render/texture.h>

SLIB_MAP_NAMESPACE_BEGIN

class MapLayerTile : public Referable
{
public:
	MapTileLocationi location;
	Ref<Texture> texture;
	Time timeLastAccess;
};

class MapLayerTileManager : public Object
{
public:
	MapLayerTileManager();
	~MapLayerTileManager();

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

protected:
	Map< MapTileLocationi, Ref<MapLayerTile> > m_tiles[SLIB_SMAP_MAX_LAYERS_COUNT];

public:
	Ref<MapLayerTile> getTile(sl_uint32 layer, const MapTileLocationi& location);
	Ref<MapLayerTile> getTileHierarchically(sl_uint32 layer, const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	Ref<MapLayerTile> loadTile(sl_uint32 layer, const MapTileLocationi& location);
	Ref<MapLayerTile> loadTileHierarchically(sl_uint32 layer, const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	void freeOldTiles();

};
SLIB_MAP_NAMESPACE_END

#endif

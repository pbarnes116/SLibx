#ifndef CHECKHEADER_SLIB_MAP_TILE_PICTURE
#define CHECKHEADER_SLIB_MAP_TILE_PICTURE

#include "definition.h"

#include "util.h"
#include "data.h"

#include "../../slib/core/object.h"
#include "../../slib/core/map.h"
#include "../../slib/core/time.h"
#include "../../slib/render/texture.h"

SLIB_MAP_NAMESPACE_START

class MapPictureTile : public Referable
{
public:
	MapTileLocationi location;
	Ref<Texture> texture;
	Time timeLastAccess;
};

class MapPictureTileManager : public Object
{
public:
	MapPictureTileManager();
	~MapPictureTileManager() {}

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

protected:
	Map< MapTileLocationi, Ref<MapPictureTile> > m_tiles;

public:
	Ref<MapPictureTile> getTile(const MapTileLocationi& location);
	Ref<MapPictureTile> getTileHierarchically(const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	Ref<MapPictureTile> loadTile(const MapTileLocationi& location);
	Ref<MapPictureTile> loadTileHierarchically(const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	void freeOldTiles();

};
SLIB_MAP_NAMESPACE_END

#endif

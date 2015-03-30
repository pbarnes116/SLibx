#ifndef CHECKHEADER_SLIB_MAP_TILE_DEM
#define CHECKHEADER_SLIB_MAP_TILE_DEM

#include "definition.h"

#include "util.h"
#include "data.h"
#include "dem.h"

#include "../../slib/core/object.h"
#include "../../slib/core/map.h"
#include "../../slib/core/time.h"

SLIB_MAP_NAMESPACE_START

class MapDEMTile : public Referable
{
public:
	MapTileLocationi location;
	DEM dem;
	Time timeLastAccess;
};

class MapDEMTileManager : public Object
{
public:
	MapDEMTileManager();
	~MapDEMTileManager() {}

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

protected:
	Map< MapTileLocationi, Ref<MapDEMTile> > m_tiles;

public:
	Ref<MapDEMTile> getTile(const MapTileLocationi& location);
	Ref<MapDEMTile> getTileHierarchically(const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	Ref<MapDEMTile> loadTile(const MapTileLocationi& location);
	Ref<MapDEMTile> loadTileHierarchically(const MapTileLocationi& location, Rectangle* rectangle = sl_null);

	void freeOldTiles();

	static float getAltitudeFromDEM(float x, float y, MapDEMTile* tile);
	SLIB_INLINE static float getAltitudeFromDEM(const Vector2& pos, MapDEMTile* tile)
	{
		return getAltitudeFromDEM(pos.x, pos.y, tile);
	}
	float getAltitude(const MapTileLocation& location);

};

SLIB_MAP_NAMESPACE_END

#endif

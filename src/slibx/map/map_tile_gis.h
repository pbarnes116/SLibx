#ifndef CHECKHEADER_SLIB_MAP_TILE_GIS
#define CHECKHEADER_SLIB_MAP_TILE_GIS

#include "../../../inc/slibx/map/definition.h"

#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slibx/map/data.h"
#include "../../../inc/slibx/map/data_gis.h"

#include "map_tile_dem.h"

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/time.h>
#include <slib/graphics/freetype.h>

SLIB_MAP_NAMESPACE_BEGIN

class MapGISShape : public Referable
{
public:
	Color color;
	sl_real width;
	List<MapGISLineData> lines;

	Ref<VertexBuffer> vb;

	Ref<MapDEMTile> demTileRef;
};

class MapGISLineTile : public Referable
{
public:
	MapTileLocationi location;
	sl_bool flagLoaded;
	List< Ref<MapGISShape> > shapes;
	Time timeLastAccess;
};

class MapGISLineTileManager : public Object
{
public:
	MapGISLineTileManager();
	~MapGISLineTileManager() {}

public:
	SLIB_REF_PROPERTY(MapDataLoader, DataLoader);
	SLIB_PROPERTY(Sizef, ViewportSize);
	SLIB_PROPERTY(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY(sl_uint32, MaxTilesCount);

protected:
	HashMap< MapTileLocationi, Ref<MapGISLineTile> > m_tiles;
	MapGISLine_DataLoader m_dataLoader;

public:
	Ref<MapGISLineTile> getTile(const MapTileLocationi& location);
	Ref<MapGISLineTile> loadTile(const MapTileLocationi& location);
	void freeOldTiles();

	SLIB_INLINE void setWayNames(Map<sl_int64, String> wayNames)
	{
		m_dataLoader.setWayNames(wayNames);
	}
};

class MapGISPoi : public Referable
{
public:
	SLIB_INLINE MapGISPoi()
	{
		_flagInit = sl_false;
	}

public:
	LatLon location;
	sl_uint32 level;
	sl_int64 id;
	Ref<Texture> texture;

	String text;
	MapGISPoiData::Type type;

	sl_int32 showMinLevel;
	sl_uint32 fontSize;
	Color clr;

	void init(const Map<sl_int64, MapGISPoiInfo>& info);

private:
	sl_bool _flagInit;
};

class MapGISPoiTile : public Referable
{
public:
	MapTileLocationi location;
	sl_bool flagLoaded;
	List< Ref<MapGISPoi> > pois;
	Time timeLastAccess;
};

class MapGISPoiTileManager : public Object
{
public:
	MapGISPoiTileManager();
	~MapGISPoiTileManager() {}

public:
	SLIB_REF_PROPERTY(MapDataLoader, DataLoader);
	SLIB_PROPERTY(Sizef, ViewportSize);
	SLIB_PROPERTY(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY(sl_uint32, MaxTilesCount);

	SLIB_PROPERTY(Ref<FreeType>, FontForPOI);

protected:
	HashMap< MapTileLocationi, Ref<MapGISPoiTile> > m_tiles;
	MapGISPoi_DataLoader m_dataLoader;

public:
	Ref<MapGISPoiTile> getTile(const MapTileLocationi& location);
	Ref<MapGISPoiTile> loadTile(const MapTileLocationi& location);
	void freeOldTiles();

};
SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_TILE_GIS
#define CHECKHEADER_SLIB_MAP_TILE_GIS

#include "definition.h"

#include "util.h"
#include "data.h"
#include "data_gis.h"
#include "tile_dem.h"

#include "../../slib/core/object.h"
#include "../../slib/core/map.h"
#include "../../slib/core/time.h"
#include "../../slib/image/freetype.h"

SLIB_MAP_NAMESPACE_START

struct MapGISShape
{
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
	List<MapGISShape> shapes;
	Time timeLastAccess;
};

class MapGISLineTileManager : public Object
{
public:
	MapGISLineTileManager();
	~MapGISLineTileManager() {}

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

protected:
	Map< MapTileLocationi, Ref<MapGISLineTile> > m_tiles;
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

struct MapGISPoi
{
	LatLon location;
	MAP_GIS_POI_TYPE type;
	Ref<Texture> texture;
};
class MapGISPoiTile : public Referable
{
public:
	MapTileLocationi location;
	sl_bool flagLoaded;
	List<MapGISPoi> pois;
	Time timeLastAccess;
};

class MapGISPoiTileManager : public Object
{
public:
	MapGISPoiTileManager();
	~MapGISPoiTileManager() {}

public:
	SLIB_PROPERTY_INLINE(Ref<MapDataLoader>, DataLoader);

	SLIB_PROPERTY_INLINE(sl_uint32, TileLifeMillseconds);
	SLIB_PROPERTY_INLINE(sl_uint32, MaxTilesCount);

	SLIB_PROPERTY_INLINE(Ref<FreeType>, FontForPOI);

protected:
	Map< MapTileLocationi, Ref<MapGISPoiTile> > m_tiles;
	MapGISPoi_DataLoader m_dataLoader;

public:
	Ref<MapGISPoiTile> getTile(const MapTileLocationi& location);
	Ref<MapGISPoiTile> loadTile(const MapTileLocationi& location);
	void freeOldTiles();

	SLIB_INLINE void setPoiInformation(Map<sl_int64, MapGISPoiInfo> poiInfo)
	{
		m_dataLoader.setPoiInformation(poiInfo);
	}

};
SLIB_MAP_NAMESPACE_END

#endif

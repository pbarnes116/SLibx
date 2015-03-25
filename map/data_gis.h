#ifndef CHECKHEADER_SLIB_MAP_DATA_GIS
#define CHECKHEADER_SLIB_MAP_DATA_GIS

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"
#include "../../slib/db.h"
SLIB_MAP_NAMESPACE_START

enum MAP_GISPOI_TYPE {
	POITypeNone = 0
	, AmenityStart = 1
	, Transport = 10
	, ServiceBuilding = 20
	, Tourism = 30
	, Healthcare = 40
	, SocialBuilding = 50
	, Education = 60
	, LiteratureBuilding = 70
	, OtherBuilding = 80
	, AmenityEnd = 99

	, PlaceStart = 100
	, PlaceCountry = 110
	, PlaceState = 109
	, PlaceCity = 108
	, PlaceTown = 107
	, PlaceVilliage = 106		
	, PlaceExtra = 101
	, PlaceEnd = 110

	, Historic =  120
	, Military = 130
	, Natural = 140
	, RailStation = 160

	, Hospital = 200
	, Restraunt = 201
	, Hotel = 202
	, Parking = 203
	, FillingStation = 204

};

struct Map_GIS_Poi
{
	sl_int64 id;
	MAP_GISPOI_TYPE type;
	LatLon location;
	String name;
};

struct Map_GIS_Line
{
	sl_int64 id;
	LatLon start;
	LatLon end;
};

class Map_GIS_Shape : public Referable
{
public:
	SLIB_INLINE Map_GIS_Shape()
	{
		lines.clear();
		width = 0.f;
		showMinLevel = 5;
		initShape();
	}
	~Map_GIS_Shape()
	{

	}
	void initShape();

	sl_uint32 boundType;
	sl_uint32 highWayType;
	sl_uint32 extraType;
	sl_uint32 naturalType;

	List<Map_GIS_Line> lines;
	Color clr;
	sl_real width;
	sl_uint32 showMinLevel;
};

class Map_GIS_Poi_TileLoader : public Referable
{
public:
	void openNameDatabase(const String& dbPath);
	List<Map_GIS_Poi> loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location);
private:
	String getPoiNameFromDatabase(sl_int64 id);
private:
	Ref<SqliteDatabase> dbPoiName;
};

class Map_GIS_Line_Tile
{
public:
	Map<sl_int32, Ref<Map_GIS_Shape>> shapes;
	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

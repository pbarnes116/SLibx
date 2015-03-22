#ifndef CHECKHEADER_SLIB_MAP_DATA_GIS
#define CHECKHEADER_SLIB_MAP_DATA_GIS

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START


typedef enum GISPOI_TYPE {
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
	, PlaceVilliage = 107			
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

}GISPOI_TYPE;

struct GIS_Poi
{
	sl_int64 id;
	GISPOI_TYPE type;
	LatLon location;
};

struct GIS_Line
{
	sl_int64 id;
	LatLon start;
	LatLon end;
};

class GIS_Shape : public Referable
{
public:
	SLIB_INLINE GIS_Shape()
	{
		lines.clear();
		width = 0.f;
		showMinLevel = 5;
		initShape();
	}
	~GIS_Shape()
	{

	}
	void initShape();

	sl_uint32 boundType;
	sl_uint32 highWayType;
	sl_uint32 extraType;
	sl_uint32 naturalType;

	List<GIS_Line> lines;
	Color clr;
	sl_real width;
	sl_uint32 showMinLevel;
};

class GIS_Poi_Tile
{
public:
	List<GIS_Poi> pois;
	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

class GIS_Line_Tile
{
public:
	Map<sl_int32, Ref<GIS_Shape>> shapes;
	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

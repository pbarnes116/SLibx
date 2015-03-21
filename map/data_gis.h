#ifndef CHECKHEADER_SLIB_MAP_DATA_GIS
#define CHECKHEADER_SLIB_MAP_DATA_GIS

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START


typedef enum GISPOI_TYPE {
	NodeTypeNone = 0
	, Amenity = 100
	, Historic = 200
	, Military = 300
	, Natural = 400
	, Tourism = 500
	, RailStation = 600
	, PlaceStart = 1000
	, PlaceCountry = 1000
	, PlaceState = 1001
	, PlaceCity = 1002
	, PlaceVilliage = 1003			
	, PlaceExtra = 1004
	, PlaceEnd = 1100
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

	sl_int32 boundType;
	sl_int32 highWayType;
	sl_int32 extraType;
	sl_int32 naturalType;

	List<GIS_Line> lines;
	Color clr;
	sl_real width;
	sl_int32 showMinLevel;
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

#ifndef CHECKHEADER_SLIB_MAP_GIS
#define CHECKHEADER_SLIB_MAP_GIS

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

typedef enum GIS_SHAPE_TYPE {
	ShapeTypeNone = 0
	, HighwayMotor = 100
	, HighwayTrunk = 101
	, HighwayPrimary = 102
	, HighwaySecondary = 103
	, HighwayTeritary = 104
	, HighwayResidental = 105
	, HighwayExtra = 106

	, Railway = 200
	, OneWay = 201
	, AeroWay = 202
	, WaterWay = 203
	, FootWay = 204
	, AerialWay = 205
	, CycleWay = 206
	
	, Tunnel = 220
	, Route = 221

	, BoundaryStart = 300
	, BoundaryAdmin = 301
	, BoundaryMapBox = 302
	, BoundaryProtectedArea = 303
	, BoundaryExtra = 304
	, BoundaryEnd = 350
}GIS_SHAPE_TYPE;

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
		type = ShapeTypeNone;
		width = 0.f;
		initShape();
	}
	~GIS_Shape()
	{

	}
	void initShape();
	GIS_SHAPE_TYPE type;
	List<GIS_Line> lines;
	Color clr;
	sl_real width;
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
	Map<GIS_SHAPE_TYPE, Ref<GIS_Shape>> shapes;
	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_DATA_GIS
#define CHECKHEADER_SLIB_MAP_DATA_GIS

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"
#include "../../slib/db.h"

SLIB_MAP_NAMESPACE_START

enum MAP_GIS_POI_TYPE {
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

struct MapGISPoiData
{
	sl_int64 id;
	MAP_GIS_POI_TYPE type;
	LatLon location;
	String name;

	sl_int32 showMinLevel;
	sl_uint32 fontSize;
	Color clr;
	void initPoi();
};

struct MapGISLineData
{
	sl_int64 id;
	LatLon start;
	LatLon end;
	String name;
};

class MapGISShapeData : public Referable
{
public:
	SLIB_INLINE MapGISShapeData()
	{
		lines.clear();
		width = 0.f;
		showMinLevel = 0;
		initShape();
	}
	~MapGISShapeData()
	{

	}
	void initShape();
	sl_uint32 boundType;
	sl_uint32 highWayType;
	sl_uint32 extraType;
	sl_uint32 naturalType;

	List<MapGISLineData> lines;
	Color clr;
	sl_real width;
	sl_uint32 showMinLevel;
};

struct MapGISPoiInfo
{
	String name;
	sl_int32 type;
};


class MapGISPoi_DataLoader : public Referable
{
public:
	void setPoiInformation(Map<sl_int64, MapGISPoiInfo> _poiInformation)
	{
		this->poiInformation = _poiInformation;
	}
	List<MapGISPoiData> loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location);
private:
	Map<sl_int64, MapGISPoiInfo> poiInformation;
};

class MapGISLine_DataLoader: public Referable
{
public:
	void setWayNames(Map<sl_int64, String> _wayNames)
	{
		this->wayNames = _wayNames;
	}

	Map<sl_int64, String> wayNames;
	Map<sl_int32, Ref<MapGISShapeData>> loadTile(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

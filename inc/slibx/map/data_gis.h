#ifndef CHECKHEADER_SLIB_MAP_DATA_GIS
#define CHECKHEADER_SLIB_MAP_DATA_GIS

#include "definition.h"
#include "data.h"

#include <slib/render/engine.h>
#include <slib/math/geograph.h>
#include <slib/db.h>

SLIB_MAP_NAMESPACE_BEGIN

struct SLIB_EXPORT MapGISPoiData
{
	sl_int64 id;
	LatLon location;
	String name;

	enum Type {
		typeNone = 0
		, typeAmenityStart = 1
		, typeTransport = 10
		, typeServiceBuilding = 20
		, typeTourism = 30
		, typeHealthcare = 40
		, typeSocialBuilding = 50
		, typeEducation = 60
		, typeLiteratureBuilding = 70
		, typeOtherBuilding = 80
		, typeAmenityEnd = 99

		, typePlaceStart = 100
		, typePlaceCountry = 110
		, typePlaceState = 109
		, typePlaceCity = 108
		, typePlaceTown = 107
		, typePlaceVilliage = 106
		, typePlaceExtra = 101
		, typePlaceEnd = 115

		, typeHistoric = 120
		, typeMilitary = 130
		, typeNatural = 140
		, typeRailStation = 160

		, typeHospital = 200
		, typeRestraunt = 201
		, typeHotel = 202
		, typeParking = 203
		, typeFillingStation = 204

		, typeNaturalMountain = 300
		, typeNaturalRiver = 301
		, typeNaturalLake = 302

	};
	Type type;
};

struct SLIB_EXPORT MapGISLineData
{
	sl_int64 id;
	LatLon start;
	LatLon end;
	String name;
};

class SLIB_EXPORT MapGISShapeData : public Referable
{
public:
	SLIB_INLINE MapGISShapeData()
	{
		width = 0.f;
		showMinLevel = 0;
		initShape();
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

struct SLIB_EXPORT MapGISPoiInfo
{
	String name;
	sl_int32 type;
};


class SLIB_EXPORT MapGISPoi_DataLoader : public Referable
{
public:
	List<MapGISPoiData> loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location);
};

class SLIB_EXPORT MapGISLine_DataLoader: public Referable
{
public:
	void setWayNames(Map<sl_int64, String> _wayNames)
	{
		this->wayNames = _wayNames;
	}

	SafeMap<sl_int64, String> wayNames;
	
	Map<sl_int32, Ref<MapGISShapeData>> loadTile(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

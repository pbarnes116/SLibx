#include "data_gis.h"

#include "../../slib/core/io.h"

#include "data_config.h"

SLIB_MAP_NAMESPACE_START

List<Map_GIS_Poi> Map_GIS_Poi_TileLoader::loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	List<Map_GIS_Poi> ret;
	Memory mem = data->loadData(type, location, SLIB_MAP_GIS_PACKAGE_DIMENSION, SLIB_MAP_GIS_POI_TILE_EXT);
	if (mem.isEmpty()) {
		return ret;
	}
	MemoryReader reader(mem);
	sl_int32 poiCount = reader.readInt32CVLI();
	sl_int64 timeStamp = reader.readInt64CVLI();
	for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
		Map_GIS_Poi poi;
		poi.id = reader.readInt64CVLI();
		poi.type = (MAP_GISPOI_TYPE)reader.readInt32CVLI();
		poi.location.latitude = reader.readDouble();
		poi.location.longitude = reader.readDouble();

		Variant poiInfo = poiInformation.getValue(poi.id, Variant::null());
		if (poiInfo.isNotNull()) {
			poi.name = poiInfo.getField("name").getString();
			
			poi.type = (MAP_GISPOI_TYPE)poiInfo.getField("type").getInt32();
			poi.initPoi();
			if (poi.type != MAP_GISPOI_TYPE::POITypeNone && poi.id > 0 && poi.name.length() > 0) {
				ret.add(poi);
			}
		}
		
	}
	return ret;
}

Map<sl_int32, Ref<Map_GIS_Shape>> Map_GIS_Line_TileLoader::loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	Map<sl_int32, Ref<Map_GIS_Shape>> ret;
	Memory mem = data->loadData(type, location, SLIB_MAP_GIS_PACKAGE_DIMENSION, SLIB_MAP_GIS_LINE_TILE_EXT);
	if (mem.isEmpty()) {
		return ret;
	}

	MemoryReader reader(mem);
	sl_int32 poiCount = reader.readInt32CVLI();
	sl_int64 timeStamp = reader.readInt64CVLI();
	for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
		Map_GIS_Line line;

		line.id = reader.readInt64CVLI();
		sl_int32 shapeType = (sl_int32)reader.readInt32CVLI();
		line.start.latitude = reader.readDouble();
		line.start.longitude = reader.readDouble();
		line.end.latitude = reader.readDouble();
		line.end.longitude = reader.readDouble();
		line.name = wayNames.getValue(line.id, _SLT(""));

		if (shapeType > 0 && line.id > 0) {
			Ref<Map_GIS_Shape> shape = ret.getValue(shapeType, sl_null);
			if (shape.isNotNull()) {
				shape->lines.add(line);
			} else {
				shape = new Map_GIS_Shape;
				shape->boundType = (shapeType >> 12) & 0xF;
				shape->highWayType = (shapeType >> 8) & 0xF;
				shape->naturalType = (shapeType >> 4) & 0xF;
				shape->extraType = (shapeType)& 0xF;
				shape->initShape();
				shape->lines.add(line);
				ret.put(shapeType, shape);
			}
		}
	}
	return ret;
}

void Map_GIS_Poi::initPoi()
{
	if (type != POITypeNone) {
		showMinLevel = 14;
		clr = Color::LightGoldenrodYellow;
		fontSize = 15;
		if (type == PlaceCountry || type == PlaceState) {
			showMinLevel = 6;
			clr = Color::Yellow;
			fontSize = 30;
		} else if (type == PlaceCity) {
			showMinLevel = 7;
			clr = Color::YellowGreen;
			fontSize = 24;
		} else if (type == PlaceTown) {
			showMinLevel = 11;
			clr = Color::Orange;
			fontSize = 20;
		} else if (type == PlaceVilliage) {
			showMinLevel = 12;
			clr = Color::Orange;
			fontSize = 15;
		}
	} else {
		showMinLevel = 14;

	}

}

void Map_GIS_Shape::initShape()
{
	showMinLevel = 15;
	width = 2.0f;
	if (boundType > 0) {
		clr = Color::Yellow;
		if (boundType < 5) {
			width = 2.0f;
			showMinLevel = 6;
		} else if (boundType < 8) {
			width = 2.0f;
			showMinLevel = 10;
		}
		if (highWayType > 0) {
			if (highWayType < 2) {
				width = 2.0f;
				clr = Color::LightSalmon;
				showMinLevel = 7;
			} else if (highWayType < 4) {
				width = 2.0f;
				clr = Color::LightSalmon;
				showMinLevel = 9;
			}
		}
	}
	if (showMinLevel != 15) {
		return;
	}
	if (highWayType > 0) {
		clr = Color::LightSalmon;
		if (highWayType < 2) {
			width = 2.0f;
			showMinLevel = 7;
		} else if (highWayType < 4) {
			width = 2.0f;
			showMinLevel = 9;
		} else if (highWayType == 4) {
			width = 1.0f;
			showMinLevel = 11;
		} else if (highWayType < 7) {
			width = 1.0f;
			showMinLevel = 13;
		} else {
			width = 1.0f;
			clr = Color::White;
			showMinLevel = 14;
		}
	} else if (extraType > 0) {
		showMinLevel = 14;
		if (extraType == 4) {
			clr = Color::LightCyan;
			showMinLevel = 8;
		} else if (extraType == 1 || extraType == 4) {
			clr = Color::LightBlue;
		} else if (extraType == 3) {
			clr = Color::Brown;
		} else if (extraType == 5 || extraType == 6) {
			clr = Color::LightGreen;
		}
	}
	else {
		clr = Color::White;
	}
}

SLIB_MAP_NAMESPACE_END

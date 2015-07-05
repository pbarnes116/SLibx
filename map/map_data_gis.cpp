#include "../../../inc/slibx/map/data_gis.h"

#include "../../../inc/slib/core/io.h"

#include "map_data_config.h"

SLIB_MAP_NAMESPACE_BEGIN

List<MapGISPoiData> MapGISPoi_DataLoader::loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	List<MapGISPoiData> ret;
	Memory mem = data->loadData(type, location, SLIB_MAP_GIS_PACKAGE_DIMENSION, SLIB_MAP_GIS_POI_TILE_EXT);
	if (mem.isNull()) {
		return ret;
	}
	MemoryReader reader(mem);
	sl_int32 poiCount = reader.readInt32CVLI();
	sl_int64 timeStamp = reader.readInt64CVLI();
	for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
		MapGISPoiData poi;
		poi.id = reader.readInt64CVLI();
		MapGISPoiData::Type type = (MapGISPoiData::Type)(reader.readInt32CVLI());
		poi.location.latitude = reader.readDouble();
		poi.location.longitude = reader.readDouble();
		poi.type = type;
		if (poi.id > 0) {
			ret.add(poi);
		}
	}
	//ret = ret.sortBy<MapGISPoiData_Sort, MAP_GIS_POI_TYPE>(sl_false);
	return ret;
}

Map< sl_int32, Ref<MapGISShapeData> > MapGISLine_DataLoader::loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	Map< sl_int32, Ref<MapGISShapeData> > ret;
	Memory mem = data->loadData(type, location, SLIB_MAP_GIS_PACKAGE_DIMENSION, SLIB_MAP_GIS_LINE_TILE_EXT);
	if (mem.isNull()) {
		return ret;
	}

	MemoryReader reader(mem);
	sl_int32 poiCount = reader.readInt32CVLI();
	sl_int64 timeStamp = reader.readInt64CVLI();
	for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
		MapGISLineData line;

		line.id = reader.readInt64CVLI();
		sl_int32 shapeType = (sl_int32)reader.readInt32CVLI();
		line.start.latitude = reader.readDouble();
		line.start.longitude = reader.readDouble();
		line.end.latitude = reader.readDouble();
		line.end.longitude = reader.readDouble();
		line.name = wayNames.getValue(line.id, String::null());

		if (shapeType > 0 && line.id > 0) {
			Ref<MapGISShapeData> shape = ret.getValue(shapeType, sl_null);
			if (shape.isNotNull()) {
				shape->lines.add(line);
			} else {
				shape = new MapGISShapeData;
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

void MapGISShapeData::initShape()
{
	showMinLevel = 15;
	width = 2.0f;
	if (boundType > 0) {
		clr = Color::Yellow;
		if (boundType < 5) {
			width = 2.0f;
			showMinLevel = 5;
		} else if (boundType < 8) {
			width = 2.0f;
			showMinLevel = 8;
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

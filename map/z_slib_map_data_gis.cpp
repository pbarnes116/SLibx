#include "data_gis.h"

#include "../../slib/core/io.h"

SLIB_MAP_NAMESPACE_START
class _GIS_Poi_Loader
{
public:

	List<GIS_Poi> pois;

	sl_bool loadFromFile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
	{
		Memory mem = data->loadData(type, location);
		if (mem.isEmpty()) {
			return sl_false;
		}
		MemoryReader reader(mem);
		sl_int32 poiCount = reader.readInt32CVLI();
		sl_int64 timeStamp = reader.readInt64CVLI();
		for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
			GIS_Poi poi;
			poi.id = reader.readInt64CVLI();
			poi.type = (GISPOI_TYPE)reader.readInt32CVLI();
			poi.location.latitude = reader.readDouble();
			poi.location.longitude = reader.readDouble();

			if (poi.type != GISPOI_TYPE::POITypeNone && poi.id > 0) {
				pois.add(poi);
			}
		}
		return sl_true;
	}
};


class _GIS_Shape_Loader
{
public:

	Map<sl_int32, Ref<GIS_Shape>> shapes;

	sl_bool loadFromFile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
	{
		Memory mem = data->loadData(type, location);
		if (mem.isEmpty()) {
			return sl_false;
		}

		MemoryReader reader(mem);
		sl_int32 poiCount = reader.readInt32CVLI();
		sl_int64 timeStamp = reader.readInt64CVLI();
		for (sl_int32 poiIndex = 0; poiIndex < poiCount; poiIndex++) {
			GIS_Line line;

			line.id = reader.readInt64CVLI();
			sl_int32 shapeType = (sl_int32)reader.readInt32CVLI();
			line.start.latitude = reader.readDouble();
			line.start.longitude = reader.readDouble();
			line.end.latitude = reader.readDouble();
			line.end.longitude = reader.readDouble();

			if (shapeType > 0 && line.id > 0) {
				Ref<GIS_Shape> shape = shapes.getValue(shapeType, sl_null);
				if (shape.isNotNull()) {
					shape->lines.add(line);
				}
				else {
					shape = new GIS_Shape;

					shape->boundType = (shapeType >> 12) & 0xF;
					shape->highWayType = (shapeType >> 8) & 0xF;
					shape->naturalType = (shapeType >> 4) & 0xF;
					shape->extraType = (shapeType)& 0xF;

					shape->initShape();
					shapes.put(shapeType, shape);
					shape->lines.add(line);
				}
			}
		}
		return sl_true;
	}
};

sl_bool GIS_Poi_Tile::load(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	_GIS_Poi_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		pois = loader.pois;
		return sl_true;
	}
	return sl_false;
}

sl_bool GIS_Line_Tile::load(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	_GIS_Shape_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		shapes = loader.shapes;
		return sl_true;
	}
	return sl_false;
}


void GIS_Shape::initShape()
{
	if (boundType > 0) {
		clr = Color::Yellow;
		if (boundType < 5) {
			showMinLevel = 5;
		} else if (boundType < 8) {
			showMinLevel = 10;
		} else {
			showMinLevel = 13;
		}
		if (highWayType > 0) {
			if (highWayType < 2) {
				clr = Color::LightSalmon;
				showMinLevel = 7;
			} else if (highWayType < 3) {
				clr = Color::LightSalmon;
				showMinLevel = 9;
			}
		}
		
	} else if (highWayType > 0) {
		if (highWayType < 2) {
			clr = Color::LightSalmon;
			showMinLevel = 7;
		} else if (highWayType < 4) {
			clr = Color::LightSalmon;
			showMinLevel = 9;
		} else if (highWayType == 4) {
			clr = Color::LightSalmon;
			showMinLevel = 11;
		} else if (highWayType < 7) {
			clr = Color::LightSalmon;
			showMinLevel = 13;
		} else {
			clr = Color::White;
			showMinLevel = 14;
		}
	} else if (extraType > 0) {
		if (extraType == 1 || extraType == 4) {
			clr = Color::LightCyan; 
		} else if (extraType == 2) {
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

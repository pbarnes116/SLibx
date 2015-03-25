#include "data_gis.h"

#include "../../slib/core/io.h"

#include "data_config.h"

SLIB_MAP_NAMESPACE_START

class _Map_GIS_Shape_Loader
{
public:

	Map<sl_int32, Ref<Map_GIS_Shape>> shapes;

	sl_bool loadFromFile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
	{
		Memory mem = data->loadData(type, location, SLIB_MAP_TILE_PACKAGE_DIMENSION, SLIB_MAP_GIS_LINE_TILE_EXT);
		if (mem.isEmpty()) {
			return sl_false;
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

			if (shapeType > 0 && line.id > 0) {
				Ref<Map_GIS_Shape> shape = shapes.getValue(shapeType, sl_null);
				if (shape.isNotNull()) {
					shape->lines.add(line);
				}
				else {
					shape = new Map_GIS_Shape;

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

void Map_GIS_Poi_TileLoader::openNameDatabase(const String& dbPath)
{
	dbPoiName = SqliteDatabase::createDatabase();
	Database::Param param;
	dbPoiName->connect(param, dbPath);
}

String Map_GIS_Poi_TileLoader::getPoiNameFromDatabase(sl_int64 id)
{
	String ret = _SLT("");
	String sql = String(_SLT("select f_name from tbl_poi_name where f_id = ")) + String::fromInt64(id);
	ListLocker< Map<String, Variant> > result(dbPoiName->executeSelectQuery(sql));
	if (result.count() > 0) {
		Variant item = result[0];
		ret = item.getField("f_name").getString();
	}
	return ret;
}

List<Map_GIS_Poi> Map_GIS_Poi_TileLoader::loadTile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	List<Map_GIS_Poi> ret;
	Memory mem = data->loadData(type, location, SLIB_MAP_TILE_PACKAGE_DIMENSION, SLIB_MAP_GIS_POI_TILE_EXT);
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
		poi.name = getPoiNameFromDatabase(poi.id);
		if (poi.type != MAP_GISPOI_TYPE::POITypeNone && poi.id > 0 && poi.name.length() > 0) {
			ret.add(poi);
		}
	}
	return ret;
}

sl_bool Map_GIS_Line_Tile::load(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	_Map_GIS_Shape_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		shapes = loader.shapes;
		return sl_true;
	}
	return sl_false;
}


void Map_GIS_Shape::initShape()
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

#include "gis.h"

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

			if (poi.type != GISPOI_TYPE::NodeTypeNone && poi.id > 0) {
				pois.add(poi);
			}
		}
		return sl_true;
	}
};

class _GIS_Line_Loader
{
public:

	List<GIS_Line> lines;

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
			line.type = (GIS_SHAPE_TYPE)reader.readInt32CVLI();
			line.start.latitude = reader.readDouble();
			line.start.longitude = reader.readDouble();
			line.end.latitude = reader.readDouble();
			line.end.longitude = reader.readDouble();

			if (line.type != GIS_SHAPE_TYPE::ShapeTypeNone && line.id > 0) {
				lines.add(line);
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
	_GIS_Line_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		lines = loader.lines;
		return sl_true;
	}
	return sl_false;
}

SLIB_MAP_NAMESPACE_END

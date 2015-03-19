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

class _GIS_Shape_Loader
{
public:

	Map<GIS_SHAPE_TYPE, Ref<GIS_Shape>> shapes;

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
			GIS_SHAPE_TYPE shapeType = (GIS_SHAPE_TYPE)reader.readInt32CVLI();
			line.start.latitude = reader.readDouble();
			line.start.longitude = reader.readDouble();
			line.end.latitude = reader.readDouble();
			line.end.longitude = reader.readDouble();

			if (shapeType != GIS_SHAPE_TYPE::ShapeTypeNone && line.id > 0) {
				Ref<GIS_Shape> shape = shapes.getValue(shapeType, sl_null);
				if (shape.isNotNull()) {
					shape->lines.add(line);
				}
				else {
					shape = new GIS_Shape;
					shape->type = shapeType;
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
	switch (type) {
	case BoundaryAdmin:
	case BoundaryExtra:
	case BoundaryMapBox:
	case BoundaryProtectedArea:
		clr = Color(Color::White);
		break;
	case HighwayMotor:
	case HighwayTrunk:
		clr = Color::Red;
		break;
	case HighwayPrimary:
	case HighwaySecondary:
		clr = Color::Yellow;
		break;
	case HighwayResidental:
	case HighwayTeritary:
		clr = Color::LightPink;
		break;
	case HighwayExtra:
		clr = Color::LightGreen;
		break;
	case AerialWay:
	case AeroWay:
	case WaterWay:
		clr = Color::LightGray;
		break;
	case FootWay:
	case OneWay:
	case CycleWay:
		clr = Color::Coral;
		break;
	case Railway:
	case Tunnel:
		clr = Color::Brown;
		break;
	default:
		clr = Color::Transparent;
	}
}

SLIB_MAP_NAMESPACE_END

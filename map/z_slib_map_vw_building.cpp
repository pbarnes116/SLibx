#include "vw_building.h"

#include "../../slib/core/io.h"

SLIB_MAP_NAMESPACE_START
class _VM_Building_Loader
{
public:
	class _Object
	{
	public:
		double longitude;
		double latitude;
		float altitude;
		Boxlf bound;

		sl_bool load(Ref<MapDataLoader> data, String type, const MapTileLocation& location, String objectFileName)
		{
			Memory mem = data->loadData(type, location, objectFileName);
			if (mem.isEmpty()) {
				return sl_false;
			}
			MemoryReader reader(mem);
			if (!reader.seek(5)) {
				return sl_false;
			}
			sl_uint8 lenPNU;
			if (!reader.readUint8(&lenPNU)) {
				return sl_false;
			}
			if (!reader.seek(lenPNU)) {
				return sl_false;
			}
			return sl_true;
		}
	};

	List<_Object> objects;

	sl_bool loadFromFile(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
	{
		Memory mem = data->loadData(type, location);
		if (mem.isEmpty()) {
			return sl_false;
		}
		MemoryReader reader(mem);
		sl_uint32 level, x, y;
		if (!reader.readUint32(&level)) {
			return sl_false;
		}
		if (level != location.level) {
			return sl_false;
		}
		if (!reader.readUint32(&x)) {
			return sl_false;
		}
		if (x != (sl_uint32)(location.x)) {
			return sl_false;
		}
		if (!reader.readUint32(&y)) {
			return sl_false;
		}
		if (y != (sl_uint32)(location.y)) {
			return sl_false;
		}
		sl_uint32 countObjects;
		if (!reader.readUint32(&countObjects)) {
			for (sl_uint32 i = 0; i < countObjects; i++) {
				_Object obj;
				sl_uint32 sig;
				if (!reader.readUint32(&sig)) {
					break;
				}
				sl_uint8 sig2;
				if (!reader.readUint8(&sig2)) {
					break;
				}
				sl_uint8 lenPNU;
				if (!reader.readUint8(&lenPNU)) {
					break;
				}
				if (!reader.seek(lenPNU)) {
					break;
				}
				double longitude, latitude;
				float altitude;
				if (!reader.readDouble(&longitude)) {
					break;
				}
				if (!reader.readDouble(&latitude)) {
					break;
				}
				if (!reader.readFloat(&altitude)) {
					break;
				}
				obj.longitude = longitude;
				obj.latitude = latitude;
				obj.altitude = altitude;
				if (!reader.seek(6 * 8)) {
					break;
				}
				if (!reader.seek(1)) {
					break;
				}
				sl_uint8 lenFileName;
				if (!reader.readUint8(&lenFileName)) {
					break;
				}
				char aFileName[256];
				if (!reader.read(aFileName, (sl_uint32)lenFileName + 1)) {
					break;
				}
				if (sig == 0x02000003) {
					if (sig2 == 0x08) {
						aFileName[255] = 0;
						if (obj.load(data, type, location, String::fromUtf8(aFileName))) {
							objects.add(obj);
						}
					}
				}
			}
			return sl_false;
		}
		return sl_true;
	}
};

sl_bool VW_Building::load(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	_VM_Building_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		return sl_true;
	}
	return sl_false;
}

SLIB_MAP_NAMESPACE_END

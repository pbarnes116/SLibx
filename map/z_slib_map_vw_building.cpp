#include "vw_building.h"

#include "../../slib/core/io.h"

SLIB_MAP_NAMESPACE_START
class _VM_Building_Loader
{
public:

	List<VW_Building> buildings;

	sl_bool loadBuilding(VW_Building& building, Ref<MapDataLoader> data)
	{
		if (building.version != 0x03000001 && building.version != 0x03000002) {
			return sl_false;
		}
		Memory mem = data->loadData(building.type, building.location, building.objectFileName);
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
		if (!reader.readDouble(&(building.bound.x1))) {
			return sl_false;
		}
		if (!reader.readDouble(&(building.bound.y1))) {
			return sl_false;
		}
		if (!reader.readDouble(&(building.bound.z1))) {
			return sl_false;
		}
		if (!reader.readDouble(&(building.bound.x2))) {
			return sl_false;
		}
		if (!reader.readDouble(&(building.bound.y2))) {
			return sl_false;
		}
		if (!reader.readDouble(&(building.bound.z2))) {
			return sl_false;
		}
		if (!reader.readFloat(&(building.height))) {
			return sl_false;
		}
		sl_uint8 countMesh = 1;
		if (building.version == 0x03000002) {
			if (!reader.readUint8(&countMesh)) {
				return sl_false;
			}
		}
		for (sl_uint8 iMesh = 0; iMesh < countMesh; iMesh++) {
			VW_Building_Mesh mesh;
			sl_uint32 countVertices;
			if (!reader.readUint32(&countVertices)) {
				return sl_false;
			}
			sl_uint32 i;
			SLIB_SCOPED_ARRAY(RenderProgram3D_PositionNormalTexture_Diffuse::VertexData, vertices, countVertices);
			for (i = 0; i < countVertices; i++) {
				reader.readFloat(&(vertices[i].position.x));
				reader.readFloat(&(vertices[i].position.y));
				reader.readFloat(&(vertices[i].position.z));
				reader.readFloat(&(vertices[i].normal.x));
				reader.readFloat(&(vertices[i].normal.y));
				reader.readFloat(&(vertices[i].normal.z));
				reader.readFloat(&(vertices[i].texCoord.x));
				reader.readFloat(&(vertices[i].texCoord.y));
			}
			mesh.vb = VertexBuffer::create(vertices, sizeof(RenderProgram3D_PositionNormalTexture_Diffuse::VertexData) * countVertices);
			sl_uint32 countIndices;
			if (!reader.readUint32(&countIndices)) {
				return sl_false;
			}
			SLIB_SCOPED_ARRAY(sl_uint16, indices, countIndices);
			for (i = 0; i < countIndices; i++) {
				reader.readUint16(&(indices[i]));
			}
			mesh.ib = IndexBuffer::create(indices, countIndices * 2);
			mesh.countElements = countIndices;
			if (!reader.readUint32(&(mesh.color))) {
				return sl_false;
			}
			if (!reader.readUint8(&(mesh.nTextureLevels))) {
				return sl_false;
			}
			sl_uint8 nTexName = 0;
			if (!reader.readUint8(&nTexName)) {
				return sl_false;
			}
			char bufTexName[256];
			if (reader.read(bufTexName, nTexName) != nTexName) {
				return sl_false;
			}
			mesh.textureFileName = String::fromUtf8(bufTexName, nTexName);
			sl_uint32 nThumbSize;
			if (!reader.readUint32(&nThumbSize)) {
				return sl_false;
			}
			mesh.memThumbnailTexture = reader.readToMemory(nThumbSize);
			if (mesh.memThumbnailTexture.size() != nThumbSize) {
				return sl_false;
			}
			mesh.texture = Texture::create(Image::loadFromMemory(mesh.memThumbnailTexture));
			if (mesh.texture.isNull()) {
				return sl_false;
			}
			building.meshes.add(mesh);
		}
		return sl_true;
	}

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
		if (reader.readUint32(&countObjects)) {
			for (sl_uint32 i = 0; i < countObjects; i++) {
				VW_Building building;
				sl_uint32 version;
				if (!reader.readUint32(&version, sl_true)) {
					break;
				}
				sl_uint8 itype;
				if (!reader.readUint8(&itype)) {
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
				building.longitude = longitude;
				building.latitude = latitude;
				building.altitude = altitude;
				
				// bound
				Boxlf& bound = building.bound;
				if (!reader.readDouble(&(bound.x1))) {
					break;
				}
				if (!reader.readDouble(&(bound.y1))) {
					break;
				}
				if (!reader.readDouble(&(bound.z1))) {
					break;
				}
				if (!reader.readDouble(&(bound.x2))) {
					break;
				}
				if (!reader.readDouble(&(bound.y2))) {
					break;
				}
				if (!reader.readDouble(&(bound.z2))) {
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
				if (reader.read(aFileName, (sl_uint32)lenFileName) != lenFileName) {
					break;
				}
				aFileName[lenFileName] = 0;

				sl_uint8 lenImageFileName;
				if (!reader.readUint8(&lenImageFileName)) {
					break;
				}
				char aImageFileName[256];
				if (reader.read(aImageFileName, (sl_uint32)lenImageFileName) != lenImageFileName) {
					break;
				}
				aImageFileName[lenImageFileName] = 0;

				building.version = version;
				building.type = type;
				building.location = location;
				building.objectFileName = String::fromUtf8(aFileName);
				building.imageFileName = String::fromUtf8(aImageFileName);
				if (loadBuilding(building, data)) {
					buildings.add(building);
				}
			}
		}
		return sl_true;
	}
};

sl_bool VW_Building_Tile::load(Ref<MapDataLoader> data, String type, const MapTileLocation& location)
{
	_VM_Building_Loader loader;
	if (loader.loadFromFile(data, type, location)) {
		buildings = loader.buildings;
		return sl_true;
	}
	return sl_false;
}

SLIB_MAP_NAMESPACE_END

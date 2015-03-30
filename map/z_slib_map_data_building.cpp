#include "data_building.h"

#include "../../slib/core/io.h"

#include "data_config.h"

SLIB_MAP_NAMESPACE_START

Ref<VW_Building_Object> VW_Building::readObject(Ref<MapDataLoader> data, VW_Building_ObjectInfo* info)
{
	Ref<VW_Building_Object> ret;
	if (info->version != 0x03000001 && info->version != 0x03000002) {
		return ret;
	}
	Memory mem = data->loadData(info->dataType, info->location, SLIB_MAP_BUILDING_PACKAGE_DIMENSION, _SLT("/") + info->objectFileName);
	if (mem.isEmpty()) {
		return ret;
	}
	MemoryReader reader(mem);
	sl_uint8 objectType;
	if (!reader.readUint8(&objectType)) {
		return ret;
	}
	sl_uint32 objectNo;
	if (!reader.readUint32(&objectNo)) {
		return ret;
	}
	sl_uint8 lenKey;
	if (!reader.readUint8(&lenKey)) {
		return ret;
	}
	char aKey[256];
	if (reader.read(aKey, lenKey) != lenKey) {
		return ret;
	}
	Boxlf bound;
	if (!reader.readDouble(&(bound.x1))) {
		return ret;
	}
	if (!reader.readDouble(&(bound.y1))) {
		return ret;
	}
	if (!reader.readDouble(&(bound.z1))) {
		return ret;
	}
	if (!reader.readDouble(&(bound.x2))) {
		return ret;
	}
	if (!reader.readDouble(&(bound.y2))) {
		return ret;
	}
	if (!reader.readDouble(&(bound.z2))) {
		return ret;
	}
	float altitude;
	if (!reader.readFloat(&(altitude))) {
		return ret;
	}
	sl_uint8 countMesh = 1;
	if (info->version == 0x03000002) {
		if (!reader.readUint8(&countMesh)) {
			return ret;
		}
	}

	ret = new VW_Building_Object;
	if (ret.isNull()) {
		return ret;
	}

	ret->objectType = objectType;
	ret->objectNo = objectNo;
	ret->key = String::fromUtf8(aKey, lenKey);
	ret->bound.setStart(convertVWPosition(bound.getStart()));
	ret->bound.setEnd(convertVWPosition(bound.getEnd()));
	ret->altitude = altitude;

	for (sl_uint8 iMesh = 0; iMesh < countMesh; iMesh++) {
		VW_Building_Mesh mesh;
		sl_uint32 countVertices;
		if (!reader.readUint32(&countVertices)) {
			return ret;
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
			return ret;
		}
		SLIB_SCOPED_ARRAY(sl_uint16, indices, countIndices);
		for (i = 0; i < countIndices; i++) {
			reader.readUint16(&(indices[i]));
		}
		mesh.ib = IndexBuffer::create(indices, countIndices * 2);
		mesh.countElements = countIndices;
		if (!reader.readUint32(&(mesh.color))) {
			return ret;
		}
		if (!reader.readUint8(&(mesh.nTextureLevels))) {
			return ret;
		}
		sl_uint8 nTexName = 0;
		if (!reader.readUint8(&nTexName)) {
			return ret;
		}
		char bufTexName[256];
		if (reader.read(bufTexName, nTexName) != nTexName) {
			return ret;
		}
		mesh.textureFileName = String::fromUtf8(bufTexName, nTexName);
		sl_uint32 nThumbSize;
		if (!reader.readUint32(&nThumbSize)) {
			return ret;
		}
		Memory memThumbnailTexture = reader.readToMemory(nThumbSize);
		if (memThumbnailTexture.size() != nThumbSize) {
			return ret;
		}
		mesh.textureThumbnail = Texture::loadFromMemory(memThumbnailTexture);
		if (mesh.textureThumbnail.isNull()) {
			return ret;
		}
		ret->meshes.add(mesh);
	}
	return ret;
}

List< Ref<VW_Building_ObjectInfo> > VW_Building::readTile(Ref<MapDataLoader> data, String dataType, const MapTileLocationi& location, sl_bool flagBridge)
{
	List< Ref<VW_Building_ObjectInfo> > ret;

	Memory mem = data->loadData(dataType, location, SLIB_MAP_BUILDING_PACKAGE_DIMENSION, SLIB_MAP_BUILDING_TILE_EXT);
	if (mem.isEmpty()) {
		return ret;
	}

	MemoryReader reader(mem);
	sl_uint32 level, x, y;
	if (!reader.readUint32(&level)) {
		return ret;
	}
	if (level != location.level) {
		return ret;
	}
	if (!reader.readUint32(&x)) {
		return ret;
	}
	if (x != (sl_uint32)(location.x)) {
		return ret;
	}
	if (!reader.readUint32(&y)) {
		return ret;
	}
	if (y != (sl_uint32)(location.y)) {
		return ret;
	}
	sl_uint32 countObjects;
	if (reader.readUint32(&countObjects)) {
		for (sl_uint32 i = 0; i < countObjects; i++) {
			sl_uint32 version;
			if (!reader.readUint32(&version, sl_true)) {
				return ret;
			}
			sl_uint8 objectType;
			if (!reader.readUint8(&objectType)) {
				return ret;
			}
			sl_uint8 lenKey;
			if (!reader.readUint8(&lenKey)) {
				return ret;
			}
			char aKey[256];
			if (reader.read(aKey, (sl_uint32)lenKey) != lenKey) {
				return ret;
			}

			double longitude, latitude;
			float altitude;
			if (!reader.readDouble(&longitude)) {
				return ret;
			}
			if (!reader.readDouble(&latitude)) {
				return ret;
			}
			if (!reader.readFloat(&altitude)) {
				return ret;
			}

			// bound
			Boxlf bound;
			if (!reader.readDouble(&(bound.x1))) {
				return ret;
			}
			if (!reader.readDouble(&(bound.y1))) {
				return ret;
			}
			if (!reader.readDouble(&(bound.z1))) {
				return ret;
			}
			if (!reader.readDouble(&(bound.x2))) {
				return ret;
			}
			if (!reader.readDouble(&(bound.y2))) {
				return ret;
			}
			if (!reader.readDouble(&(bound.z2))) {
				return ret;
			}
			sl_uint8 nTextureLevels;
			if (!reader.readUint8(&nTextureLevels)) {
				return ret;
			}
			sl_uint8 lenFileName;
			if (!reader.readUint8(&lenFileName)) {
				return ret;
			}
			char aFileName[256];
			if (reader.read(aFileName, (sl_uint32)lenFileName) != lenFileName) {
				return ret;
			}

			sl_uint8 lenImageFileName;
			if (!reader.readUint8(&lenImageFileName)) {
				return ret;
			}
			char aImageFileName[256];
			if (reader.read(aImageFileName, (sl_uint32)lenImageFileName) != lenImageFileName) {
				return ret;
			}

			SLIB_NEW_REF(VW_Building_ObjectInfo, building);
			if (building) {
				building->flagBridge = flagBridge;
				building->dataType = dataType;
				building->location = location;

				building->version = version;
				building->objectType = objectType;
				building->key = String::fromUtf8(aKey, lenKey);
				building->longitude = longitude;
				building->latitude = latitude;
				building->altitude = altitude;
				building->bound.setStart(convertVWPosition(bound.getStart()));
				building->bound.setEnd(convertVWPosition(bound.getEnd()));
				building->nTextureLevels = nTextureLevels;

				building->objectFileName = String::fromUtf8(aFileName, lenFileName);
				building->imageFileName = String::fromUtf8(aImageFileName, lenImageFileName);

				ret.add(building);
			}
		}
	}
	return ret;
}

Vector3lf VW_Building::convertVWPosition(const Vector3lf& positionVW)
{
	return Vector3lf(-positionVW.y, positionVW.z, -positionVW.x);
}

Matrix4lf VW_Building::getModelTransformMatrixForMesh(const Vector3lf& positionCenter)
{
	Matrix4lf m;
	m.setRow0(Vector4(0, 0, -1, 0));
	m.setRow1(Vector4(-1, 0, 0, 0));
	m.setRow2(Vector4(0, 1, 0, 0));
	m.setRow3(Vector4(positionCenter, 1));
	return m;
}
SLIB_MAP_NAMESPACE_END

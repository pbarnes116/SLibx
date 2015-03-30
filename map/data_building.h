#ifndef CHECKHEADER_SLIB_MAP_DATA_BUILDING
#define CHECKHEADER_SLIB_MAP_DATA_BUILDING

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START

class VW_Building_Mesh
{
public:
	Ref<VertexBuffer> vb;
	Ref<IndexBuffer> ib;
	sl_uint32 countElements;
	sl_uint32 color;

	String textureFileName;
	sl_uint8 nTextureLevels;

	Ref<Texture> textureThumbnail;
};

class VW_Building_ObjectInfo : public Referable
{
public:
	String dataType;
	MapTileLocationi location;
	sl_bool flagBridge;

	sl_uint32 version;
	sl_uint32 objectType;
	String key;
	double longitude;
	double latitude;
	float altitude;
	Boxlf bound;
	sl_uint32 nTextureLevels;

	String objectFileName;
	String imageFileName;
};

class VW_Building_Object : public Referable
{
public:
	sl_uint32 objectType;
	sl_uint32 objectNo;
	String key;
	Boxlf bound;

	float altitude;

	List<VW_Building_Mesh> meshes;
};

class VW_Building
{
public:
	static List< Ref<VW_Building_ObjectInfo> > readTile(Ref<MapDataLoader> loader, String type, const MapTileLocationi& location, sl_bool flagBridge);
	static Ref<VW_Building_Object> readObject(Ref<MapDataLoader> loader, VW_Building_ObjectInfo* info);

	static Vector3 convertVWPosition(const Vector3& positionVW);
	static Matrix4 getModelTransformMatrixForMesh(const Vector3& positionCenter);
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_DATA_BUILDING
#define CHECKHEADER_SLIB_MAP_DATA_BUILDING

#include "definition.h"
#include "data.h"

#include <slib/render/engine.h>
#include <slib/math/box.h>

SLIB_MAP_NAMESPACE_BEGIN

class SLIB_EXPORT VW_Building_Mesh
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

class SLIB_EXPORT VW_Building_ObjectInfo : public Referable
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

class SLIB_EXPORT VW_Building_Object : public Referable
{
public:
	sl_uint32 objectType;
	sl_uint32 objectNo;
	String key;
	Boxlf bound;

	float altitude;

	List<VW_Building_Mesh> meshes;
};

class SLIB_EXPORT VW_Building
{
public:
	static List< Ref<VW_Building_ObjectInfo> > readTile(Ref<MapDataLoader> loader, String type, const MapTileLocationi& location, sl_bool flagBridge);
	static Ref<VW_Building_Object> readObject(Ref<MapDataLoader> loader, VW_Building_ObjectInfo* info);

	static Vector3lf convertVWPosition(const Vector3lf& positionVW);
	static Matrix4lf getModelTransformMatrixForMesh(const Vector3lf& positionCenter);
};

SLIB_MAP_NAMESPACE_END

#endif

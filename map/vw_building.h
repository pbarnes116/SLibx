#ifndef CHECKHEADER_SLIB_MAP_VW_BUILDING
#define CHECKHEADER_SLIB_MAP_VW_BUILDING

#include "definition.h"

#include "data.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START

struct VW_Building_Mesh
{
	Ref<VertexBuffer> vb;
	Ref<IndexBuffer> ib;
	sl_uint32 countElements;
	sl_uint32 color;
	Ref<Texture> texture;

	String textureFileName;
	sl_uint8 nTextureLevels;
	Memory memThumbnailTexture;
};

struct VW_Building
{
	String type;
	MapTileLocation location;
	String objectFileName;
	String imageFileName;

	sl_uint32 version;
	double longitude;
	double latitude;
	float altitude;
	float height;
	Boxlf bound;

	List<VW_Building_Mesh> meshes;
};

class VW_Building_Tile
{
public:
	List<VW_Building> buildings;
	
	sl_bool load(Ref<MapDataLoader> loader, String type, const MapTileLocation& location);
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_GEARTH_TILE
#define CHECKHEADER_SLIB_MAP_GEARTH_TILE

#include "definition.h"

#include "../../slib/render/engine.h"
#include "../../slib/core/memory.h"

SLIB_MAP_NAMESPACE_START

class GEarthTile
{
public:
	class Mesh
	{
	public:
		Ref<VertexBuffer> vertexBuffer;
		Ref<IndexBuffer> indexBuffer;
		Ref<Texture> texture;
	};
	List<Mesh> meshes;
	Ref<Texture> texture;

public:
	sl_bool load(const void* data, sl_size size);
	SLIB_INLINE sl_bool load(Memory mem)
	{
		return load(mem.getBuf(), mem.getSize());
	}
};

SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_DATA_GEARTH
#define CHECKHEADER_SLIB_MAP_DATA_GEARTH

#include "definition.h"

#include <slib/render/engine.h>
#include <slib/core/memory.h>

SLIB_MAP_NAMESPACE_BEGIN

class SLIB_EXPORT GEarthTile
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
		return load(mem.getData(), mem.getSize());
	}
};

SLIB_MAP_NAMESPACE_END

#endif

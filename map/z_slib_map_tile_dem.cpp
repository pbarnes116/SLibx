#include "tile.h"
#include "dem.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

class _MapTile_DEM_RenderProgram_Tile : public RenderProgram3D
{
public:
	_MapTile_DEM_RenderProgram_Tile() {}
	~_MapTile_DEM_RenderProgram_Tile() {}

	class MapInfo_GLES2 : public Info_GLES2
	{
	public:
		sl_int32 attrAltitude;	// float
	};

public:

	Ref<RenderProgramInfo> create(RenderEngine* engine)
	{
		Ref<RenderProgramInfo> ret;
		RenderEngine::EngineType type = engine->getEngineType();
		if (type == RenderEngine::OPENGL_ES2) {
			ret = new MapInfo_GLES2;
		}
		return ret;
	}

	sl_bool onInit(RenderEngine* engine, RenderProgramInfo* _info)
	{
		RenderProgram3D::onInit(engine, _info);

		RenderEngine::EngineType type = engine->getEngineType();

#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		if (type == RenderEngine::OPENGL_ES2) {
			MapInfo_GLES2* info = (MapInfo_GLES2*)_info;
			sl_uint32 program = info->program_GLES;
			info->attrAltitude = GLES2::getAttributeLocation(program, _SLT("a_Altitude"));
			return sl_true;
		}
#endif
		return sl_false;
	}

	sl_bool onPreRender(RenderEngine* engine, RenderProgramInfo* _info, Primitive* primitive)
	{
		RenderEngine::EngineType type = engine->getEngineType();
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		if (type == RenderEngine::OPENGL_ES2) {
			MapInfo_GLES2* info = (MapInfo_GLES2*)_info;
			SLIB_RENDER_GLES2_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(info->attrPosition, DEM_Vertex, position);
			SLIB_RENDER_GLES2_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(info->attrTexCoord, DEM_Vertex, texCoord);
			SLIB_RENDER_GLES2_SET_VERTEX_FLOAT_ARRAY_ATTRIBUTE(info->attrAltitude, DEM_Vertex, altitude);
			return sl_true;
		}
#endif
		return sl_false;
	}

	void onPostRender(RenderEngine* engine, RenderProgramInfo* _info, Primitive* primitive)
	{
		RenderEngine::EngineType type = engine->getEngineType();
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		if (type == RenderEngine::OPENGL_ES2) {
			MapInfo_GLES2* info = (MapInfo_GLES2*)_info;
			GLES2::disableVertexArrayAttribute(info->attrPosition);
			GLES2::disableVertexArrayAttribute(info->attrTexCoord);
			GLES2::disableVertexArrayAttribute(info->attrAltitude);
		}
#endif
	}
	
	String getVertexShader_GLES2(RenderEngine* engine)
	{
		String source;
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		source = SLIB_STRINGIFY(
			precision mediump float;
			uniform mat4 u_Transform;
			attribute vec3 a_Position;
			attribute vec2 a_TexCoord;
			attribute float a_Altitude;
			varying vec2 v_TexCoord;
			varying float v_Altitude;
			void main() {
				vec4 P = vec4(a_Position, 1.0) * u_Transform;
				gl_Position = P;
				v_TexCoord = a_TexCoord;
				v_Altitude = a_Altitude;
			}
		);
#endif
		return source;
	}

	String getFragmentShader_GLES2(RenderEngine* engine)
	{
		String source;
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		source = SLIB_STRINGIFY(
			precision mediump float;
			uniform sampler2D u_Texture;
			varying vec2 v_TexCoord;
			void main() {
				vec4 colorTexture = texture2D(u_Texture, v_TexCoord);
				gl_FragColor = colorTexture;
			}
		);
#endif
		return source;
	}
};

class _MapTile_DEM_RenderProgram_Tile_TestTextureColor : public _MapTile_DEM_RenderProgram_Tile
{
public:
	_MapTile_DEM_RenderProgram_Tile_TestTextureColor() {}
	~_MapTile_DEM_RenderProgram_Tile_TestTextureColor() {}

	String getFragmentShader_GLES2(RenderEngine* engine)
	{
		String source;
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		source = SLIB_STRINGIFY(
			precision mediump float;
			varying vec2 v_TexCoord;
			void main() {
				gl_FragColor = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
			}
		);
#endif
		return source;
	}
};


class _MapTile_DEM : public MapTile
{
public:
	Ref<DEM> dem;
	Rectangle rectDEM;

	Rectangle rectPicture;
	Ref<Texture> texture;

	_MapTile_DEM()
	{
	}
};

MapTileManager_DEM::MapTileManager_DEM()
{
	m_programTile = new _MapTile_DEM_RenderProgram_Tile;
}

void MapTileManager_DEM::initialize()
{
	MapTileManager::initialize();
	_initializeTopTiles(0, 5, 10);
	setMaximumLevel(15);
}

Ref<MapTile> MapTileManager_DEM::createTile()
{
	return new _MapTile_DEM;
}

sl_bool MapTileManager_DEM::initializeTile(MapTile* _tile, MapTile* _parent)
{
	_MapTile_DEM* tile = (_MapTile_DEM*)_tile;
	_MapTile_DEM* parent = (_MapTile_DEM*)_parent;

	Ref<MapDataLoaderPack> loader = getDataLoader();
	if (loader.isNull()) {
		return sl_false;
	}
	// tile
	{
		MapTileLocation loc;
		loc.level = tile->level;
		loc.x = tile->x;
		loc.y = tile->y;
		Memory mem = loader->picture->loadData(_SLT("picture"), loc);
		Ref<Image> picture;
		if (mem.isNotEmpty()) {
			picture = Image::loadFromMemory(mem);
		}
		if (picture.isNull()) {
			if (parent) {
				tile->texture = parent->texture;
				sl_real hw = parent->rectPicture.getWidth() * 0.5f;
				sl_real hh = parent->rectPicture.getHeight() * 0.5f;
				tile->rectPicture.left = parent->rectPicture.left + hw * tile->ix;
				tile->rectPicture.top = parent->rectPicture.top + hh * (1 - tile->iy);
				tile->rectPicture.right = tile->rectPicture.left + hw;
				tile->rectPicture.bottom = tile->rectPicture.top + hh;
			} else {
				return sl_false;
			}
		} else {
			tile->texture = Texture::create(picture);
			tile->texture->setFreeSourceOnUpdate(sl_true);
			tile->rectPicture = Rectangle(0, 0, 1, 1);
		}
	}
	// dem - dem
	{
		MapTileLocation loc;
		loc.level = tile->level;
		loc.x = tile->x;
		loc.y = tile->y;
		Memory mem = loader->dem->loadData(_SLT("dem"), loc);
		if (mem.isNotEmpty()) {
			Ref<DEM> dem = new DEM;
			if (dem->initializeFromFloatData(65, mem.getBuf(), mem.getSize())) {
				tile->dem = dem;
			}
		}
		if (tile->dem.isNull()) {
			if (parent) {
				tile->dem = parent->dem;
				sl_real hw = parent->rectDEM.getWidth() * 0.5f;
				sl_real hh = parent->rectDEM.getHeight() * 0.5f;
				tile->rectDEM.left = parent->rectDEM.left + hw * tile->ix;
				tile->rectDEM.top = parent->rectDEM.top + hh * (1 - tile->iy);
				tile->rectDEM.right = tile->rectDEM.left + hw;
				tile->rectDEM.bottom = tile->rectDEM.top + hh;
			} else {
				tile->rectDEM = Rectangle(0, 0, 1, 1);
			}
		} else {
			tile->rectDEM = Rectangle(0, 0, 1, 1);
		}
	}
	return sl_true;
}

void MapTileManager_DEM::freeTile(MapTile* _tile)
{
	_MapTile_DEM* tile = (_MapTile_DEM*)_tile;
	tile->dem.setNull();
	tile->texture.setNull();
}

void MapTileManager_DEM::renderTile(MapTile* _tile, RenderEngine* engine, MapEnvironment* environment)
{
	_MapTile_DEM* tile = (_MapTile_DEM*)_tile;
	Ref<Texture> texture = tile->texture;
	Ref<DEM> dem = tile->dem;
	if (texture.isNotNull()) {
		m_programTile->setTexture(texture);
		Primitive primitive;
		if (dem.isNotNull()) {
			dem->makeMesh(primitive, 17, tile->rect, tile->rectDEM, tile->rectPicture);
		} else {
			DEM dem;
			dem.makeMesh(primitive, 17, tile->rect, tile->rectDEM, tile->rectPicture);
		}
		engine->draw(m_programTile, &primitive);
	}
}

void MapTileManager_DEM::renderTiles(RenderEngine* engine, MapEnvironment* environment)
{
	m_programTile->setViewMatrix(environment->transformView);
	m_programTile->setProjectionMatrix(environment->transformProjection);
	MapTileManager::renderTiles(engine, environment);
}
SLIB_MAP_NAMESPACE_END

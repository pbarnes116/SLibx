#include "earth.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

class _MapEarth_RenderProgram_SurfaceTile : public RenderProgram3D
{
public:
	_MapEarth_RenderProgram_SurfaceTile() {}
	~_MapEarth_RenderProgram_SurfaceTile() {}

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
			precision highp float;
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
			precision highp float;
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

class _MapEarth_RenderProgram_SurfaceTile_TestTextureColor : public _MapEarth_RenderProgram_SurfaceTile
{
public:
	_MapEarth_RenderProgram_SurfaceTile_TestTextureColor() {}
	~_MapEarth_RenderProgram_SurfaceTile_TestTextureColor() {}

	String getFragmentShader_GLES2(RenderEngine* engine)
	{
		String source;
#ifdef SLIB_RENDER_SUPPORT_OPENGL_ES2
		source = SLIB_STRINGIFY(
			precision highp float;
		varying vec2 v_TexCoord;
		void main() {
			gl_FragColor = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
		}
		);
#endif
		return source;
	}
};

const SphericalGlobe& MapEarth::getGlobe()
{
	static SphericalGlobe globe(getRadius());
	return globe;
}

MapEarthRenderer::MapEarthRenderer()
{
	m_flagInitialized = sl_false;

	m_nMaxLevel = 15;
	m_nY = 5;
	m_nX = 10;

	initializeTileLifeMillseconds(10000);

	initializeMaxPictureTilesCount(500);
	initializeMaxDEMTilesCount(1000);
	initializeMaxRenderTilesCount(300);

	m_nMaxRenderTileLevel = 0;
}

MapEarthRenderer::~MapEarthRenderer()
{
	release();
}

void MapEarthRenderer::setLevelParamter(sl_uint32 nY, sl_uint32 nX, sl_uint32 nMaxLevel)
{
	m_nY = nY;
	m_nX = nX;
	m_nMaxLevel = nMaxLevel;
}

void MapEarthRenderer::initialize()
{
	m_flagInitialized = sl_true;

	m_programSurfaceTile = new _MapEarth_RenderProgram_SurfaceTile;
	m_programLine = new RenderProgram3D_Position;

	_loadZeroLevelPictureTiles();
	_loadZeroLevelDEMTiles();

	m_threadControl = Thread::start(SLIB_CALLBACK_CLASS(MapEarthRenderer, _runThreadControl, this));
}

void MapEarthRenderer::release()
{
	MutexLocker lock(getLocker());
	m_flagInitialized = sl_false;
	if (m_threadControl.isNotNull()) {
		m_threadControl->finishAndWait();
		m_threadControl.setNull();
	}
}

LatLon MapEarthRenderer::getLatLonFromTileLocation(const MapTileLocationi& location)
{
	LatLon ret;
	sl_uint32 n = 1 << (location.level);
	ret.latitude = location.y * 180.0 / m_nY / n - 90.0;
	ret.longitude = location.x * 360.0 / m_nX / n - 180.0;
	return ret;
}

SLIB_MAP_NAMESPACE_END

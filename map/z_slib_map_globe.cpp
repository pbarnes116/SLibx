
#include "globe.h"
#include "dem.h"

#include "../../slib/render/opengl.h"

SLIB_MAP_NAMESPACE_START

class _MapGlobe_RenderProgram_Tile : public RenderProgram3D
{
public:
	_MapGlobe_RenderProgram_Tile() {}
	~_MapGlobe_RenderProgram_Tile() {}

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

class _MapGlobe_RenderProgram_Tile_TestTextureColor : public _MapGlobe_RenderProgram_Tile
{
public:
	_MapGlobe_RenderProgram_Tile_TestTextureColor() {}
	~_MapGlobe_RenderProgram_Tile_TestTextureColor() {}

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

GlobeRenderer::GlobeRenderer()
{
	m_nMaxLevel = 15;
	m_nY = 5;
	m_nX = 10;
}

GlobeRenderer::~GlobeRenderer()
{
	release();
}

void GlobeRenderer::setLevelParamter(sl_uint32 nY, sl_uint32 nX, sl_uint32 nMaxLevel)
{
	m_nY = nY;
	m_nX = nX;
	m_nMaxLevel = nMaxLevel;

	m_globe.setRadius(Earth::getEquatorialRadius());
}

void GlobeRenderer::release()
{
	if (m_threadResource.isNotNull()) {
		m_threadResource->finish();
	}
	if (m_threadControl.isNotNull()) {
		m_threadControl->finishAndWait();
	}
	if (m_threadResource.isNotNull()) {
		m_threadResource->finishAndWait();
	}

}

void GlobeRenderer::render(RenderEngine* engine, MapEnvironment* environment)
{
	_prepareRendering(engine, environment);

}

void GlobeRenderer::_runThreadControl()
{

}

void GlobeRenderer::_runThreadResource()
{

}

void GlobeRenderer::_prepareRendering(RenderEngine* engine, MapEnvironment* environment)
{
	if (m_engineResource.isNull() || !(m_engineResource->isValid())) {
		m_engineResource = engine->createSharedEngine();
		if (m_engineResource.isNotNull()) {

		} else {
			m_threadResource = Thread::start(SLIB_CALLBACK_CLASS(GlobeRenderer, _runThreadResource, this));
			m_threadControl = Thread::start(SLIB_CALLBACK_CLASS(GlobeRenderer, _runThreadControl, this));
		}
	}
	m_environment = environment;
}

SLIB_MAP_NAMESPACE_END

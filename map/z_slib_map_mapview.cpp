#include "mapview.h"

SLIB_MAP_NAMESPACE_START
MapView::MapView()
{
	m_flagInit = sl_false;
}

MapView::~MapView()
{

}

void MapView::initialize()
{
	if (m_flagInit) {
		return;
	}
	m_flagInit = sl_true;
}

void MapView::onFrame(RenderEngine* engine)
{
	initialize();

	engine->clearColorDepth(Color::black());
	engine->setDepthTest(sl_true);
	engine->setCullFace(sl_false);
	engine->setBlending(sl_false);
	engine->setDepthWriteEnabled(sl_true);

}

SLIB_MAP_NAMESPACE_END

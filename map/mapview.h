#ifndef CHECKHEADER_SLIB_MAP_MAPVIEW
#define CHECKHEADER_SLIB_MAP_MAPVIEW

#include "definition.h"

#include "camera.h"

#include "../../slib/ui/core.h"

SLIB_MAP_NAMESPACE_START
class MapView : public RenderView
{
public:
	MapView();
	~MapView();

public:
	virtual void onFrame(RenderEngine* engine);

protected:
	void initialize();

protected:
	sl_bool m_flagInit;
};
SLIB_MAP_NAMESPACE_END

#endif


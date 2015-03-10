#ifndef CHECKHEADER_SLIB_MAP_ENVIRONMENT
#define CHECKHEADER_SLIB_MAP_ENVIRONMENT

#include "camera.h"
#include "../../slib/ui/view.h"

SLIB_MAP_NAMESPACE_START
class MapEnvironment : public Referable
{
public:
	Ref<MapCamera> cameraViewEarth;
	sl_uint32 viewportWidth;
	sl_uint32 viewportHeight;
	
	Vector3 positionEye;
	Matrix4 transformView;
	Matrix4 transformViewInverse;
	Matrix4 transformProjection;
	Matrix4 transformViewProjection;

	WeakRef<RenderView> view;

public:
	MapEnvironment();

	void updateViewport(sl_uint32 viewportWidth, sl_uint32 viewportHeight);
	void update();
	void requestRender();
};
SLIB_MAP_NAMESPACE_END

#endif

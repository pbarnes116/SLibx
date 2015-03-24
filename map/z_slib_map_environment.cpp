#include "environment.h"
#include "earth.h"

SLIB_MAP_NAMESPACE_START
MapEnvironment::MapEnvironment()
{
	cameraViewEarth = new MapCamera;
	cameraViewEarth->setEyeLocation(GeoLocation(38, 126, 12000000));
}

void MapEnvironment::updateViewport(sl_uint32 _viewportWidth, sl_uint32 _viewportHeight)
{
	viewportWidth = _viewportWidth;
	viewportHeight = _viewportHeight;
	sl_real dist = (sl_real)(cameraViewEarth->getEyeLocation().altitude + 0.1f);
	sl_real zNear = dist / 5;
	sl_real zFar = (sl_real)(dist + MapEarth::getRadius() * 2);
	transformProjection = Transform3::getPerspectiveProjectionFovYMatrix(SLIB_PI / 4, (float)viewportWidth / viewportHeight, zNear, zFar);
}

void MapEnvironment::update()
{
	positionEye = MapEarth::getCartesianPosition(cameraViewEarth->getEyeLocation());
	transformView = cameraViewEarth->getViewMatrix();
	transformViewInverse = transformView.inverse();
	transformViewProjection = transformView * transformProjection;
	viewFrustum = ViewFrustum::fromMVP(transformViewProjection);
}

void MapEnvironment::requestRender()
{
	Ref<RenderView> _view = view.lock();
	if (_view.isNotNull()) {
		_view->requestRender();
	}
}
SLIB_MAP_NAMESPACE_END

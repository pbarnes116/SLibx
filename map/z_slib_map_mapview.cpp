#include "mapview.h"
#include "vw_building_tile.h"

#define STATUS_WIDTH 2048
#define STATUS_HEIGHT 40

SLIB_MAP_NAMESPACE_START
MapView::MapView()
{
	SLIB_REFERABLE_CONSTRUCTOR;

	m_flagInit = sl_false;

	setRenderMode(renderModeWhenDirty);

	m_environment = new MapEnvironment;
	m_environment->view = this;

	m_tileManagerDEM = new MapTileManager_DEM;
	m_tileManagerVWBuilding = new MapTileManager_VWBuilding;

	initializeDataLoader(new MapDataLoaderPack);

}

MapView::~MapView()
{
	release();
}

void MapView::release()
{
	m_tileManagerDEM->release();
	m_tileManagerVWBuilding->release();
}

void MapView::initialize()
{
	m_flagInit = sl_true;

	m_tileManagerDEM->setDataLoader(getDataLoader());
	m_tileManagerDEM->initialize();

	m_tileManagerVWBuilding->setDataLoader(getDataLoader());
	m_tileManagerVWBuilding->initialize();

	m_textureStatus = Texture::create(Image::create(STATUS_WIDTH, STATUS_HEIGHT));
}

void MapView::onFrame(RenderEngine* engine)
{
	if (!m_flagInit) {
		return;
	}
	m_environment->updateViewport(engine->getViewportWidth(), engine->getViewportHeight());
	m_environment->update();

	engine->clearColorDepth(Color::black());
	engine->setDepthTest(sl_true);
	engine->setCullFace(sl_false);
	engine->setBlending(sl_false);
	engine->setDepthWriteEnabled(sl_true);

	m_tileManagerDEM->renderTiles(engine, m_environment);
	m_tileManagerVWBuilding->renderTiles(engine, m_environment);

	// render status
	{
		Ref<FreeType> fontStatus = getStatusFont();
		if (fontStatus.isNotNull()) {
			engine->setDepthTest(sl_false);
			engine->setBlending(sl_true);
			m_textureStatus->getImage()->fillColor(Color(0, 0, 0, 120));

			String textStatus = getStatusText();
			Size size = fontStatus->getStringExtent(textStatus);
			fontStatus->drawString(
				m_textureStatus->getImage(), m_environment->viewportWidth / 2 - (sl_uint32)(size.x / 2)
				, STATUS_HEIGHT / 2 + (sl_uint32)(size.y / 2)
				, textStatus, Color::white());
			m_textureStatus->update();
			engine->drawTexture2D(0, (sl_real)(m_environment->viewportHeight - STATUS_HEIGHT)
				, (sl_real)(m_environment->viewportWidth), (sl_real)(STATUS_HEIGHT)
				, m_textureStatus
				, 0, 0, (sl_real)(m_environment->viewportWidth), (sl_real)(STATUS_HEIGHT));
		}
	}

	engine->drawDebugText();
}

sl_bool MapView::onMouseEvent(MouseEvent& event)
{
	if (!m_flagInit) {
		return sl_false;
	}
	if (event.action == MouseEvent::actionLeftButtonDown || event.action == MouseEvent::actionTouchDown) {
		m_mouseBeforeX = event.x;
		m_mouseBeforeY = event.y;
		if (event.points.count() == 2) {
			m_flagTouch2 = sl_true;
			m_mouseBefore2X = event.points[1].x - event.x;
			m_mouseBefore2Y = event.points[1].y - event.y;
		} else {
			m_flagTouch2 = sl_false;
		}
		requestRender();
		setFocus();
	} else if (event.action == MouseEvent::actionLeftButtonDrag || event.action == MouseEvent::actionTouchMove) {
		/*
		sl_real dx = event.x - m_mouseDownX;
		sl_real dy = event.y - m_mouseDownY;
		sl_real rx = dx / 100.0f;
		sl_real ry = dy / 100.0f;
		Matrix4 matInv = (m_lastEarthMatrix * m_environment->cameraViewEarth.getViewMatrix()).inverse();
		Matrix4 mat = Transform3D::getRotationMatrix(Transform3D::getTransformedAxisX(matInv), ry) * m_lastEarthMatrix;
		mat.multiply(Transform3D::getRotationMatrix(Vector3::axisY(), rx));
		m_environment->setEarthTransform(mat);
		*/
		if (!m_flagTouch2) {
			sl_real dx = event.x - m_mouseBeforeX;
			sl_real dy = event.y - m_mouseBeforeY;
			Sphere earth(Transform3::getTransformedOrigin(m_environment->transformView), (sl_real)(Earth::getAverageRadius()));
			Line3 dirScreenO = Transform3::unprojectScreenPoint(m_environment->transformProjection, 0, 0, this->getClientRectangle());
			Line3 dirScreenX = Transform3::unprojectScreenPoint(m_environment->transformProjection, 1, 0, this->getClientRectangle());
			Line3 dirScreenY = Transform3::unprojectScreenPoint(m_environment->transformProjection, 0, 1, this->getClientRectangle());
			sl_real lx = (dirScreenO.getDirection().getNormalized() - dirScreenX.getDirection().getNormalized()).getLength();
			sl_real ly = (dirScreenO.getDirection().getNormalized() - dirScreenY.getDirection().getNormalized()).getLength();
			sl_real h = (sl_real)(m_environment->cameraViewEarth->getEyeLocation().altitude);
			sl_real f = (sl_real)(Math::getDegreeFromRadian(h * 1.5f) / Earth::getAverageRadius());
			m_environment->cameraViewEarth->move(
				dy * ly * f
				, -dx * lx * f
				, 0);
		}
		m_mouseBeforeX = event.x;
		m_mouseBeforeY = event.y;
		if (event.points.count() == 2) {
			sl_real dx2 = event.points[1].x - event.x;
			sl_real dy2 = event.points[1].y - event.y;
			if (m_flagTouch2) {
				Vector2 v1(m_mouseBefore2X, m_mouseBefore2Y);
				Vector2 v2(dx2, dy2);
				if (v1.length() > 10 && v2.length() > 10) {
					m_environment->cameraViewEarth->zoom(Math::square(v1.length() / v2.length()));
				}
			}
			m_mouseBefore2X = dx2;
			m_mouseBefore2Y = dy2;
			m_flagTouch2 = sl_true;
		} else {
			m_flagTouch2 = sl_false;
		}
		requestRender();
	} else if (event.action == MouseEvent::actionRightButtonDown) {
		m_mouseBeforeRightX = event.x;
		m_mouseBeforeRightY = event.y;
		requestRender();
		setFocus();
	} else if (event.action == MouseEvent::actionRightButtonDrag) {
		//sl_real dx = event.x - m_mouseBeforeRightX;
		sl_real dy = event.y - m_mouseBeforeRightY;

		sl_real t = m_environment->cameraViewEarth->getTilt();
		t += dy;
		if (t < 0) {
			t = 0;
		}
		if (t > 80) {
			t = 80;
		}
		m_environment->cameraViewEarth->setTilt(t);

		m_mouseBeforeRightX = event.x;
		m_mouseBeforeRightY = event.y;

		requestRender();
	} else if (event.action == MouseEvent::actionLeftButtonUp
		|| event.action == MouseEvent::actionRightButtonUp
		|| event.action == MouseEvent::actionTouchUp) {
		requestRender();
	}
	return sl_true;
}

sl_bool MapView::onMouseWheelEvent(MouseWheelEvent& event)
{
	if (!m_flagInit) {
		return sl_false;
	}
	if (event.delta > 0) {
		m_environment->cameraViewEarth->zoom(0.8f);
	} else {
		m_environment->cameraViewEarth->zoom(1.25f);
	}
	requestRender();
	return sl_true;
}

String MapView::formatLatitude(double f)
{
	String ret;
	if (f > 0) {
		ret = _SLT("N");
	} else {
		ret = _SLT("S");
		f = -f;
	}
	ret += (sl_int32)(Math::floor(f)) + String(_SLT("°"));
	ret += String::fromInt32((sl_int32)(Math::floor(f * 60)) % 60, 10, 2) + _SLT("ʹ");
	ret += String::fromDouble((sl_real)((sl_int32)(Math::floor(f * 360000)) % 6000) / 100.0, 2, sl_true, 2) + _SLT("ʺ");
	return ret;
}

String MapView::formatLongitude(double f)
{
	String ret;
	if (f > 0) {
		ret = _SLT("E");
	} else {
		ret = _SLT("W");
		f = -f;
	}
	ret += (sl_int32)(Math::floor(f)) + String(_SLT("°"));
	ret += String::fromInt32((sl_int32)(Math::floor(f * 60)) % 60, 10, 2) + _SLT("ʹ");
	ret += String::fromDouble((sl_real)((sl_int32)(Math::floor(f * 360000)) % 6000) / 100.0, 2, sl_true, 2) + _SLT("ʺ");
	return ret;
}

String MapView::formatAltitude(double f)
{
	String ret;
	if (f < 0) {
		f = -f;
		ret += "-";
	}
	if (f >= 1000) {
		ret += (sl_uint32)(f / 1000);
		ret += _SLT("km");
	} else {
		ret += (sl_uint32)(f);
		ret += _SLT("m");
	}
	return ret;
}

String MapView::getStatusText()
{
	GeoLocation loc = m_environment->cameraViewEarth->getEyeLocation();
	String status = formatLatitude(loc.latitude) + _SLT(", ") + formatLongitude(loc.longitude) + _SLT(", ") + formatAltitude(loc.altitude);
	return status;
}

SLIB_MAP_NAMESPACE_END

#include "mapview.h"

#define STATUS_WIDTH 1024
#define STATUS_HEIGHT 30

SLIB_MAP_NAMESPACE_START

MapView::MapView()
{
	SLIB_REFERABLE_CONSTRUCTOR;

	m_flagInit = sl_false;

	//setRenderMode(renderModeWhenDirty);

	setDataLoader(new MapDataLoaderList);

	m_viewportWidth = 1;
	m_viewportHeight = 1;
}

MapView::~MapView()
{
	release();
}

void MapView::release()
{
	m_earthRenderer.release();
}

void MapView::setFontForPOI(Ref<FreeType> font)
{
	m_earthRenderer.setFontForPOI(font);
}

void MapView::setWayNames(Map<sl_int64, String> wayNames)
{
	m_earthRenderer.setWayNames(wayNames);
}

Ref<MapDEMTileManager> MapView::getDEMTiles()
{
	return m_earthRenderer.getDEMTiles();
}

void MapView::setPoiInformation(Map<sl_int64, Variant> poiInformation)
{
	m_earthRenderer.setPoiInformation(poiInformation);
}

void MapView::initialize()
{
	m_flagInit = sl_true;

	m_earthRenderer.setDataLoader(getDataLoader());
	m_earthRenderer.initialize();

	m_textureStatus = Texture::create(Image::create(STATUS_WIDTH, STATUS_HEIGHT));

}

void MapView::onFrame(RenderEngine* engine)
{
	sl_int32 viewportWidth = engine->getViewportWidth();
	sl_int32 viewportHeight = engine->getViewportHeight();
	m_viewportWidth = (sl_real)viewportWidth;
	m_viewportHeight = (sl_real)viewportHeight;

	if (!m_flagInit) {
		return;
	}
	if (viewportWidth == 0 || viewportHeight == 0) {
		return;
	}

	engine->clearColorDepth(Color::black());
	engine->setDepthTest(sl_true);
	engine->setCullFace(sl_false);
	engine->setBlending(sl_false);
	engine->setDepthWriteEnabled(sl_true);

	m_earthRenderer.render(engine);

	// render status
	{
		Ref<FreeType> fontStatus = getStatusFont();
		if (fontStatus.isNotNull()) {
			engine->setDepthTest(sl_false);
			engine->setBlending(sl_true);
			m_textureStatus->getImage()->fillColor(Color(0, 0, 0, 50));

			String textStatus = getStatusText();
			Size size = fontStatus->getStringExtent(textStatus);
			fontStatus->drawString(
				m_textureStatus->getImage(), STATUS_WIDTH / 2 - (sl_uint32)(size.x / 2)
				, STATUS_HEIGHT / 2 + (sl_uint32)(size.y / 2)
				, textStatus, Color::white());
			m_textureStatus->update();
			sl_real h = (sl_real)(STATUS_HEIGHT * viewportWidth / STATUS_WIDTH);
			engine->drawTexture2D(
				engine->screenToViewport(0, (sl_real)(viewportHeight - h), (sl_real)(viewportWidth), (sl_real)(h))
				, m_textureStatus
				, Rectangle(0, 0, 1, 1));
		}
	}

}

sl_bool MapView::onMouseEvent(MouseEvent& event)
{
	if (!m_flagInit) {
		return sl_false;
	}

	Ref<MapCamera> camera = getCamera();
	if (camera.isNull()) {
		return sl_false;
	}

	if (event.action == MouseEvent::actionLeftButtonDown || event.action == MouseEvent::actionTouchDown) {
		
		m_mouseBeforeX = event.x;
		m_mouseBeforeY = event.y;
		m_timeMouseBefore = Time::now();

		m_locationDown = getCamera()->getEyeLocation().getLatLon();
		m_mouseDownX = event.x;
		m_mouseDownY = event.y;
		m_timeMouseDown = Time::now();
		m_transformDown = getCamera()->getVerticalViewMatrix();

		if (event.points.count() == 2) {
			m_flagTouch2 = sl_true;
			m_mouseBefore2X = event.points[1].x - event.x;
			m_mouseBefore2Y = event.points[1].y - event.y;
		} else {
			m_flagTouch2 = sl_false;
		}

		setFocus();

	} else if (
		event.action == MouseEvent::actionLeftButtonDrag
		|| event.action == MouseEvent::actionTouchMove
		|| event.action == MouseEvent::actionLeftButtonUp
		|| event.action == MouseEvent::actionTouchUp
		) {

		double dx = (event.x - m_mouseDownX) / m_viewportWidth;
		double dy = -(event.y - m_mouseDownY) / m_viewportHeight;
		double dt = (double)((Time::now() - m_timeMouseDown).getMillisecondsCount());

		sl_real time = 500;
		if (event.action == MouseEvent::actionLeftButtonUp
			|| event.action == MouseEvent::actionTouchUp) {
			if (dt < 400) {
				time = 4000;
				dx *= 8;
				dy *= 8;
			}
			if (dt < 100 && dx * dx + dy * dy < 0.01 ) {
				time = 0;
			}
		}

		GeoLocation locEye = camera->getEyeLocation();
		double alt = locEye.altitude;
		Vector3lf posViewSurface(-dx * alt, -dy * alt, alt);
		Vector3 pos = m_transformDown.inverse().transformPosition(posViewSurface);
		GeoLocation locTarget = MapEarth::getGeoLocation(pos);
		if (time > 0) {
			camera->startMoving(GeoLocation(locTarget.getLatLon(), alt), time);
		} else {
			camera->stopMoving();
		}

		m_mouseBeforeX = event.x;
		m_mouseBeforeY = event.y;
		m_timeMouseBefore = Time::now();
		/*
		sl_real dx = event.x - m_mouseDownX;
		sl_real dy = event.y - m_mouseDownY;
		sl_real rx = dx / 100.0f;
		sl_real ry = dy / 100.0f;
		Matrix4 matInv = (m_lastEarthMatrix * m_environment->cameraViewEarth.getViewMatrix()).inverse();
		Matrix4 mat = Transform3D::getRotationMatrix(Transform3D::getTransformedAxisX(matInv), ry) * m_lastEarthMatrix;
		mat.multiply(Transform3D::getRotationMatrix(Vector3::axisY(), rx));
		m_environment->setEarthTransform(mat);
		
		if (!m_flagTouch2) {
			sl_real dx = event.x - m_mouseBeforeX;
			sl_real dy = event.y - m_mouseBeforeY;
			Sphere earth(Transform3::getTransformedOrigin(m_environment->transformView), (sl_real)(MapEarth::getRadius()));
			Line3 dirScreenO = Transform3::unprojectScreenPoint(m_environment->transformProjection, Point(0, 0), this->getClientRectangle());
			Line3 dirScreenX = Transform3::unprojectScreenPoint(m_environment->transformProjection, Point(1, 0), this->getClientRectangle());
			Line3 dirScreenY = Transform3::unprojectScreenPoint(m_environment->transformProjection, Point(0, 1), this->getClientRectangle());
			sl_real lx = (dirScreenO.getDirection().getNormalized() - dirScreenX.getDirection().getNormalized()).getLength();
			sl_real ly = (dirScreenO.getDirection().getNormalized() - dirScreenY.getDirection().getNormalized()).getLength();
			sl_real h = (sl_real)(m_environment->cameraViewEarth->getEyeLocation().altitude);
			sl_real f = (sl_real)(Math::getDegreeFromRadian(h * 1.5f) / MapEarth::getRadius());
			m_environment->cameraViewEarth->setEyeLocation(GeoLocation(
				le.latitude + dy * ly * f
				, le.longitude -dx * lx * f
				, le.altitude));
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
					zoom(Math::square(v1.length() / v2.length()));
				}
			}
			m_mouseBefore2X = dx2;
			m_mouseBefore2Y = dy2;
			m_flagTouch2 = sl_true;
		} else {
			m_flagTouch2 = sl_false;
		}*/
		requestRender();

	} else if (event.action == MouseEvent::actionRightButtonDown) {
		
		m_mouseBeforeRightX = event.x;
		m_mouseBeforeRightY = event.y;

		setFocus();

	} else if (event.action == MouseEvent::actionRightButtonDrag) {

		sl_real dx = event.x - m_mouseBeforeRightX;
		sl_real dy = event.y - m_mouseBeforeRightY;

		sl_real r = getCamera()->getRotationZ();
		r -= dx;
		getCamera()->startRotatingZ(r);

		sl_real t = getCamera()->getTilt();
		t += dy;
		if (t < 0) {
			t = 0;
		}
		if (t > 80) {
			t = 80;
		}
		getCamera()->startTilting(t);

		m_mouseBeforeRightX = event.x;
		m_mouseBeforeRightY = event.y;

	}
	return sl_true;
}

sl_bool MapView::onMouseWheelEvent(MouseWheelEvent& event)
{
	if (!m_flagInit) {
		return sl_false;
	}
	if (event.delta > 0) {
		_zoom(0.8f);
	} else {
		_zoom(1.25f);
	}
	requestRender();
	return sl_true;
}

void MapView::_zoom(double ratio)
{
	double _min = 20;
	double _max = 12000000;
	if (ratio > 0) {
		GeoLocation le = getCamera()->getEyeLocation();
		double h = le.altitude * ratio;
		if (h < _min) {
			h = _min;
		}
		if (h > _max) {
			h = _max;
		}
		le.altitude = h;
		getCamera()->setEyeLocation(le);
	}
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
	GeoLocation loc = getCamera()->getEyeLocation();
	String status = formatLatitude(loc.latitude) + _SLT(", ") + formatLongitude(loc.longitude) + _SLT(", ") + formatAltitude(loc.altitude);
	return status;
}

Ref<MapMarker> MapView::getMarker(String key)
{
	return m_earthRenderer.markers.getValue(key, Ref<MapMarker>::null());
}

void MapView::putMarker(String key, Ref<MapMarker> marker)
{
	m_earthRenderer.markers.put(key, marker);
}

void MapView::removeMarker(String key)
{
	m_earthRenderer.markers.remove(key);
}

Ref<MapPolygon> MapView::getPolygon(String key)
{
	return m_earthRenderer.polygons.getValue(key, Ref<MapPolygon>::null());
}

void MapView::putPolygon(String key, Ref<MapPolygon> polygon)
{
	m_earthRenderer.polygons.put(key, polygon);
}

void MapView::removePolygon(String key)
{
	m_earthRenderer.polygons.remove(key);
}

SLIB_MAP_NAMESPACE_END

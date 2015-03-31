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

	m_flagTouchBefore2 = sl_false;
	m_flagMouseExitMoving = sl_false;
	m_flagMouseDown = sl_false;

	m_flagCompassHighlight = sl_false;
	setCompassSize(150);
	setCompassPosition(Point(75, 75));
}

MapView::~MapView()
{
	release();
}

void MapView::release()
{
	m_earthRenderer.release();
}

Ref<FreeType> MapView::getFontForPOI()
{
	return m_earthRenderer.getFontForPOI();
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
			m_textureStatus->getImage()->fillColor(Color(0, 0, 0, 100));

			String textStatus = getStatusText();
			Size size = fontStatus->getStringExtent(textStatus);
			fontStatus->drawString(
				m_textureStatus->getImage(), STATUS_WIDTH / 2 - (sl_uint32)(size.x / 2)
				, STATUS_HEIGHT / 2 + (sl_uint32)(size.y / 2)
				, textStatus, Color::white());
			m_textureStatus->update();
			
			sl_real heightStatus = (sl_real)(STATUS_HEIGHT * viewportWidth / STATUS_WIDTH);
			engine->drawTexture2D(
				engine->screenToViewport(0, (sl_real)(viewportHeight - heightStatus), (sl_real)(viewportWidth), (sl_real)(heightStatus))
				, m_textureStatus
				, Rectangle(0, 0, 1, 1));
		}
	}

	// render compass
	{
		Ref<Texture> texture;
		Rectangle rect;
		if (m_flagCompassHighlight) {
			texture = getCompassHighlightTexture();
			rect = getCompassHighlightTextureRectangle();
		}
		if (texture.isNull()) {
			texture = getCompassTexture();
			rect = getCompassTextureRectangle();
		}
		if (texture.isNotNull()) {
			sl_real sizeCompass = getCompassSize();
			Matrix3 transform = Transform2::getTranslationMatrix(-0.5f, -0.5f)
				* Transform2::getScalingMatrix(sizeCompass, sizeCompass)
				* Transform2::getRotationMatrix(-Math::getRadianFromDegree(getCamera()->getRotationZ()))
				* Transform2::getTranslationMatrix(getCompassPosition())
				* Transform2::getScalingMatrix(2.0f / viewportWidth, - 2.0f / viewportHeight)
				* Transform2::getTranslationMatrix(-1, 1);
			engine->drawTexture2D(transform, texture, rect);
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

	Point pt(event.x, event.y);
	Point pt2;
	sl_bool flagTouch2 = sl_false;
	if (event.points.count() >= 2) {
		pt2.x = event.points[1].x;
		pt2.y = event.points[1].y;
		flagTouch2 = sl_true;
	}

	if (event.action == MouseEvent::actionLeftButtonDown || event.action == MouseEvent::actionTouchDown) {
		
		sl_real lenCompass = (pt - getCompassPosition()).getLength();
		if (lenCompass < getCompassSize() / 2) {
			if (lenCompass > getCompassSize() / 5) {
				m_flagCompassHighlight = sl_true;
			} else {
				getCamera()->startRotatingZ(0);
				m_flagMouseDown = sl_false;
				return sl_false;
			}
		} else {
			m_flagCompassHighlight = sl_false;
		}
		m_pointMouseDown = pt;
		m_timeMouseDown = Time::now();
		m_locationMouseDown = getCamera()->getEyeLocation().getLatLon();
		m_transformMouseDown = getCamera()->getVerticalViewMatrix();
		m_flagMouseExitMoving = sl_false;
		m_flagMouseDown = sl_true;

		setFocus();

	} else if (
		event.action == MouseEvent::actionLeftButtonDrag
		|| event.action == MouseEvent::actionTouchMove
		|| event.action == MouseEvent::actionLeftButtonUp
		|| event.action == MouseEvent::actionTouchUp
		) {

		if (m_flagMouseDown) {
			if (m_flagCompassHighlight) {

				Vector2 v = pt - getCompassPosition();
				if (v.length2p() > 30) {
					sl_real r = -Math::getDegreeFromRadian(Transform2::getRotationAngleFromDirToDir(Vector2(0, -1), v));
					getCamera()->startRotatingZ(r);
				}

			} else {

				if (!flagTouch2 && !m_flagMouseExitMoving) {

					double dx = (pt.x - m_pointMouseDown.x);
					double dy = -(pt.y - m_pointMouseDown.y);
					if (dx * dx + dy * dy < 20) {
						camera->stopMoving();
					} else {
						dx /= m_viewportWidth;
						dy /= m_viewportHeight;
						double dt = (double)((Time::now() - m_timeMouseDown).getMillisecondsCount());

						sl_real time = 500;
						if (event.action == MouseEvent::actionLeftButtonUp
							|| event.action == MouseEvent::actionTouchUp) {
							if (dt < 400) {
								time = 4000;
								dx *= 8;
								dy *= 8;
							}
						}
						GeoLocation locEye = camera->getEyeLocation();
						double alt = locEye.altitude;
						Vector3lf posViewSurface(-dx * alt, -dy * alt, alt);
						Vector3 pos = m_transformMouseDown.inverse().transformPosition(posViewSurface);
						GeoLocation locTarget = MapEarth::getGeoLocation(pos);
						camera->startMoving(GeoLocation(locTarget.getLatLon(), alt), time);

					}
				}

				if (flagTouch2) {
					m_flagMouseExitMoving = sl_true;
					if (flagTouch2 && m_flagTouchBefore2) {

						Vector2 v1 = m_pointMouseBefore2 - m_pointMouseBefore;
						Vector2 v2 = pt2 - pt;

						sl_real len1 = v1.getLength();
						sl_real len2 = v2.getLength();

						if (len1 > 10 && len2 > 10) {

							_zoom(len1 / len2);

							sl_real a = Math::getDegreeFromRadian(Transform2::getRotationAngleFromDirToDir(v1, v2));

							sl_real r = getCamera()->getRotationZ();
							r -= a;
							getCamera()->startRotatingZ(r);
						}
					}
				}
			}
		}

		if (event.action == MouseEvent::actionLeftButtonUp
			|| event.action == MouseEvent::actionTouchUp) {

			m_flagCompassHighlight = sl_false;
			m_flagMouseDown = sl_false;
		}

	} else if (event.action == MouseEvent::actionRightButtonDown) {
		
		setFocus();

	} else if (event.action == MouseEvent::actionRightButtonDrag) {

		sl_real dx = pt.x - m_pointMouseBefore.x;
		sl_real dy = pt.y - m_pointMouseBefore.y;

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

	}

	m_pointMouseBefore = pt;
	m_pointMouseBefore2 = pt2;
	m_flagTouchBefore2 = flagTouch2;

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
		getCamera()->stopMoving();
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

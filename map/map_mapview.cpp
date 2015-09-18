#include "../../../inc/slibx/map/mapview.h"
#include "../../../inc/slibx/map/data.h"

#include "map_earth_renderer.h"

#include "../../../inc/slib/core/log.h"

#define STATUS_WIDTH 1024
#define STATUS_HEIGHT 30

SLIB_MAP_NAMESPACE_BEGIN

MapView::MapView()
{
	SLIB_REFERABLE_CONSTRUCTOR;

	m_flagInit = sl_false;

	m_earthRenderer = new MapEarthRenderer;

	//setRenderMode(renderModeWhenDirty);

	setDataLoader(new MapDataLoaderList);

	m_viewportWidth = 1;
	m_viewportHeight = 1;

	m_flagTouchBefore2 = sl_false;
	m_flagMouseExitMoving = sl_false;
	m_flagMouseDown = sl_false;

	m_flagCompassHighlight = sl_false;
	m_compassMouseDown = 0;

	setCompassSize(150);
	setCompassPosition(Point(75, 75));
	setCompassVisible(sl_true);
	setCompassAlignRight(sl_true);
	setCompassAlignBottom(sl_false);
	
	setStatusBarVisible(sl_true);
	setStatusBarLocateAtTop(sl_false);
}

MapView::~MapView()
{
	release();
}

void MapView::release()
{
	m_earthRenderer->release();
}

Ref<MapCamera> MapView::getCamera()
{
	return m_earthRenderer->getCamera();
}

Ref<FreeType> MapView::getFontForPOI()
{
	return m_earthRenderer->getFontForPOI();
}

void MapView::setFontForPOI(Ref<FreeType> font)
{
	m_earthRenderer->setFontForPOI(font);
}

void MapView::setViewportSize(const Sizef& size)
{
	m_earthRenderer->setViewportSize(size);
}

void MapView::setWayNames(Map<sl_int64, String> wayNames)
{
	m_earthRenderer->setWayNames(wayNames);
}

void MapView::setPoiInformation(Map<sl_int64, MapGISPoiInfo> poiInformation)
{
	m_earthRenderer->setPoiInformation(poiInformation);
}

void MapView::initialize()
{
	m_flagInit = sl_true;

	m_earthRenderer->setDataLoader(getDataLoader());
	m_earthRenderer->initialize();

	m_textureStatus = Texture::create(Image::create(STATUS_WIDTH, STATUS_HEIGHT));

}

void MapView::onFrame(RenderEngine* engine)
{
	sl_int32 viewportWidth = engine->getViewportWidth();
	sl_int32 viewportHeight = engine->getViewportHeight();
	if (viewportWidth == 0 || viewportHeight == 0) {
		return;
	}

	m_viewportWidth = (sl_real)viewportWidth;
	m_viewportHeight = (sl_real)viewportHeight;

	if (!m_flagInit) {
		return;
	}
	
	engine->clearColorDepth(Color::black());
	engine->setDepthTest(sl_true);
	engine->setCullFace(sl_false);
	engine->setBlending(sl_false);
	engine->setDepthWriteEnabled(sl_true);

	m_earthRenderer->render(engine);

	// render status
	if (isStatusBarVisible()) {
		Ref<FreeType> fontStatus = getStatusFont();
		if (fontStatus.isNotNull()) {
			engine->setDepthTest(sl_false);
			engine->setBlending(sl_true);
			m_textureStatus->getImage()->fillColor(Color(0, 0, 0, 0));

			String textStatus = getStatusText();
			Size size = fontStatus->getStringExtent(textStatus);
			fontStatus->drawString(
				m_textureStatus->getImage(), STATUS_WIDTH / 2 - (sl_uint32)(size.x / 2)
				, STATUS_HEIGHT / 2 + (sl_uint32)(size.y / 2)
				, textStatus, Color::white());
			m_textureStatus->update();
			
			sl_real heightStatus = getStatusBarHeight();
			sl_real widthStatus = (sl_real)(heightStatus * STATUS_WIDTH / STATUS_HEIGHT);
			
			sl_real top;
			if (isStatusBarLocateAtTop()) {
				top = 0;
			} else {
				top = (sl_real)(viewportHeight - heightStatus);
			}
			engine->drawRectangle2D(engine->screenToViewport(0, top, viewportWidth, heightStatus), Color(0, 0, 0, 100));
			engine->drawTexture2D(
				engine->screenToViewport((sl_real)(viewportWidth / 2 - widthStatus / 2), top, widthStatus, heightStatus)
				, m_textureStatus
				, Rectangle(0, 0, 1, 1));
		}
	}

	// render compass
	if (isCompassVisible()) {
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
			Point pt = getCompassRealPosition();
			sl_real sizeCompass = getCompassSize();
			Matrix3 transform = Transform2::getTranslationMatrix(-0.5f, -0.5f)
				* Transform2::getScalingMatrix(sizeCompass, sizeCompass)
				* Transform2::getRotationMatrix(-Math::getRadianFromDegrees(getCamera()->getRotationZ()))
				* Transform2::getTranslationMatrix(pt)
				* Transform2::getScalingMatrix(2.0f / viewportWidth, - 2.0f / viewportHeight)
				* Transform2::getTranslationMatrix(-1, 1);
			engine->setDepthTest(sl_false);
			engine->setBlending(sl_true);
			engine->drawTexture2D(transform, texture, rect);
		}
	}

}

void MapView::onMouseEvent(UIEvent* event)
{
	if (!m_flagInit) {
		return;
	}

	Ref<MapCamera> camera = getCamera();
	if (camera.isNull()) {
		return;
	}

	Point pt = event->getPoint();
	Point pt2;
	sl_bool flagTouch2 = sl_false;
	if (event->getTouchPointsCount() >= 2) {
		pt2 = event->getTouchPoint(1).point;
		flagTouch2 = sl_true;
	}

	UIEvent::Action action = event->getAction();
	if (action == UIEvent::actionLeftButtonDown || action == UIEvent::actionTouchBegin) {

		getCamera()->clearMotions();

		sl_real lenCompass = (pt - getCompassRealPosition()).getLength();
		if (isCompassVisible() && lenCompass < getCompassSize() * 0.5f && lenCompass > getCompassSize() / 8) {
			m_flagCompassHighlight = sl_true;
			m_compassMouseDown = getCamera()->getRotationZ();
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
		action == UIEvent::actionLeftButtonDrag
		|| action == UIEvent::actionTouchMove
		|| action == UIEvent::actionLeftButtonUp
		|| action == UIEvent::actionTouchEnd
		|| action == UIEvent::actionTouchCancel
		) {

		if (m_flagMouseDown) {
			if (m_flagCompassHighlight) {

				Vector2 v = pt - getCompassRealPosition();
				if (v.length2p() > 30) {
					sl_real r = -Math::getDegreesFromRadian(Transform2::getRotationAngleFromDirToDir(Vector2(0, -1), v));
					getCamera()->setTargetRotationZ(r);
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

						sl_real time = 100;
						dx *= 1.5;
						dy *= 1.5;
						if (action == UIEvent::actionLeftButtonUp
							|| action == UIEvent::actionTouchEnd
							|| action == UIEvent::actionTouchCancel) {
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
					if (!m_flagTouchBefore2) {
						getCamera()->clearMotions();
						m_rotateTouchStart = getCamera()->getRotationZ();
						m_zoomTouchStart = getCamera()->getEyeLocation().altitude;
						m_pointTouchStart1 = pt;
						m_pointTouchStart2 = pt2;
						m_flagTouchRotateStarted = sl_false;
					} else {

						Vector2 v1 = m_pointTouchStart2 - m_pointTouchStart1;
						Vector2 v2 = pt2 - pt;

						sl_real len1 = v1.getLength();
						sl_real len2 = v2.getLength();

						if (len1 > 10 && len2 > 10) {

							sl_real a = Math::getDegreesFromRadian(Transform2::getRotationAngleFromDirToDir(v1, v2));
							sl_real r = m_rotateTouchStart;
							sl_real d = Math::abs(Math::normalizeDegreeDistance(a));
							if (d > 10 || m_flagTouchRotateStarted) {
								r -= a;
								getCamera()->setTargetRotationZ(r);
								m_flagTouchRotateStarted = sl_true;
							} else {
								if (len1 > len2) {
									_zoomTo(m_zoomTouchStart * len1 / len2 * 1.4);
								}

								if (len1 < len2) {
									_zoomTo(m_zoomTouchStart * len1 / len2 / 1.4);
								}
							}
						}
					}
				}
			}
		}

		if (action == UIEvent::actionLeftButtonUp
			|| action == UIEvent::actionTouchEnd
			|| action == UIEvent::actionTouchCancel) {

			if (m_flagCompassHighlight) {

				double dx = (pt.x - m_pointMouseDown.x);
				double dy = -(pt.y - m_pointMouseDown.y);
				if (dx * dx + dy * dy < 20) {

					double dt = (double)((Time::now() - m_timeMouseDown).getMillisecondsCount());
					if (dt < 300) {
						if (Math::abs(Math::normalizeDegreeDistance(getCamera()->getTargetRotationZ() - m_compassMouseDown)) < 20) {
							getCamera()->setTargetRotationZ(0);
						}
					}

				}
				m_flagCompassHighlight = sl_false;
			}
			m_flagMouseDown = sl_false;
		}

	} else if (action == UIEvent::actionRightButtonDown) {
		
		getCamera()->clearMotions();
		setFocus();

	} else if (action == UIEvent::actionRightButtonDrag) {

		sl_real dx = pt.x - m_pointMouseBefore.x;
		sl_real dy = pt.y - m_pointMouseBefore.y;

		sl_real r = getCamera()->getRotationZ();
		r += dx;
		getCamera()->setTargetRotationZ(r);

		sl_real t = getCamera()->getTilt();
		t += dy;
		getCamera()->setTargetTilt(t);

	}

	m_pointMouseBefore = pt;
	m_pointMouseBefore2 = pt2;
	m_flagTouchBefore2 = flagTouch2;
}

void MapView::onMouseWheelEvent(UIEvent* event)
{
	if (!m_flagInit) {
		return;
	}
	sl_real delta = event->getDelta();
	if (delta > 0) {
		_zoom(0.5f);
	} else if (delta < 0) {
		_zoom(2.f);
	}
	requestRender();
}

void MapView::_zoom(double ratio)
{
	GeoLocation le = getCamera()->getEyeLocation();
	double h = le.altitude * ratio;
	_zoomTo(h);
}

void MapView::_zoomTo(double alt)
{
	double _min = 20;
	double _max = 12000000;
	if (alt > 0) {
		GeoLocation le = getCamera()->getEyeLocation();
		double h = alt;
		if (h < _min) {
			h = _min;
		}
		if (h > _max) {
			h = _max;
		}
		le.altitude = h;
		getCamera()->startMoving(le, 500);
	}
}

void MapView::startMovingToLookAt(const LatLon& loc)
{
	sl_real altitude = m_earthRenderer->getAltitudeFromLatLon(loc, sl_true);
	getCamera()->startMovingToLookAt(GeoLocation(loc, altitude + 100));
}

Point MapView::getCompassRealPosition()
{
	Point pt = getCompassPosition();
	if (isCompassAlignRight()) {
		pt.x = (sl_real)(m_viewportWidth - pt.x);
	}
	if (isCompassAlignBottom()) {
		pt.y = (sl_real)(m_viewportHeight - pt.y);
	}
	return pt;
}

String MapView::formatLatitude(double f)
{
	String ret;
	if (f > 0) {
		ret = "N";
	} else {
		ret = "S";
		f = -f;
	}
	ret += (sl_int32)(Math::floor(f)) + String(SLIB_UNICODE("°"));
	ret += String::fromInt32((sl_int32)(Math::floor(f * 60)) % 60, 10, 2) + SLIB_UNICODE("ʹ");
	ret += String::fromDouble((sl_real)((sl_int32)(Math::floor(f * 360000)) % 6000) / 100.0, 2, sl_true, 2) + SLIB_UNICODE("ʺ");
	return ret;
}

String MapView::formatLongitude(double f)
{
	String ret;
	if (f > 0) {
		ret = "E";
	} else {
		ret = "W";
		f = -f;
	}
	ret += (sl_int32)(Math::floor(f)) + String(SLIB_UNICODE("°"));
	ret += String::fromInt32((sl_int32)(Math::floor(f * 60)) % 60, 10, 2) + SLIB_UNICODE("ʹ");
	ret += String::fromDouble((sl_real)((sl_int32)(Math::floor(f * 360000)) % 6000) / 100.0, 2, sl_true, 2) + SLIB_UNICODE("ʺ");
	return ret;
}

String MapView::formatRotationZ(double f)
{
	return (sl_int32)(Math::floor(f)) + String(SLIB_UNICODE("°"));
}

String MapView::formatTilt(double f)
{
	return (sl_int32)(Math::floor(f)) + String(SLIB_UNICODE("°"));
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
		ret += "km";
	} else {
		ret += (sl_uint32)(f);
		ret += "m";
	}
	return ret;
}

sl_real MapView::getStatusBarHeight()
{
	sl_real viewportWidth = m_viewportWidth;
	return (sl_real)(STATUS_HEIGHT * viewportWidth / STATUS_WIDTH);
}

String MapView::getStatusText()
{
	GeoLocation loc = getCamera()->getEyeLocation();
	String status = formatLatitude(loc.latitude) + ", " + formatLongitude(loc.longitude) 
		+ ", " + formatAltitude(loc.altitude)
		+ ", " + formatRotationZ(getCamera()->getRotationZ()) + ", " + formatTilt(getCamera()->getTilt());
	return status;
}

Ref<MapMarker> MapView::getMarker(String key)
{
	return m_earthRenderer->markers.getValue(key, Ref<MapMarker>::null());
}

void MapView::putMarker(String key, Ref<MapMarker> marker)
{
	m_earthRenderer->markers.put(key, marker);
}

void MapView::removeMarker(String key)
{
	m_earthRenderer->markers.remove(key);
}

Ref<MapIcon> MapView::getIcon(String key)
{
	return m_earthRenderer->icons.getValue(key, Ref<MapIcon>::null());
}

void MapView::putIcon(String key, Ref<MapIcon> icon)
{
	m_earthRenderer->icons.put(key, icon);
}

void MapView::removeIcon(String key)
{
	m_earthRenderer->icons.remove(key);
}

Ref<MapPolygon> MapView::getPolygon(String key)
{
	return m_earthRenderer->polygons.getValue(key, Ref<MapPolygon>::null());
}

void MapView::putPolygon(String key, Ref<MapPolygon> polygon)
{
	m_earthRenderer->polygons.put(key, polygon);
}

void MapView::removePolygon(String key)
{
	m_earthRenderer->polygons.remove(key);
}

sl_bool MapView::getLocationFromScreenPoint(GeoLocation& out, const Vector2& point)
{
	return m_earthRenderer->getLocationFromScreenPoint(out, point);
}

GeoLocation MapView::getLocationFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways)
{
	return m_earthRenderer->getLocationFromLatLon(latLon, flagReadDEMAlways);
}

sl_real MapView::getAltitudeFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways)
{
	return m_earthRenderer->getAltitudeFromLatLon(latLon, flagReadDEMAlways);
}
SLIB_MAP_NAMESPACE_END

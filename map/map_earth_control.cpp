#include "map_earth_renderer.h"
#include "../../../inc/slibx/map/dem.h"

SLIB_MAP_NAMESPACE_BEGIN

void MapEarthRenderer::_runThreadControl()
{
	m_timeLastThreadControl = Time::now();
	while (!Thread::isStoppingCurrent()) {
		Time now = Time::now();
		m_dtimeThreadControl = (sl_uint32)(Math::clamp((now - m_timeLastThreadControl).getMillisecondsCount(), SLIB_INT64(0), SLIB_INT64(1000)));
		m_timeCurrentThreadControl = now;
		if (m_flagStartedRendering) {
			_runThreadControlStep();
		}
		Time now2 = Time::now();
		sl_uint32 dt = (sl_uint32)(Math::clamp((now2 - now).getMillisecondsCount(), SLIB_INT64(0), SLIB_INT64(1000)));
		if (dt < 20u) {
			Thread::sleep(20u - dt);
		}
		m_timeLastThreadControl = now;
	}
}

void MapEarthRenderer::_runThreadControlStep()
{
	GeoLocation eye = m_camera->getEyeLocation();
	m_altitudeEyeSurface = getAltitudeFromLatLon(eye.getLatLon());
	m_camera->setMinimumAltitude(m_altitudeEyeSurface + 20);
	m_camera->stepMotions((sl_real)m_dtimeThreadControl);
}

LatLon MapEarthRenderer::getLatLonFromTileLocation(const MapTileLocationi& location)
{
	LatLon ret;
	sl_uint32 n = 1 << (location.level);
	sl_uint32 nx = n * getCountX0();
	sl_uint32 ny = n * getCountY0();
	ret.latitude = (location.y) * 180.0 / ny - 90.0;
	ret.longitude = (location.x) * 360.0 / nx - 180.0;
	return ret;
}

MapTileLocation MapEarthRenderer::getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon)
{
	MapTileLocation ret;
	sl_uint32 n = 1 << level;
	sl_uint32 nx = n * getCountX0();
	sl_uint32 ny = n * getCountY0();
	ret.level = level;
	ret.y = (90.0 + latLon.latitude) * ny / 180.0;
	ret.x = (180.0 + latLon.longitude) * nx / 360.0;
	return ret;
}

Vector2 MapEarthRenderer::convertPointToScreen(const Vector3& point)
{
	Vector3 posScreen = Transform3::projectToViewport(m_transformViewProjection, point);
	float x = (posScreen.x + 1.0f) * m_viewportWidth / 2.0f;
	float y = (1.0f - posScreen.y) * m_viewportHeight / 2.0f;
	return Vector2(x, y);
}

sl_bool MapEarthRenderer::getLocationFromScreenPoint(GeoLocation& out, const Vector2& point)
{
	Line3lf line = Transform3lf::unprojectScreenPoint(m_transformProjection, Vector2lf(point.x, point.y), m_viewportWidth, m_viewportHeight);
	Spherelf globe(Vector3lf::zero(), MapEarth::getRadius() + m_altitudeEyeSurface);
	Vector3lf pt1, pt2;
	line.transform(m_transformViewInverse);
	if (globe.intersectLine(line, &pt1, &pt2) > 0) {
		out = MapEarth::getGeoLocation(pt1);
		out.altitude = getAltitudeFromLatLon(out.getLatLon(), sl_true);
		return sl_true;
	} else {
		return sl_false;
	}
}

sl_real MapEarthRenderer::getAltitudeFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways)
{
	MapTileLocation tile = getTileLocationFromLatLon(15, latLon);
	sl_real ret = 0;
	if (m_tilesDEM.isNotNull()) {
		if (flagReadDEMAlways) {
			ret = m_tilesDEM->readAltitudeHierarchically(tile);
		} else {
			ret = m_tilesDEM->getAltitudeHierarchically(tile);
		}
	}
	return ret;
}

GeoLocation MapEarthRenderer::getLocationFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways)
{
	return GeoLocation(latLon, getAltitudeFromLatLon(latLon, flagReadDEMAlways));
}

SLIB_MAP_NAMESPACE_END

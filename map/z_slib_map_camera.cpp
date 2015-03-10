#include "camera.h"

SLIB_MAP_NAMESPACE_START
MapCamera::MapCamera() : m_location(0, 0, 0)
{
	m_tilt = 0;
	m_rotationZ = 0;

	m_flagValidMatrixView = sl_false;
}

void MapCamera::setEyeLocation(const GeoLocation& eye)
{
	m_location = eye;
	m_flagValidMatrixView = sl_false;
}

void MapCamera::setTilt(sl_real angle)
{
	m_tilt = angle;
	m_flagValidMatrixView = sl_false;
}

void MapCamera::setRotationZ(sl_real angle)
{
	m_rotationZ = angle;
	m_flagValidMatrixView = sl_false;
}

void MapCamera::move(sl_geo_val latitude, sl_geo_val longitude, sl_geo_val altitude)
{
	moveTo(m_location.latitude + latitude, m_location.longitude + longitude, m_location.altitude + altitude);
}

void MapCamera::moveTo(sl_geo_val latitude, sl_geo_val longitude, sl_geo_val altitude)
{
	m_location.altitude = altitude;
	m_location.latitude = latitude;
	if (m_location.latitude < -89) {
		m_location.latitude = -89;
	}
	if (m_location.latitude > 89) {
		m_location.latitude = 89;
	}
	m_location.longitude = longitude;
	while (m_location.longitude > 180) {
		m_location.longitude -= 360;
	}
	while (m_location.longitude < -180) {
		m_location.longitude += 360;
	}
	m_flagValidMatrixView = sl_false;
}

void MapCamera::zoom(sl_geo_val ratio, sl_geo_val _min, sl_geo_val _max)
{
	if (ratio > 0) {
		sl_geo_val h = m_location.altitude * ratio;
		if (h < _min) {
			h = _min;
		}
		if (h > _max) {
			h = _max;
		}
		m_location.altitude = h;
		m_flagValidMatrixView = sl_false;
	}
}

void MapCamera::rotateZ(sl_real angle)
{
	setRotationZ(m_rotationZ + angle);
}

void MapCamera::updateViewMatrix()
{
	if (!m_flagValidMatrixView) {
		GeoLocation locAt;
		locAt.altitude = 0;
		locAt.latitude = m_location.latitude;
		locAt.longitude = m_location.longitude;
		Vector3 at = locAt.convertToPosition();
		Vector3 eye = m_location.convertToPosition();
		Matrix4 t = Transform3D::getLookAtMatrix(eye, at, Vector3::axisY());
		if (m_tilt > 0) {
			Vector3 raxis = (eye - at).cross(Vector3::axisY());
			Matrix4 m = Transform3D::getTranslationMatrix(-at)
				* Transform3D::getRotationMatrix(raxis, m_tilt * SLIB_PI / 180.0f)
				* Transform3D::getTranslationMatrix(at);
			eye = (Vector4(eye, 1.0f) * m).xyz();
			t = Transform3D::getLookAtMatrix(eye, at, Vector3::axisY());			
		}
		m_matrixView = t;
		m_flagValidMatrixView = sl_true;
	}
}
SLIB_MAP_NAMESPACE_END

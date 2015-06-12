#include "../../../inc/slibx/map/camera.h"
#include "../../../inc/slibx/map/earth.h"

SLIB_MAP_NAMESPACE_BEGIN

void MapCameraLocation::setEyeLocation(const GeoLocation& eye)
{
	m_location = eye;
	if (m_location.latitude < -80) {
		m_location.latitude = -80;
	}
	if (m_location.latitude > 80) {
		m_location.latitude = 80;
	}
	m_location.normalize();
}

void MapCameraLocation::setTilt(sl_real angle)
{
	if (angle < 0) {
		angle = 0;
	}
	if (angle > 90) {
		angle = 90;
	}
	m_tilt = angle;
}

Matrix4lf MapCameraLocation::getVerticalViewMatrix() const
{
	Matrix4lf matView;
	Vector3lf eye = MapEarth::getCartesianPosition(m_location);
	double lenEye = eye.getLength();

	// look at
	{
		matView = Transform3lf::getRotationYMatrix(Math::getRadianFromDegrees(m_location.longitude));
		matView *= Transform3lf::getRotationXMatrix(Math::getRadianFromDegrees(-m_location.latitude));
		matView *= Transform3lf::getTranslationMatrix(0, 0, lenEye);
	}
	if (m_rotationZ != 0) {
		matView *= Transform3lf::getRotationZMatrix(Math::getRadianFromDegrees(m_rotationZ));
	}
	return matView;
}

Matrix4lf MapCameraLocation::getViewMatrix() const
{
	Matrix4lf matView = getVerticalViewMatrix();
	double t = getTilt();
	if (t > 0) {
		matView *= Transform3lf::getRotationXMatrix(Math::getRadianFromDegrees(t));
	}
	return matView;
}

sl_real MapCameraLocation::getMaxTilt() const
{
	double m1 = 350000, m2 = 10000;
	if (m_location.altitude > m1) {
		return 0;
	} else {
		Vector3lf eye = MapEarth::getCartesianPosition(m_location);
		double lenEye = eye.getLength();
		double R = MapEarth::getRadius();
		if (lenEye < R) {
			lenEye = R;
		}
		double sinAlpha = (R - 1) / lenEye;
		double alpha = Math::getDegreesFromRadian(Math::arcsin(sinAlpha));
		if (m_location.altitude < m2) {
			alpha = alpha * (m1 - m_location.altitude) / (m1 - m2);
		}
		return (sl_real)alpha;
	}
}


MapCamera::MapCamera()
{
	m_flagMoving = sl_false;
	m_indexMovingTargets = 0;
	m_targetAltitudeMin = 0;

	m_flagRotatingZ = sl_false;
	m_targetRotationZ = 0;

	m_flagTilting = sl_false;
	m_targetTilt = 0;

}

void MapCamera::clearMovingTargets()
{
	MutexLocker lock(getLocker());
	m_flagMoving = sl_false;
	m_listMovingTargets.clear();
	m_indexMovingTargets = 0;
}

void MapCamera::addMovingTarget(const GeoLocation& location, sl_real duration)
{
	MutexLocker lock(getLocker());
	MapCameraMovingTarget target;
	target.location = location;
	target.duration = duration;
	m_listMovingTargets.add(target);
}

void MapCamera::addMovingTargets(const List<MapCameraMovingTarget>& targets)
{
	MutexLocker lock(getLocker());
	m_listMovingTargets.add(targets);
}

void MapCamera::startMoving()
{
	MutexLocker lock(getLocker());
	m_indexMovingTargets = 0;
	m_flagMoving = sl_true;
}

void MapCamera::startMoving(const GeoLocation& location, sl_real duration)
{
	MutexLocker lock(getLocker());
	startMoving(makeBufferedMovingTargets(getEyeLocation(), location, duration, 4, 0.7f, 0.3f));
}

void MapCamera::startMoving(const LatLon& location, sl_real durationMillis)
{
	startMoving(GeoLocation(location, getEyeLocation().altitude), durationMillis);
}

void MapCamera::startMoving(const List<MapCameraMovingTarget>& targets)
{
	MutexLocker lock(getLocker());
	m_listMovingTargets = targets;
	m_indexMovingTargets = 0;
	m_flagMoving = sl_true;
}

void MapCamera::startMovingToLookAt(const GeoLocation& location)
{
	MutexLocker lock(getLocker());
	setTargetTilt(0);
	Vector3lf pos1 = MapEarth::getCartesianPosition(getEyeLocation());
	Vector3lf pos2 = MapEarth::getCartesianPosition(location);
	double len = (pos2 - pos1).getLength();
	GeoLocation eye = getEyeLocation();
	double a = (eye.altitude + location.altitude) / 2;
	if (len > a * 2) {
		List<MapCameraMovingTarget> targets;
		GeoLocation midLoc;
		midLoc.longitude = (location.longitude + eye.longitude) / 2;
		midLoc.latitude = (location.latitude + eye.latitude) / 2;
		midLoc.altitude = a + len / 3;
		MapCameraMovingTarget target;
		target.duration = 1000;
		target.location = midLoc;
		targets.add(target);
		targets.add(makeBufferedMovingTargets(midLoc, location, 2000, 4, 0.7f, 0.3f));
		startMoving(targets);
	} else {
		startMoving(location, 3000);
	}
}

List<MapCameraMovingTarget> MapCamera::makeBufferedMovingTargets(
	const GeoLocation& locationCurrent, const GeoLocation& locationTarget
	, sl_real durationMillis, sl_uint32 nBufferSegments
	, sl_real bufferRatioLocation, sl_real bufferRatioDuration)
{
	List<MapCameraMovingTarget> ret;
	MapCameraMovingTarget target;
	if (nBufferSegments == 0) {
		return ret;
	}
	sl_real rt = durationMillis;
	GeoLocation c = locationCurrent;
	for (sl_uint32 i = 0; i < nBufferSegments; i++) {
		if (i == nBufferSegments - 1) {
			target.location = locationTarget;
			target.duration = rt;
			ret.add(target);
		} else {
			target.location = c.lerp(locationTarget, bufferRatioLocation);
			target.duration = rt * bufferRatioDuration;
			ret.add(target);
			c = target.location;
			rt = rt - target.duration;
		}
	}
	return ret;
}

void MapCamera::stopMoving()
{
	MutexLocker lock(getLocker());
	m_flagMoving = sl_false;
}

void MapCamera::resumeMoving()
{
	MutexLocker lock(getLocker());
	m_flagMoving = sl_true;
}

void MapCamera::setTargetRotationZ(sl_real target)
{
	MutexLocker lock(getLocker());
	m_flagRotatingZ = sl_true;
	m_targetRotationZ = target;
}

void MapCamera::setTargetTilt(sl_real target)
{
	MutexLocker lock(getLocker());
	m_flagTilting = sl_true;
	m_targetTilt = normalizeTilt(target);
}

void MapCamera::setMinimumAltitude(double target)
{
	m_targetAltitudeMin = target;
}

void MapCamera::clearMotions()
{
	MutexLocker lock(getLocker());
	m_flagMoving = sl_false;
	m_flagRotatingZ = sl_false;
	m_flagTilting = sl_false;
}

void MapCamera::stepMotions(sl_real dt)
{
	MutexLocker lock(getLocker());
	if (m_flagMoving) {
		if (m_indexMovingTargets < m_listMovingTargets.count()) {
			MapCameraMovingTarget& t = m_listMovingTargets[m_indexMovingTargets];
			GeoLocation tl = t.location;
			if (tl.altitude < m_targetAltitudeMin) {
				tl.altitude = m_targetAltitudeMin;
			}
			if (Math::isLessThanEpsilon(t.duration) || t.duration < dt) {
				setEyeLocation(tl);
				m_indexMovingTargets++;
			} else {
				GeoLocation o = getEyeLocation();
				sl_real s = dt / t.duration;
				setEyeLocation(o.lerp(tl, s));
				t.duration -= dt;
			}
		} else {
			m_flagMoving = sl_false;
		}
	}
	{
		double ta = m_targetAltitudeMin;
		double a = getEyeLocation().altitude;
		if (a < ta) {
			double da = ta - a;
			double k = dt;
			if (da < k) {
				a = ta;
			} else {
				a += k;
			}
			setEyeLocation(GeoLocation(getEyeLocation().getLatLon(), a));
		}
	}

	if (m_flagRotatingZ) {
		sl_real tr = Math::normalizeDegree(m_targetRotationZ);
		sl_real _or = getRotationZ();
		sl_real dr = tr - _or;
		if (dr > 180) {
			dr -= 360;
		} else if (dr < -180) {
			dr += 360;
		}
		sl_real s = 360.0f * dt / 1000.0f;
		if (Math::abs(dr) <= s) {
			setRotationZ(tr);
			m_flagRotatingZ = sl_false;
		} else {
			if (dr > 0) {
				setRotationZ(Math::normalizeDegree(_or + s));
			} else {
				setRotationZ(Math::normalizeDegree(_or - s));
			}
		}
	}
	sl_real maxTilt = m_current.getMaxTilt();
	if (!m_flagTilting) {
		if (getTilt() > maxTilt) {
			m_targetTilt = maxTilt;
			m_flagTilting = sl_true;
		}
	}
	if (m_flagTilting) {
		if (m_targetTilt > maxTilt) {
			m_targetTilt = maxTilt;
		}
		sl_real tr = normalizeTilt(m_targetTilt);
		sl_real _or = getTilt();
		sl_real dr = tr - _or;
		sl_real s = 90.0f * dt / 2000.0f;
		if (Math::abs(dr) <= s) {
			setTilt(tr);
			m_flagTilting = sl_false;
		} else {
			if (dr > 0) {
				setTilt(normalizeTilt(_or + s));
			} else {
				setTilt(normalizeTilt(_or - s));
			}
		}
	}
	
}

sl_real MapCamera::normalizeTilt(sl_real t)
{
	if (t < 0) {
		t = 0;
	}
	if (t > 90) {
		t = 90;
	}
	return t;
}
SLIB_MAP_NAMESPACE_END

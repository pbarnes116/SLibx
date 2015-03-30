#include "camera.h"
#include "earth.h"

SLIB_MAP_NAMESPACE_START


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

sl_real MapCameraLocation::getRevisedRotationZ() const
{
	return m_rotationZ;
}

sl_real MapCameraLocation::getRevisedTilt() const
{
	Vector3 eye = MapEarth::getCartesianPosition(m_location);
	sl_real lenEye = eye.getLength();
	sl_real R = (float)(MapEarth::getRadius());
	sl_real t = m_tilt;
	if (t > 0) {
		if (lenEye < R) {
			lenEye = R;
		}
		sl_real sinAlpha = (float)(R / lenEye);
		sl_real alpha = Math::getDegreeFromRadian(Math::arcsin(sinAlpha));
		if (t > alpha) {
			t = alpha;
		}
		if (m_location.altitude > 20000) {
			t = (sl_real)(t * (20000 / m_location.altitude));
		}
	} else {
		t = 0;
	}
	return t;
}

Matrix4 MapCameraLocation::getVerticalViewMatrix() const
{
	Matrix4 matView;
	Vector3 eye = MapEarth::getCartesianPosition(m_location);
	sl_real lenEye = eye.getLength();

	// look at
	{
		matView = Transform3::getRotationYMatrix((float)(Math::getRadianFromDegree(m_location.longitude)));
		matView *= Transform3::getRotationXMatrix((float)(Math::getRadianFromDegree(-m_location.latitude)));
		matView *= Transform3::getTranslationMatrix(0, 0, lenEye);
	}
	if (m_rotationZ != 0) {
		matView *= Transform3::getRotationZMatrix(Math::getRadianFromDegree(m_rotationZ));
	}
	return matView;
}

Matrix4 MapCameraLocation::getViewMatrix() const
{
	Matrix4 matView = getVerticalViewMatrix();
	sl_real t = getRevisedTilt();
	if (t > 0) {
		matView *= Transform3::getRotationXMatrix(Math::getRadianFromDegree(t));
	}
	return matView;
}


MapCamera::MapCamera()
{
	m_flagMoving = sl_false;
	m_flagRotatingZ = sl_false;
	m_flagTilting = sl_false;
	m_indexMovingTargets = 0;
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
	MapCameraMovingTarget target;
	target.location = location;
	target.duration = duration;
	m_listMovingTargets.clear();
	m_listMovingTargets.add(target);
	m_indexMovingTargets = 0;
	m_flagMoving = sl_true;
	startMoving(makeBufferedMovingTargets(getEyeLocation(), location, duration, 10, 0.8f, 0.3f));
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

void MapCamera::startRotatingZ(sl_real target)
{
	MutexLocker lock(getLocker());
	m_flagRotatingZ = sl_true;
	m_targetRotationZ = target;
}

void MapCamera::startTilting(sl_real target)
{
	MutexLocker lock(getLocker());
	m_flagTilting = sl_true;
	m_targetTilt = target;
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
			if (Math::isLessThanEpsilon(t.duration) || t.duration < dt) {
				setEyeLocation(t.location);
				m_indexMovingTargets++;
			} else {
				GeoLocation o = getEyeLocation();
				sl_real s = dt / t.duration;
				setEyeLocation(o.lerp(t.location, s));
				t.duration -= dt;
			}
		} else {
			m_flagMoving = sl_false;
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
	if (m_flagTilting) {
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

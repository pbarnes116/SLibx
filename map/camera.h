#ifndef CHECKHEADER_SLIB_MAP_CAMERA
#define CHECKHEADER_SLIB_MAP_CAMERA

#include "definition.h"
#include "earth.h"

#include "../../slib/math/geometry.h"
#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

class MapCameraLocation
{
public:
	SLIB_INLINE MapCameraLocation() : m_location(0, 0, 0)
	{
		m_tilt = 0;
		m_rotationZ = 0;
	}

	SLIB_INLINE MapCameraLocation(const MapCameraLocation& other)
		: m_location(other.m_location)
		, m_tilt(other.m_tilt)
		, m_rotationZ(other.m_rotationZ)
	{
	}

	SLIB_INLINE const GeoLocation& getEyeLocation() const
	{
		return m_location;
	}
	void setEyeLocation(const GeoLocation& eye);

	SLIB_INLINE sl_real getTilt() const
	{
		return m_tilt;
	}
	void setTilt(sl_real degree);

	SLIB_INLINE sl_real getRotationZ() const
	{
		return m_rotationZ;
	}
	SLIB_INLINE void setRotationZ(sl_real degree)
	{
		m_rotationZ = degree;
	}

	Matrix4lf getViewMatrix() const;
	Matrix4lf getVerticalViewMatrix() const;

	sl_real getMaxTilt() const;

private:
	GeoLocation m_location;
	sl_real m_tilt;
	sl_real m_rotationZ;

};

struct MapCameraMovingTarget
{
	GeoLocation location;
	sl_real duration; // milliseconds
};

class MapCamera : public Object
{
	SLIB_DECLARE_OBJECT(MapCamera, map_camera);
public:
	MapCamera();

	SLIB_INLINE const GeoLocation& getEyeLocation() const
	{
		return m_current.getEyeLocation();
	}
	SLIB_INLINE void setEyeLocation(const GeoLocation& eye)
	{
		m_current.setEyeLocation(eye);
	}
	SLIB_INLINE Vector3 getEyeCartesianPosition() const
	{
		return MapEarth::getCartesianPosition(getEyeLocation());
	}

	SLIB_INLINE sl_real getTilt() const
	{
		return m_current.getTilt();
	}
	SLIB_INLINE void setTilt(sl_real degree)
	{
		m_current.setTilt(degree);
	}

	SLIB_INLINE sl_real getRotationZ() const
	{
		return m_current.getRotationZ();
	}
	SLIB_INLINE void setRotationZ(sl_real degree)
	{
		m_current.setRotationZ(degree);
	}

	SLIB_INLINE Matrix4 getViewMatrix() const
	{
		return m_current.getViewMatrix();
	}

	SLIB_INLINE Matrix4 getVerticalViewMatrix() const
	{
		return m_current.getVerticalViewMatrix();
	}

	void clearMovingTargets();
	void addMovingTarget(const GeoLocation& location, sl_real durationMillis);
	void addMovingTargets(const List<MapCameraMovingTarget>& targets);
	void startMoving();
	void startMoving(const GeoLocation& location, sl_real durationMillis);
	void startMoving(const LatLon& location, sl_real durationMillis);
	void startMoving(const List<MapCameraMovingTarget>& targets);
	void resumeMoving();
	void stopMoving();

	void setTargetRotationZ(sl_real target);
	sl_real getTargetRotationZ()
	{
		return m_targetRotationZ;
	}

	void setTargetTilt(sl_real target);
	sl_real getTargetTilt()
	{
		return m_targetTilt;
	}

	void setMinimumAltitude(double altitude);
	double getMimimumAltitude()
	{
		return m_targetAltitudeMin;
	}

	void clearMotions();

	void stepMotions(sl_real dt);

	static List<MapCameraMovingTarget> makeBufferedMovingTargets(
		const GeoLocation& locationCurrent, const GeoLocation& locationTarget
		, sl_real durationMillis, sl_uint32 nBufferSegments
		, sl_real bufferRatioLocation, sl_real bufferRatioDuration);

	static sl_real normalizeTilt(sl_real t);

private:
	MapCameraLocation m_current;

	sl_bool m_flagMoving;
	List<MapCameraMovingTarget> m_listMovingTargets;
	sl_size m_indexMovingTargets;

	double m_targetAltitudeMin;

	sl_bool m_flagRotatingZ;
	sl_real m_targetRotationZ;

	sl_bool m_flagTilting;
	sl_real m_targetTilt;
};

SLIB_MAP_NAMESPACE_END

#endif

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

	sl_real getRevisedRotationZ() const;
	sl_real getRevisedTilt() const;

	Matrix4lf getViewMatrix() const;
	Matrix4lf getVerticalViewMatrix() const;

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
		return m_current.getRevisedTilt();
	}
	SLIB_INLINE void setTilt(sl_real degree)
	{
		m_current.setTilt(degree);
	}

	SLIB_INLINE sl_real getRotationZ() const
	{
		return m_current.getRevisedRotationZ();
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

	void startRotatingZ(sl_real target);
	void startTilting(sl_real target);

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

	sl_bool m_flagRotatingZ;
	sl_real m_targetRotationZ;

	sl_bool m_flagTilting;
	sl_real m_targetTilt;
};

SLIB_MAP_NAMESPACE_END

#endif

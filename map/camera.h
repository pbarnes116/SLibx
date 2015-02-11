#ifndef CHECKHEADER_SLIB_MAP_CAMERA
#define CHECKHEADER_SLIB_MAP_CAMERA

#include "definition.h"

#include "../../slib/math/geometry.h"
#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

class MapCamera
{
public:
	MapCamera();

	const GeoLocation& getEyeLocation() const
	{
		return m_location;
	}
	void setEyeLocation(const GeoLocation& eye);

	sl_real getTilt() const
	{
		return m_tilt;
	}
	void setTilt(sl_real degree);

	sl_real getRotationZ() const
	{
		return m_rotationZ;
	}
	void setRotationZ(sl_real degree);

	void move(sl_geo_val latitudeOffset, sl_geo_val longitudeOffset, sl_geo_val altitudeOffset);
	void moveTo(sl_geo_val latitude, sl_geo_val longitude, sl_geo_val altitude);
	void zoom(sl_geo_val ratio, sl_geo_val altitudeMin = 100, sl_geo_val altitudeMax = 12000000);
	void rotateZ(sl_real angle);

	SLIB_INLINE Matrix4 getViewMatrix()
	{
		updateViewMatrix();
		return m_matrixView;
	}
	SLIB_INLINE Matrix4 getTiltViewMatrix()
	{
		updateViewMatrix();
		return m_matrixViewTilt;
	}

protected:
	GeoLocation m_location;
	sl_real m_tilt;
	sl_real m_rotationZ;

	sl_bool m_flagValidMatrixView;
	Matrix4 m_matrixView;
	Matrix4 m_matrixViewTilt;

	void updateViewMatrix();
};
SLIB_MAP_NAMESPACE_END

#endif

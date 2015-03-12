#ifndef CHECKHEADER_SLIB_MAP_CAMERA
#define CHECKHEADER_SLIB_MAP_CAMERA

#include "definition.h"

#include "../../slib/math/geometry.h"
#include "../../slib/math/geograph.h"

SLIB_MAP_NAMESPACE_START

class MapCamera : public Referable
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

	void move(double latitudeOffset, double longitudeOffset, double altitudeOffset);
	void moveTo(double latitude, double longitude, double altitude);
	void zoom(double ratio, double altitudeMin = 20, double altitudeMax = 12000000);
	void rotateZ(sl_real angle);

	SLIB_INLINE Matrix4 getViewMatrix()
	{
		updateViewMatrix();
		return m_matrixView;
	}

protected:
	GeoLocation m_location;
	sl_real m_tilt;
	sl_real m_rotationZ;

	sl_bool m_flagValidMatrixView;
	Matrix4 m_matrixView;

	void updateViewMatrix();
};
SLIB_MAP_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_MAP_MAPVIEW
#define CHECKHEADER_SLIB_MAP_MAPVIEW

#include "definition.h"

#include "environment.h"
#include "earth.h"
#include "data.h"

#include "../../slib/ui/view.h"
#include "../../slib/image/freetype.h"
#include "../../slib/sensor.h"

SLIB_MAP_NAMESPACE_START

class SensorCallback
{
public:
	SensorCallback(){}
	~SensorCallback(){}

	virtual void onLocationChanged(Sensor* sensor, const GeoLocation& location) {};

	// degree, 0 - North, 180 - South, 90 - East, 270 - West
	virtual void onCompassChanged(Sensor* sensor, sl_real declination) {};

	virtual void onAccelerometerChanged(Sensor* sensor, sl_real xAccel, sl_real yAccel, sl_real zAccel){};
};

class MapViewSensorListener : public SensorListener
{
public:
	SLIB_INLINE MapViewSensorListener()
	{
		m_cbSensor = sl_null;
	}

	SLIB_INLINE MapViewSensorListener(SensorCallback* _cbSensor)
	{
		m_cbSensor = _cbSensor;
	}
	~MapViewSensorListener()
	{

	}

	virtual void onLocationChanged(Sensor* sensor, const GeoLocation& location) 
	{
		if (m_cbSensor != sl_null) {
			m_cbSensor->onLocationChanged(sensor, location);
		}	
	}

	// degree, 0 - North, 180 - South, 90 - East, 270 - West
	virtual void onCompassChanged(Sensor* sensor, sl_real declination) 
	{
		if (m_cbSensor != sl_null) {
			m_cbSensor->onCompassChanged(sensor, declination);
		}
	}

	virtual void onAccelerometerChanged(Sensor* sensor, sl_real xAccel, sl_real yAccel, sl_real zAccel)
	{
		if (m_cbSensor != sl_null) {
			m_cbSensor->onAccelerometerChanged(sensor, xAccel, yAccel, zAccel);
		}
	}

	SLIB_INLINE void setCallback(SensorCallback* _cbSensor) 
	{
		m_cbSensor = _cbSensor;
	}
private:
	SensorCallback* m_cbSensor;
};

class MapView : public RenderView, SensorCallback
{
public:
	MapView();
	~MapView();

public:
	virtual void release();

	virtual void onFrame(RenderEngine* engine);
	virtual sl_bool onMouseEvent(MouseEvent& event);
	virtual sl_bool onMouseWheelEvent(MouseWheelEvent& event);

	virtual void onAccelerometerChanged(Sensor* sensor, sl_real xAccel, sl_real yAccel, sl_real zAccel);
	virtual void onCompassChanged(Sensor* sensor, sl_real declination);
	virtual void onLocationChanged(Sensor* sensor, const GeoLocation& location);

	virtual String formatLatitude(double f);
	virtual String formatLongitude(double f);
	virtual String formatAltitude(double f);
	virtual String formatCompass(sl_real f);
	virtual String getStatusText();

protected:
	void initialize();
	
protected:
	sl_bool m_flagInit;

	Ref<MapEnvironment> m_environment;

	Ref<MapEarthRenderer> m_earthRenderer;

	Ref<Texture> m_textureStatus;

	Ref<Sensor> m_sensor;
	Ref<SensorListener> m_sensorListener;
	
	SLIB_PROPERTY_SIMPLE(Ref<FreeType>, StatusFont);
	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoader>, DataLoader);

protected:
	sl_real m_mouseBeforeX;
	sl_real m_mouseBeforeY;
	sl_bool m_flagTouch2;
	sl_real m_mouseBefore2X;
	sl_real m_mouseBefore2Y;
	sl_real m_mouseBeforeRightX;
	sl_real m_mouseBeforeRightY;

	sl_real m_sensorBeforeAccelY;
	sl_real m_compassBare;
};
SLIB_MAP_NAMESPACE_END

#endif


#ifndef CHECKHEADER_SLIB_MAP_MAPVIEW
#define CHECKHEADER_SLIB_MAP_MAPVIEW

#include "definition.h"

#include "environment.h"
#include "earth.h"
#include "data.h"

#include "../../slib/ui/view.h"
#include "../../slib/image/freetype.h"

SLIB_MAP_NAMESPACE_START

class MapView : public RenderView
{
public:
	MapView();
	~MapView();

public:
	virtual void release();

	virtual void onFrame(RenderEngine* engine);
	virtual sl_bool onMouseEvent(MouseEvent& event);
	virtual sl_bool onMouseWheelEvent(MouseWheelEvent& event);

	virtual String formatLatitude(double f);
	virtual String formatLongitude(double f);
	virtual String formatAltitude(double f);
	virtual String getStatusText();

	void setFontForPOI(Ref<FreeType> font);
	void setPoiNames(Map<sl_int64, String> poiNames);
	void setWayNames(Map<sl_int64, String> wayNames);

	Ref<MapMarker> getMarker(String key);
	void putMarker(String key, Ref<MapMarker> marker);
	void removeMarker(String key);

	Ref<MapPolygon> getPolygon(String key);
	void putPolygon(String key, Ref<MapPolygon> polygon);
	void removePolygon(String key);

protected:
	void initialize();
	
private:
	sl_bool m_flagInit;

	Ref<MapEnvironment> m_environment;
	Ref<MapEarthRenderer> m_earthRenderer;
	Ref<Texture> m_textureStatus;

	sl_real m_mouseBeforeX;
	sl_real m_mouseBeforeY;
	sl_bool m_flagTouch2;
	sl_real m_mouseBefore2X;
	sl_real m_mouseBefore2Y;
	sl_real m_mouseBeforeRightX;
	sl_real m_mouseBeforeRightY;

public:
	const Ref<MapEnvironment>& getEnvironment()
	{
		return m_environment;
	}

	SLIB_PROPERTY_SIMPLE(Ref<FreeType>, StatusFont);
	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoader>, DataLoader);

};
SLIB_MAP_NAMESPACE_END

#endif


#ifndef CHECKHEADER_SLIB_MAP_MAPVIEW
#define CHECKHEADER_SLIB_MAP_MAPVIEW

#include "definition.h"
#include "camera.h"
#include "object.h"
#include "data.h"
#include "data_gis.h"

#include <slib/ui/render_view.h>
#include <slib/graphics/freetype.h>
#include <slib/core/thread.h>

SLIB_MAP_NAMESPACE_BEGIN

class MapEarthRenderer;
class MapDEMTileManager;

class SLIB_EXPORT MapView : public RenderView
{
	SLIB_DECLARE_OBJECT(MapView, RenderView)
public:
	MapView();
	~MapView();

public:
	void initialize();
	virtual void release();

	Ref<MapCamera> getCamera();
	
	Ref<FreeType> getFontForPOI();
	void setFontForPOI(Ref<FreeType> font);
	void setViewportSize(const Sizef& size);
	void setPoiInformation(Map<sl_int64, MapGISPoiInfo> poiInformation);
	void setWayNames(Map<sl_int64, String> wayNames);

	Ref<MapMarker> getMarker(String key);
	void putMarker(String key, Ref<MapMarker> marker);
	void removeMarker(String key);

	Ref<MapIcon> getIcon(String key);
	void putIcon(String key, Ref<MapIcon> icon);
	void removeIcon(String key);

	Ref<MapPolygon> getPolygon(String key);
	void putPolygon(String key, Ref<MapPolygon> polygon);
	void removePolygon(String key);

	void startMovingToLookAt(const LatLon& loc);

	LatLon getLatLonFromTileLocation(const MapTileLocationi& location);
	MapTileLocation getTileLocationFromLatLon(sl_uint32 level, const LatLon& latLon);

	sl_bool getLocationFromScreenPoint(GeoLocation& out, const Vector2& point);
	GeoLocation getLocationFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways = sl_false);
	sl_real getAltitudeFromLatLon(const LatLon& latLon, sl_bool flagReadDEMAlways = sl_false);
	
	sl_bool isShowLayer(sl_uint32 layer);
	void setShowLayer(sl_uint32 layer, sl_bool flagShow);
	sl_bool isShowBuilding();
	void setShowBuilding(sl_bool flagShow);
	sl_bool isShowGISPoi();
	void setShowGISPoi(sl_bool flagShow);
	sl_bool isShowGISLine();
	void setShowGISLine(sl_bool flagShow);
	
public:
	virtual String formatLatitude(double f);
	virtual String formatLongitude(double f);
	virtual String formatAltitude(double f);
	virtual String formatRotationZ(double f);
	virtual String formatTilt(double f);
	virtual String getStatusText();
	virtual sl_real getStatusBarHeight();
	
	SLIB_INLINE sl_real getViewportWidth()
	{
		return m_viewportWidth;
	}
	
	SLIB_INLINE sl_real getViewportHeight()
	{
		return m_viewportHeight;
	}

protected:
	virtual void onFrame(RenderEngine* engine);
	virtual void onMouseEvent(UIEvent* event);
	virtual void onMouseWheelEvent(UIEvent* event);
	virtual void onClick();

protected:
	void _zoomTo(double alt);
	void _zoom(double ratio);
	
public:
	SLIB_REF_PROPERTY(FreeType, StatusFont);
	SLIB_REF_PROPERTY(MapDataLoader, DataLoader);
	
	SLIB_REF_PROPERTY(Texture, CompassTexture);
	SLIB_PROPERTY(Rectangle, CompassTextureRectangle);
	SLIB_REF_PROPERTY(Texture, CompassHighlightTexture);
	SLIB_PROPERTY(Rectangle, CompassHighlightTextureRectangle);
	SLIB_PROPERTY(sl_real, CompassSize);
	SLIB_PROPERTY(Point, CompassPosition);
	SLIB_BOOLEAN_PROPERTY(CompassVisible);
	SLIB_BOOLEAN_PROPERTY(CompassAlignRight);
	SLIB_BOOLEAN_PROPERTY(CompassAlignBottom);
	
	Point getCompassRealPosition();
	
	SLIB_BOOLEAN_PROPERTY(StatusBarVisible);
	SLIB_BOOLEAN_PROPERTY(StatusBarLocateAtTop);
	
	SLIB_REF_PROPERTY(Runnable, OnClick);
	SLIB_PROPERTY(Point, LastClickedPoint);
	
private:
	sl_bool m_flagInit;

	Ref<MapEarthRenderer> m_earthRenderer;

	SafeRef<Texture> m_textureStatus;

	sl_real m_viewportWidth;
	sl_real m_viewportHeight;

	Point m_pointMouseBefore;
	Point m_pointMouseBefore2;
	sl_bool m_flagTouchBefore2;

	LatLon m_locationMouseDown;
	Point m_pointMouseDown;
	Time m_timeMouseDown;
	Matrix4 m_transformMouseDown;
	sl_bool m_flagMouseExitMoving;
	sl_bool m_flagMouseDown;

	Point m_pointTouchStart1;
	Point m_pointTouchStart2;
	double m_zoomTouchStart;
	sl_real m_rotateTouchStart;
	sl_bool m_flagTouchRotateStarted;
	
	sl_bool m_flagCompassHighlight;
	sl_bool m_flagClicking;
	sl_real m_compassMouseDown;
};
SLIB_MAP_NAMESPACE_END

#endif


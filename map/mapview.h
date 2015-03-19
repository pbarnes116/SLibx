#ifndef CHECKHEADER_SLIB_MAP_MAPVIEW
#define CHECKHEADER_SLIB_MAP_MAPVIEW

#include "definition.h"

#include "environment.h"
#include "tile.h"
#include "data.h"

#include "../../slib/ui/view.h"
#include "../../slib/image/freetype.h"

SLIB_MAP_NAMESPACE_START
class MapTileManager_VWBuilding;
class MapTileManager_GIS_Shape;
class MapTileManager_GIS_Poi;
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

protected:
	void initialize();

protected:
	sl_bool m_flagInit;

	Ref<MapEnvironment> m_environment;

	Ref<MapTileManager_DEM> m_tileManagerDEM;
	Ref<MapTileManager_VWBuilding> m_tileManagerVWBuilding;
	
	Ref<MapTileManager_GIS_Poi> m_tileManagerGISPoi;
	Ref<MapTileManager_GIS_Shape> m_tileManagerGISLine;

	Ref<Texture> m_textureStatus;
	
	SLIB_PROPERTY_SIMPLE(Ref<FreeType>, StatusFont);
	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoaderPack>, DataLoader);

protected:
	sl_real m_mouseBeforeX;
	sl_real m_mouseBeforeY;
	sl_bool m_flagTouch2;
	sl_real m_mouseBefore2X;
	sl_real m_mouseBefore2Y;
	sl_real m_mouseBeforeRightX;
	sl_real m_mouseBeforeRightY;
};
SLIB_MAP_NAMESPACE_END

#endif


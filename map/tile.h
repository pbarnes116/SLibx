#ifndef CHECKHEADER_SLIB_MAP_TILE
#define CHECKHEADER_SLIB_MAP_TILE

#include "definition.h"

#include "environment.h"
#include "util.h"
#include "data.h"

#include "../../slib/core/object.h"
#include "../../slib/core/time.h"
#include "../../slib/core/thread.h"
#include "../../slib/core/array2d.h"
#include "../../slib/render/engine.h"

SLIB_MAP_NAMESPACE_START

class MapTile : public Referable
{
public:
	Ref<MapTile> children[2][2];

	sl_uint32 level;
	sl_uint32 y;
	sl_uint32 x;
	sl_uint32 iy;
	sl_uint32 ix;

	GeoRectangle rect;
	Vector3 positions[4];
	Vector3 positionCenter;
	Time timeLastAccess;

	sl_bool flagCanRender;

protected:
	MapTile();
};

class MapTileManager : public Referable
{
protected:
	MapTileManager();
	~MapTileManager();

public:
	SLIB_PROPERTY_SIMPLE(sl_uint32, MaximumLevel);
	SLIB_PROPERTY_SIMPLE(sl_uint32, TileLifeMillseconds);

public:
	void initialize();
	void release();
	virtual void renderTiles(RenderEngine* engine, MapEnvironment* environment);

protected:
	virtual Ref<MapTile> createTile() = 0;
	virtual sl_bool initializeTile(MapTile* tile, MapTile* parent) = 0;
	virtual void freeTile(MapTile* tile) = 0;
	virtual void renderTile(MapTile* tile, RenderEngine* engine, MapEnvironment* environment) = 0;
	virtual LatLon getLatLonFromTileLocation(sl_uint32 level, sl_uint32 y, sl_uint32 x);

protected:
	Ref<MapTile> _createTile(sl_uint32 level, sl_uint32 y, sl_uint32 x, sl_uint32 iy, sl_uint32 ix);
	sl_bool _initializeTile(MapTile* tile, MapTile* parent);
	void _renderTile(MapTile* tile, RenderEngine* engine, MapEnvironment* environment);
	void _freeTile(MapTile* tile);
	void _initializeTopTiles(sl_uint32 level, sl_uint32 nY, sl_uint32 nX);
	void _setTileLocation(MapTile* tile, sl_uint32 level, sl_uint32 y, sl_uint32 x);
	sl_bool _checkTileVisibleInViewFrustum(MapTile* tile);
	void _updateRenderState(MapEnvironment* environment);

protected:
	Array2D< Ref<MapTile> > m_tilesTop;
	sl_uint32 m_levelTop;
	sl_uint32 m_nYTop;
	sl_uint32 m_nXTop;

	Ref<MapEnvironment> m_environment;
	Ref<Thread> m_threadManage;

	Time m_timeCurrent;
	Time m_timeLastRenderRequest;

private:
	static void _threadManageTile(MapTileManager* manager);
	void _runThreadManageTileStep();
};

class MapTileManager_DEM : public MapTileManager
{
public:
	MapTileManager_DEM();

	void initialize();

	Ref<MapTile> createTile();
	sl_bool initializeTile(MapTile* tile, MapTile* parent);
	void freeTile(MapTile* tile);
	void renderTile(MapTile* tile, RenderEngine* engine, MapEnvironment* environment);
	void renderTiles(RenderEngine* engine, MapEnvironment* environment);

protected:
	Ref<RenderProgram3D> m_programTile;

	SLIB_PROPERTY_SIMPLE(Ref<MapDataLoaderPack>, DataLoader);
};

SLIB_MAP_NAMESPACE_END

#endif

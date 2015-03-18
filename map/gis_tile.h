#ifndef CHECKHEADER_SLIB_MAP_GIS_TILE
#define CHECKHEADER_SLIB_MAP_GIS_TILE

#include "definition.h"

#include "tile.h"

SLIB_MAP_NAMESPACE_START

class MapTileManager_GIS_Poi : public MapTileManager
{
public:
	MapTileManager_GIS_Poi();

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

class MapTileManager_GIS_Line : public MapTileManager
{
public:
	MapTileManager_GIS_Line();

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

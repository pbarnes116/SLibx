#ifndef CHECKHEADER_SLIB_MAP_DATA
#define CHECKHEADER_SLIB_MAP_DATA

#include "definition.h"

#include "util.h"

#include "../../slib/core/object.h"
#include "../../slib/core/memory.h"
#include "../../slib/core/string.h"

SLIB_MAP_NAMESPACE_START

class MapDataLoader : public Object
{
protected:
	SLIB_INLINE MapDataLoader() {}

public:
	virtual Memory loadData(const String& type, const MapTileLocationi& location, const String& subPath) = 0;
	SLIB_INLINE Memory loadData(const String& type, const MapTileLocationi& location)
	{
		return loadData(type, location, String::null());
	}
};

class MapDataLoaderList : public MapDataLoader
{
public:
	SLIB_INLINE MapDataLoaderList() {}

	Memory loadData(const String& type, const MapTileLocationi& location, const String& subPath);
	SLIB_INLINE Memory loadData(const String& type, const MapTileLocationi& location)
	{
		return loadData(type, location, String::null());
	}
public:
	List< Ref<MapDataLoader> > list;
};

// basePath/type/level/y/x.ext
class MapData_GenericFileLoader : public MapDataLoader
{
public:
	MapData_GenericFileLoader();
	MapData_GenericFileLoader(String basePath, String password, sl_uint32 packageDimension);

public:
	Memory loadData(const String& type, const MapTileLocationi& location, const String& subPath);

	SLIB_PROPERTY_SIMPLE(String, BasePath);
	SLIB_PROPERTY_SIMPLE(String, SecureFilePackagePassword);
	SLIB_PROPERTY_SIMPLE(sl_uint32, PackageDimension);

protected:
	Memory _readData(const String& packagePath, const String& filePath);
};

SLIB_MAP_NAMESPACE_END

#endif

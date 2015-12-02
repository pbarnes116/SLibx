#ifndef CHECKHEADER_SLIB_MAP_DATA
#define CHECKHEADER_SLIB_MAP_DATA

#include "definition.h"
#include "util.h"

#include <slib/core/object.h>
#include <slib/core/memory.h>
#include <slib/core/string.h>

SLIB_MAP_NAMESPACE_BEGIN

class SLIB_EXPORT MapDataLoader : public Object
{
protected:
	SLIB_INLINE MapDataLoader() {}

public:
	virtual Memory loadData(const String& type, const MapTileLocationi& location, sl_uint32 packageDimension, const String& subPath) = 0;
};

class SLIB_EXPORT MapDataLoaderList : public MapDataLoader
{
public:
	SLIB_INLINE MapDataLoaderList() {}

	Memory loadData(const String& type, const MapTileLocationi& location, sl_uint32 packageDimension, const String& subPath);

public:
	List< Ref<MapDataLoader> > list;
};

// basePath/type/level/y/x.ext
class SLIB_EXPORT MapData_GenericFileLoader : public MapDataLoader
{
public:
	MapData_GenericFileLoader();
	MapData_GenericFileLoader(String basePath, String password);

public:
	Memory loadData(const String& type, const MapTileLocationi& location, sl_uint32 packageDimension, const String& subPath);

	SLIB_PROPERTY_INLINE(String, BasePath);
	SLIB_PROPERTY_INLINE(String, SecureFilePackagePassword);

protected:
	Memory _readData(const String& packagePath, const String& filePath);
};

SLIB_MAP_NAMESPACE_END

#endif

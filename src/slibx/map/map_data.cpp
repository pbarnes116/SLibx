
#include "../../../inc/slibx/map/data.h"
#include "../../../inc/slibx/map/package.h"
#include "../../../inc/slibx/sfile/secure_file_pack.h"

#include <slib/core/file.h>

SLIB_MAP_NAMESPACE_BEGIN

Memory MapDataLoaderList::loadData(const String& type, const MapTileLocationi& location, sl_uint32 packageDimension, const String& subPath)
{
	ListLocker< Ref<MapDataLoader> > loaders(list);
	for (sl_size i = 0; i < loaders.count(); i++) {
		Ref<MapDataLoader> loader = loaders[i];
		if (loader.isNotNull()) {
			Memory ret = loader->loadData(type, location, packageDimension, subPath);
			if (ret.isNotEmpty()) {
				return ret;
			}
		}
	}
	return Memory::null();
}

MapData_GenericFileLoader::MapData_GenericFileLoader()
{
}

MapData_GenericFileLoader::MapData_GenericFileLoader(String basePath, String password)
{
	setBasePath(basePath);
	setSecureFilePackagePassword(password);
}

Memory MapData_GenericFileLoader::loadData(const String& type, const MapTileLocationi& location, sl_uint32 packageDimension, const String& _subPath)
{
	// map-standard-package
 	{
		MapPackage pkgReader(getSecureFilePackagePassword(), packageDimension, packageDimension);
		String subPath = _subPath;
		if (subPath.startsWith('/')) {
			subPath = subPath.substring(1); 
		} else {
			subPath = String::null();
		}
		Memory mem = pkgReader.read(getBasePath() + "/" + type, location, subPath);
		if (mem.isNotEmpty()) {
			return mem;
		}
 	}

	String prefix = getBasePath() + "/" + type + "/";
	String packagePath;
	String filePath;
	// generic style path
	{
		MapTilePath::makeGenericStylePath(location, &packagePath, &filePath);
		filePath += _subPath;
		Memory ret = _readData(prefix + packagePath, filePath);
		if (ret.isNotEmpty()) {
			return ret;
		}
	}
	// vw-style path
	{
		MapTilePath::makeVWStylePath(location, &packagePath, &filePath);
		filePath += _subPath;
		Memory ret = _readData(prefix + packagePath, filePath);
		if (ret.isNotEmpty()) {
			return ret;
		}
	}
	return Memory::null();
}

Memory MapData_GenericFileLoader::_readData(const String& packagePath, const String& filePath)
{
	String path = packagePath + "/" + filePath;
	if (File::exists(path)) {
		return File::readAllBytes(path);
	} else {
		path = packagePath + ".slp";
		if (File::exists(path)) {
			SecureFilePackage package;
			SecureFilePackage::OpenParam param;
			param.password = getSecureFilePackagePassword();
			if (package.open(path, param) == SecureFilePackage::errorOK) {
				SecureFilePackage::FileDesc desc;
				if (package.findFile(filePath, &desc)) {
					String fileName;
					Memory ret = package.readFile(desc.position);
					return ret;
				}
			}
		}
	}
	return Memory::null();
}
SLIB_MAP_NAMESPACE_END

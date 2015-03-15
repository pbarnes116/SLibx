
#include "data.h"
#include "package.h"
#include "../../slib/core/file.h"
#include "../sfile/secure_file_pack.h"

SLIB_MAP_NAMESPACE_START

Memory MapDataLoaderList::loadData(const String& type, const MapTileLocation& location, const String& subPath)
{
	ListLocker< Ref<MapDataLoader> > loaders(list);
	for (sl_size i = 0; i < loaders.count(); i++) {
		Ref<MapDataLoader> loader = loaders[i];
		if (loader.isNotNull()) {
			Memory ret = loader->loadData(type, location, subPath);
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

MapData_GenericFileLoader::MapData_GenericFileLoader(String basePath, String ext, String password)
{
	setBasePath(basePath);
	setExt(ext);
	setSecureFilePackagePassword(password);
}

Memory MapData_GenericFileLoader::loadData(const String& type, const MapTileLocation& location, const String& _subPath)
{
	// map-standard-package
 	{
		Ref<MapPackage> pkgReader = new MapPackage(_SLT("TestPackageKey"));
		Memory mem = pkgReader->read(getBasePath() + _SLT("/") + type, location, MapPackage::VWorldMapPackage);
		if (mem.isNotEmpty()) {
			return mem;
		}
 	}
	String prefix = getBasePath() + _SLT("/") + type + _SLT("/");
	String packagePath;
	String filePath;
	// generic style path
	{
		MapTilePath::makeGenericStylePath(location, &packagePath, &filePath);
		String subPath = _subPath;
		if (subPath.isNotEmpty()) {
			filePath += _SLT("/");
			filePath += subPath;
		} else {
			filePath += _SLT(".");
			filePath += getExt();
		}
		Memory ret = _readData(prefix + packagePath, filePath);
		if (ret.isNotEmpty()) {
			return ret;
		}
	}
	// vw-style path
	{
		MapTilePath::makeVWStylePath(location, &packagePath, &filePath);
		String subPath = _subPath;
		if (subPath.isNotEmpty()) {
			filePath += _SLT("/");
			filePath += subPath;
		} else {
			filePath += _SLT(".");
			filePath += getExt();
		}
		Memory ret = _readData(prefix + packagePath, filePath);
		if (ret.isNotEmpty()) {
			return ret;
		}
	}
	return Memory::null();
}

Memory MapData_GenericFileLoader::_readData(const String& packagePath, const String& filePath)
{
	String path = packagePath + _SLT("/") + filePath;
	if (File::exists(path)) {
		return File::readAllBytes(path);
	} else {
		path = packagePath + _SLT(".slp");
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

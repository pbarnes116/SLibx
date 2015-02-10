#ifndef CHECKHEADER_SLIB_MAP_PACKAGE_HEADER
#define CHECKHEADER_SLIB_MAP_PACKAGE_HEADER

#include "definition.h"
#include "../../slib/slib.h"
#include "./location.h"
SLIB_MAP_NAMESPACE_START

#define PACKAGE_ITEM_IDENTIFY 0xFEFF0823
#define PACKAGE_HEADER_SIZE	32
#define PACKAGE_IDENTIFY	_SLT("SMAP-PACKAGE V1.0")
#define PACKAGE_LOG_TAG		_SLT("Package Error")
class Package : public Object{

public:
	enum PackageType{
		None = 0
		, GoogleMapTile
		, VWorldMapTile
		, OpenStreetPOI
		, OpenStreetWay
		, VWorldPOI
	};
	struct PackageInformation {
		PackageType type;
		String typeName;
		String subFolderName;
		String pkgExtension;
	};

	SLIB_INLINE Package() 
	{
		m_pkgFile = Ref<File>::null();
		m_flagOpen = sl_false;
	}
	~Package()
	{
		close();
	}
private:

	void		create(const String& filePath);
	sl_bool		check();
	sl_int32	getOffset(sl_int32 x, sl_int32 y, sl_int32 zoom);
	sl_int32	getNextItemOffset(const Memory& item);
	Memory		getHeader();
	Memory		getItem(sl_int32 offset);
	Memory		getDataFromItem(const Memory& item);
	Memory		createItem(const Memory& data, sl_int32 oldItemOffset);
public:
	static PackageInformation* getPackageModel(sl_int32& outPackageCount);
	static PackageInformation getPackageModelInformation(Package::PackageType type);
	sl_bool open(const String& dirPath, const Package::PackageType& type, const MapLocation& loc);
	sl_bool open(const String& fileName);
	sl_bool write(const MapLocation& loc, const Memory& data);
	sl_bool write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data);
	Memory	read(const MapLocation& loc, sl_int32 version = 0);
	Memory	read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, sl_int32 version = 0);
	SLIB_INLINE sl_bool close()
	{
		sl_bool flag = m_pkgFile->close();
		m_pkgFile = Ref<File>::null();
		m_flagOpen = sl_false;
		return flag;
	}
	SLIB_INLINE Ref<File> getFileInstance() const
	{
		return m_pkgFile;
	}
	SLIB_INLINE sl_bool isOpened()
	{
		return m_flagOpen;
	}
private:
	Ref<File> m_pkgFile;
	sl_bool m_flagOpen;
};
SLIB_MAP_NAMESPACE_END
#endif

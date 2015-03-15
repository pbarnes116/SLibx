#ifndef CHECKHEADER_SLIB_MAP_PACKAGE
#define CHECKHEADER_SLIB_MAP_PACKAGE

#include "definition.h"

#include "util.h"

#include "../../slib/core/object.h"
#include "../../slib/core/file.h"
#include "../../slib/crypto/aes.h"
SLIB_MAP_NAMESPACE_START
class MapPackage : public Object
{
public:
	enum MapPackageType{
		MapPackageTypeNone = 0
		, GoogleMapPackage
		, VWorldMapPackage
		, VWorldMap3DPackage
		, OpenStreetMapPackage
	};
	SLIB_INLINE MapPackage()
	{
		m_flagOpen = sl_false;
	}

	~MapPackage()
	{
		close();
	}
public:

	static sl_bool write(const String& dirPath, const MapTileLocation& loc, const Memory& data, MapPackageType type);
	static Memory read(const String& dirPath, const MapTileLocation& loc, MapPackageType type);
	static Ref<AES256> getEncryptionInstance()
	{
		SLIB_SAFE_STATIC(slib::Ref<AES256>, _packageEncryptionInstance, new AES256);
		return _packageEncryptionInstance;
	}
	static SLIB_INLINE void setEncryptionKey(const String& key)
	{
		getEncryptionInstance()->setKey_SHA256(key);
	}
public:
	sl_bool open(const String& fileName, MapPackageType type);

	sl_bool write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data);
	Memory	read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom);

	SLIB_INLINE void close()
	{
		if (m_flagOpen) {
			m_pkgFile.setNull();
			m_flagOpen = sl_false;
		}
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
private:
	void		create(const String& filePath, MapPackageType type);
	sl_bool		check(MapPackageType type);
	Memory		getHeader(MapPackageType type);
	Memory		getNextDataFromItem(const Memory& item);
	Memory		createItem(const Memory& data, sl_int32 oldItemOffset);
	sl_int32	getItemOffset(sl_int32 x, sl_int32 y, sl_int32 zoom);
	Memory		getItem(sl_int32 offset);
	Memory		getDataFromItem(const Memory& item);
public:
	static String getPackageFilePathAndOffset(const MapTileLocation& location, MapPackageType type, sl_int32& outX, sl_int32& outY, sl_int32& zoom);
};

SLIB_MAP_NAMESPACE_END

#endif

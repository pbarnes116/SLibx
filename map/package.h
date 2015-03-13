#ifndef CHECKHEADER_SLIB_MAP_PACKAGE
#define CHECKHEADER_SLIB_MAP_PACKAGE

#include "definition.h"

#include "util.h"

#include "../../slib/core/object.h"
#include "../../slib/core/file.h"

SLIB_MAP_NAMESPACE_START
class MapPackage : public Object
{
public:
	SLIB_INLINE MapPackage()
	{
		m_flagOpen = sl_false;
	}

	~MapPackage()
	{
		close();
	}

public:
	sl_bool open(const String& fileName);

	sl_bool write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data);

	Memory	read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, sl_int32 version = 0);

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
	void		create(const String& filePath);
	sl_bool		check();
	sl_int32	getOffset(sl_int32 x, sl_int32 y, sl_int32 zoom);
	sl_int32	getNextItemOffset(const Memory& item);
	Memory		getHeader();
	Memory		getItem(sl_int32 offset);
	Memory		getDataFromItem(const Memory& item);
	Memory		createItem(const Memory& data, sl_int32 oldItemOffset);

public:
	static String makePackageFilePath(const MapTileLocation& location, const String& subFolderName, String* packagePath = sl_null, String* filePath = sl_null);
	static String makePackageFilePath(const LatLon& location, sl_int32 zoomLevel, const String& subFolderName, String* packagePath = sl_null, String* filePath = sl_null);
	static void getPackageFileOffsetXY(const MapTileLocation& location, sl_int32& blockOffsetX, sl_int32& blockOffsetY);
	static void getPackageFileOffsetXY(const LatLon& location, sl_int32 zoomLevel, sl_int32& blockOffsetX, sl_int32& blockOffsetY);
};

SLIB_MAP_NAMESPACE_END

#endif

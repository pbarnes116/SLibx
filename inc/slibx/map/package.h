#ifndef CHECKHEADER_SLIB_MAP_PACKAGE
#define CHECKHEADER_SLIB_MAP_PACKAGE

#include "definition.h"
#include "util.h"

#include <slib/core/map.h>
#include <slib/core/object.h>
#include <slib/core/file.h>
#include <slib/crypto/aes.h>

SLIB_MAP_NAMESPACE_BEGIN
class SLIB_EXPORT MapPackage : public Object
{
public:
	
	SLIB_INLINE MapPackage(const String& encKey)
	{
		m_flagOpen = sl_false;
		m_encryption.setKey_SHA256(encKey);
		m_nTilesXNum = m_nTilesYNum = 256;
	}

	SLIB_INLINE MapPackage(const String& encKey, sl_int32 xCount, sl_int32 yCount)
	{
		m_flagOpen = sl_false;
		m_encryption.setKey_SHA256(encKey);
		m_nTilesXNum = xCount;
		m_nTilesYNum = yCount;
	}

	~MapPackage()
	{
		close();
	}

public:
	sl_bool open(const String& fileName, sl_bool flagReadOnly);

	sl_bool write(const String& dirPath, const MapTileLocationi& loc, const Map<String, Memory>& itemData);
	Memory read(const String& dirPath, const MapTileLocationi& loc, const String& subName = String::null());

	sl_bool write(sl_int32 offsetX, sl_int32 offsetY, const Map<String, Memory>& itemData);
	Memory	read(sl_int32 offsetX, sl_int32 offsetY, const String& subName);

	String getPackageFilePathAndOffset(const MapTileLocationi& location, sl_int32& outX, sl_int32& outY);

	SLIB_INLINE void close()
	{
		if (m_flagOpen) {
			m_pkgFile.setNull();
			m_flagOpen = sl_false;
			m_nTilesXNum = m_nTilesYNum = 256;
		}
	}

	SLIB_INLINE void setXYItemCount(sl_int32 xItemCount, sl_int32 yItemCount)
	{
		m_nTilesXNum = xItemCount;
		m_nTilesYNum = yItemCount;
	}

	SLIB_INLINE void setEncryptionKey(const String& key)
	{
		m_encryption.setKey_SHA256(key);
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
	SafeRef<File> m_pkgFile;
	sl_bool m_flagOpen;
private:
	void		create(const String& filePath);
	sl_bool		checkHeader();
	Memory		getHeader();

	Memory		createItem(const Map<String, Memory>& itemData, sl_int32 oldItemOffset);
	Memory		getItem(sl_int32 offset);
	Map<String, Memory>		getDataFromItem(const Memory& item);

	sl_int32	getItemOffset(sl_int32 x, sl_int32 y);
	sl_int32	getTableOffset(sl_int32 offsetX, sl_int32 offsetY);

	AES m_encryption;
	sl_int32 m_nTilesXNum;
	sl_int32 m_nTilesYNum;
};

SLIB_MAP_NAMESPACE_END

#endif

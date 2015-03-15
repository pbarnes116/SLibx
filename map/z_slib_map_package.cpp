#include "package.h"
#include "../../slib/core/io.h"

#define PACKAGE_ITEM_IDENTIFY	0xFEFF0823
#define PACKAGE_HEADER_SIZE		32
#define PACKAGE_IDENTIFY		_SLT("SMAP-PACKAGE V1.0")

SLIB_MAP_NAMESPACE_START

void MapPackage::create(const String& filePath, MapPackageType type)
{
	m_pkgFile->setPath(filePath);
	String dirPath = File::getParentDirectoryPath(m_pkgFile->getPath());
	File dir(dirPath);
	dir.createDirectories();
	if (m_pkgFile->openForWrite()) {
		Memory header = getHeader(type);
		sl_int32 offsetTableSize = sizeof(sl_int32)* 256 * 256 * 8;
		SLIB_SCOPED_ARRAY(sl_int32, offsetTable, offsetTableSize);
		Base::zeroMemory(offsetTable, offsetTableSize * sizeof(sl_int32));
		m_pkgFile->write(header.getBuf(), header.getSize());
		m_pkgFile->write(offsetTable, offsetTableSize);
		m_pkgFile->seekToBegin();
		m_pkgFile->close();
	}
}

Memory MapPackage::getHeader(MapPackageType type)
{
	Memory ret = Memory::create(PACKAGE_HEADER_SIZE);
	MemoryWriter writer(ret);

	Base::zeroMemory(ret.getBuf(), ret.getSize());
	String packageIdentify = PACKAGE_IDENTIFY;
	Utf8StringBuffer buffer = packageIdentify.getUtf8();
	writer.write(buffer.sz, buffer.len);
	writer.writeInt32CVLI(type);
	return ret;
}

sl_bool MapPackage::check(MapPackageType type)
{
	sl_bool ret = sl_true;
	Memory originalHeader = getHeader(type);
	Memory header = Memory::create(PACKAGE_HEADER_SIZE);
	m_pkgFile->read(header.getBuf(), header.getSize());
	if (Base::compareMemory(originalHeader.getBuf(), header.getBuf(), PACKAGE_HEADER_SIZE) != 0) {
		ret = sl_false;
	}
	return ret;
}

sl_bool MapPackage::open(const String& filePath, MapPackageType type)
{
	close();
	m_pkgFile = new File();
	if (!File::exists(filePath)) {
		create(filePath, type);
		m_pkgFile->close();
	}
	sl_bool flagOpen = m_pkgFile->open(filePath, File::modeReadWriteNoTruncate);
	if (flagOpen && check(type)) {
		m_flagOpen = sl_true;
		return sl_true;
	}
	return sl_false;
}

static inline sl_int32 getTableOffset(sl_int32 offsetX, sl_int32 offsetY, sl_int32 zoom)
{
	sl_int32 ret = PACKAGE_HEADER_SIZE + (zoom * 256 * 256 + offsetY * 256 + offsetX) * sizeof(sl_int32);
	return ret;
}

sl_int32 MapPackage::getItemOffset(sl_int32 x, sl_int32 y, sl_int32 zoom)
{
	sl_int32 ret = -1;
	sl_int32 offsetToTable = getTableOffset(x, y, zoom);
	if (offsetToTable + 4 < m_pkgFile->getSize()) {
		m_pkgFile->seek(offsetToTable, File::positionBegin);
		m_pkgFile->read(&ret, sizeof(sl_int32));
		m_pkgFile->seekToBegin();
	}
	return ret;
}

Memory MapPackage::getItem(sl_int32 itemOffset)
{
	Memory ret;
	if (itemOffset + 4 < m_pkgFile->getSize()) {
		m_pkgFile->seek(itemOffset);
		sl_int32 identify = 0;
		m_pkgFile->read(&identify, sizeof(sl_int32));
		if (identify == PACKAGE_ITEM_IDENTIFY) {
			sl_int32 itemSize = 0;
			m_pkgFile->read(&itemSize, sizeof(sl_int32));
			ret = Memory::create(itemSize);
			m_pkgFile->read(ret.getBuf(), itemSize);
		}
	}
	return ret;
}

Memory MapPackage::getDataFromItem(const Memory& item)
{
	Memory ret;
	if (item.isNotEmpty()) {
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64();
		sl_int32 itemNextOffset = reader.readInt32();
		sl_size position = reader.getPosition();

		Memory encData = Memory::create(item.getSize() - position);
		reader.read(encData.getBuf(), encData.getSize());

		Memory decData = Memory::create(encData.getSize() + 256);
		sl_size decSize = m_encryption.decrypt_CBC_PKCS7Padding(encData.getBuf(), encData.getSize(), decData.getBuf());
		ret = Memory(decData.getBuf(), decSize);
	}
	return ret;
}

Memory MapPackage::createItem(const Memory& data, sl_int32 oldItemOffset)
{
	Memory ret;
	MemoryWriter writer;

	Memory encryption = Memory::create(data.getSize() + 256);
	sl_size encryptSize = m_encryption.encrypt_CBC_PKCS7Padding(data.getBuf(), data.getSize(), encryption.getBuf());
	sl_int64 curTime = Time::now().getSecondsCount();
	writer.writeInt32(PACKAGE_ITEM_IDENTIFY);

	sl_int64 pkgSize = sizeof(sl_int64) + sizeof(sl_int32) + encryptSize;
	writer.writeInt32((sl_uint32)pkgSize);
	writer.writeInt64(curTime);
	writer.writeInt32(oldItemOffset);
	writer.write(encryption.getBuf(), encryptSize);
	ret = writer.getData();

	return ret;
}

sl_bool MapPackage::write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data)
{
	sl_bool ret = sl_false;
	if (m_flagOpen && data.isNotEmpty()) {
		Memory packageItem = createItem(data, 0);
		sl_int32 itemPosition = (sl_int32)m_pkgFile->getSize();
		sl_int32 tblOffset = getTableOffset(offsetX, offsetY, offsetZoom);

		m_pkgFile->seek(tblOffset, File::positionBegin);
		sl_int32 nRet = m_pkgFile->write(&itemPosition, sizeof(sl_int32));
		if (nRet != 4) {
			return sl_false;
		}

		m_pkgFile->seekToEnd();
		nRet = m_pkgFile->write(packageItem.getBuf(), packageItem.getSize());
		if (nRet != packageItem.getSize()) {
			return sl_false;
		}
		ret = sl_true;
	}
	return ret;
}

Memory MapPackage::read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom)
{
	Memory ret;
	sl_int32 currentVersion = 0;
	sl_int32 itemOffset = getItemOffset(offsetX, offsetY, offsetZoom);
	if (itemOffset != -1) {
		ret = getDataFromItem(getItem(itemOffset));
	}
	return ret;
}

Memory MapPackage::read(const String& dirPath, const MapTileLocation& loc, MapPackageType type)
{	
	sl_int32 offsetX, offsetY, zoom;
	String path = dirPath + _SLT("/") + getPackageFilePathAndOffset(loc, type, offsetX, offsetY, zoom);
	Memory ret;
	if (open(path, type)) {
		ret = read(offsetX, offsetY, zoom);
	}
	return ret;
}

sl_bool MapPackage::write(const String& dirPath, const MapTileLocation& loc, const Memory& data, MapPackageType type)
{
	sl_int32 offsetX, offsetY, zoom;
	String path = dirPath + _SLT("/") +getPackageFilePathAndOffset(loc, type, offsetX, offsetY, zoom);
	sl_bool ret = sl_false;
	if (open(path, type)) {
		ret = write(offsetX, offsetY, zoom, data);
	}
	return ret;
}

String MapPackage::getPackageFilePathAndOffset(const MapTileLocation& location, MapPackageType type, sl_int32& outX, sl_int32& outY, sl_int32& zoom)
{
	String zoomFolderPath = "";
	zoom = location.level;
	sl_int32 tilesNum = 1;

	if (type == VWorldMap3DPackage || type == VWorldMapPackage) {
		zoom = location.level + 4;
	}
	if (zoom <= 20 && zoom > 12) {
		zoom = zoom - 13;
		zoomFolderPath = _SLT("P13-20");
	} else if (zoom <= 12 && zoom > 4) {
		zoom = zoom - 5;
		zoomFolderPath = _SLT("P05-12");
	} else {
		zoom = location.level;
		zoomFolderPath = _SLT("P00-04");
	}
	
	tilesNum = 2 << zoom;
	
	sl_int32 packageX = (sl_int32)(location.x / tilesNum);
	sl_int32 packageY = (sl_int32)(location.y / tilesNum);
	outX = (sl_int32)location.x % tilesNum;
	outY = (sl_int32)location.y % tilesNum;

	String filePath = String::fromInt32(packageX) + _SLT(".pkg");
	String pkgPath = zoomFolderPath + _SLT("/") + String::fromInt32(packageY);
	
	return pkgPath + _SLT("/") + filePath;
}

SLIB_MAP_NAMESPACE_END

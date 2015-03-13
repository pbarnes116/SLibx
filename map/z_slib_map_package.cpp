#include "package.h"
#include "../../slib/core/io.h"

#define PACKAGE_ITEM_IDENTIFY	0xFEFF0823
#define PACKAGE_HEADER_SIZE		32
#define PACKAGE_IDENTIFY		_SLT("SMAP-PACKAGE V1.0")

SLIB_MAP_NAMESPACE_START

void MapPackage::create(const String& filePath)
{
	m_pkgFile->setPath(filePath);
	String dirPath = File::getParentDirectoryPath(m_pkgFile->getPath());
	File dir(dirPath);
	dir.createDirectories();
	if (m_pkgFile->openForWrite()) {
		Memory header = getHeader();
		sl_int32 offsetTableSize = sizeof(sl_int32)* 256 * 256 * 8;
		SLIB_SCOPED_ARRAY(sl_int32, offsetTable, offsetTableSize);
		Base::zeroMemory(offsetTable, offsetTableSize * sizeof(sl_int32));
		m_pkgFile->write(header.getBuf(), header.getSize());
		m_pkgFile->write(offsetTable, offsetTableSize);
		m_pkgFile->seekToBegin();
		m_pkgFile->close();
	}
}

Memory MapPackage::getHeader()
{
	Memory ret = Memory::create(PACKAGE_HEADER_SIZE);
	Base::zeroMemory(ret.getBuf(), ret.getSize());
	String packageIdentify = PACKAGE_IDENTIFY;
	Utf8StringBuffer buffer = packageIdentify.getUtf8();
	ret.write(buffer.sz, buffer.len);
	return ret;
}

sl_bool MapPackage::check()
{
	sl_bool ret = sl_false;
	Memory originalHeader = getHeader();
	Memory header = Memory::create(PACKAGE_HEADER_SIZE);
	m_pkgFile->read(header.getBuf(), header.getSize());
	if (Base::compareMemory(originalHeader.getBuf(), header.getBuf(), PACKAGE_HEADER_SIZE) == 0) {
		ret = sl_true;
	}
	return ret;
}

sl_bool MapPackage::open(const String& filePath)
{
	close();
	m_pkgFile = new File();
	if (!File::exists(filePath)) {
		create(filePath);
		m_pkgFile->close();
	}
	sl_bool flagOpen = m_pkgFile->open(filePath, File::modeReadWriteNoTruncate);
	if (flagOpen && check()) {
		m_flagOpen = sl_true;
		return sl_true;
	}
	return sl_false;
}

sl_int32 MapPackage::getOffset(sl_int32 x, sl_int32 y, sl_int32 zoom)
{
	sl_int32 ret = -1;
	sl_int32 offsetToTable = PACKAGE_HEADER_SIZE + ((zoom -1) * 256 * 256 + y * 256 + x) * sizeof(sl_int32);
	if (offsetToTable + 4 < m_pkgFile->getSize()) {
		m_pkgFile->seek(offsetToTable, File::positionBegin);
		m_pkgFile->read(&ret, sizeof(sl_int32));
		m_pkgFile->seekToBegin();
	}
	return ret;
}

sl_int32 MapPackage::getNextItemOffset(const Memory& item)
{
	sl_int32 ret = -1;
	if (item.isNotEmpty()) {
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64();
		ret = reader.readInt32();
	}
	return ret;
}

Memory MapPackage::getDataFromItem(const Memory& item)
{
	Memory ret;
	if (item.isNotEmpty())
	{
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64();
		sl_int32 itemNextOffset = reader.readInt32();
		sl_int64 position = reader.getPosition();
		ret = Memory::create(item.getSize() - position);
		reader.read(ret.getBuf(), ret.getSize());
	}
	return ret;
}

Memory MapPackage::getItem(sl_int32 offset)
{
	Memory ret;
	if (offset + 4 < m_pkgFile->getSize()) {
		m_pkgFile->seek(offset);
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

Memory MapPackage::createItem(const Memory& data, sl_int32 oldItemOffset)
{
	Memory ret;
	MemoryWriter writer;
	sl_int64 curTime = Time::now().getSecondsCount();
	sl_int64 pkgSize = sizeof(sl_int64) + sizeof(sl_int32) + data.getSize(); //update version, next offset for old version
	writer.writeInt32(PACKAGE_ITEM_IDENTIFY);
	writer.writeInt32((sl_uint32)pkgSize);
	writer.writeInt64(curTime);
	writer.writeInt32(oldItemOffset);
	writer.write(data.getBuf(), data.getSize());
	ret = writer.getData();
	return ret;
}

sl_bool MapPackage::write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data)
{
	sl_bool ret = sl_false;
	if (m_flagOpen && data.isNotEmpty()) {
		if (offsetZoom > 0) {
			//sl_int32 offset = getOffset(offsetX, offsetY, offsetZoom);
			//if (offset != -1) {
				Memory packageItem = createItem(data, 0);
				sl_int32 itemPosition = (sl_int32)m_pkgFile->getSize();
				sl_int32 tblOffset = PACKAGE_HEADER_SIZE + ((offsetZoom - 1) * 256 * 256 + offsetY * 256 + offsetX) * sizeof(sl_int32);
				m_pkgFile->seek(tblOffset, File::positionBegin);
				m_pkgFile->write(&itemPosition, sizeof(sl_int32));
				m_pkgFile->seekToEnd();
				m_pkgFile->write(packageItem.getBuf(), packageItem.getSize());
				ret = sl_true;
			//}
		}
	}
	return ret;
}

Memory MapPackage::read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, sl_int32 version)
{
	Memory ret;
	sl_int32 currentVersion = 0;
	sl_int32 itemOffset = getOffset(offsetX, offsetY, offsetZoom);
	Memory item;
	while (itemOffset != -1 && currentVersion < version + 1) {
		item = getItem(itemOffset);
		itemOffset = getNextItemOffset(item);
		currentVersion++;
	}
	if (currentVersion == version + 1 && item.isNotEmpty()) {
		ret = getDataFromItem(item);
	}
	return ret;
}

Memory MapPackage::read(const MapTileLocation& loc)
{	
	sl_int32 offsetX, offsetY;
	getPackageFileOffsetXY(loc, offsetX, offsetY);
	sl_int32 zoom = loc.level;
	if (zoom <= 18 && zoom > 10) {
		zoom = zoom - 10;
	}
	else if (zoom <= 10 && zoom > 2) {
		zoom = (zoom - 2);
	}
	return read(offsetX, offsetY, zoom);
}

String MapPackage::makePackageFilePath(const MapTileLocation& location, const String& subFolderName, String* _packagePath /* = sl_null */, String* _filePath /* = sl_null */)
{
	String zoomFolderPath = "";
	sl_int32 tilesNum = 1 << location.level;
	sl_int32 blockX = (sl_int32)((location.x + 180.0) / 360.0 * tilesNum);
	sl_int32 blockY = (sl_int32)((location.y + 90.0) / 180.0 * tilesNum);
	sl_int32 fragmentNum = 1;
	if (location.level <= 18 && location.level > 10) {
		fragmentNum = 1 << (location.level - 10);
		zoomFolderPath = _SLT("P11-18");
	} else if (location.level <= 10 && location.level > 2) {
		fragmentNum = 1 << (location.level - 2);
		zoomFolderPath = _SLT("P03-10");
	} else {
		fragmentNum = 1 << (location.level);
		zoomFolderPath = _SLT("P00-02");
	}
	sl_int32 packageX = blockX / fragmentNum;
	sl_int32 packageY = blockY / fragmentNum;
	sl_int32 packageOffsetX = blockX % fragmentNum;
	sl_int32 packageOffsetY = blockY % fragmentNum;

	String filePath = String::fromInt32(packageX) + _SLT(".pkg");
	String pkgPath = zoomFolderPath + _SLT("/") + String::fromInt32(packageY);
	if (_packagePath) {
		*_packagePath = pkgPath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return pkgPath + _SLT("/") + filePath;
}

String MapPackage::makePackageFilePath(const LatLon& location, sl_int32 zoomLevel, const String& subFolderName, String* packagePath /* = sl_null */, String* filePath /* = sl_null */)
{
	MapTileLocation newLoc;
	newLoc.x = location.longitude;
	newLoc.y = location.latitude;
	newLoc.level = zoomLevel;
	return makePackageFilePath(newLoc, subFolderName, packagePath, filePath);
}

void MapPackage::getPackageFileOffsetXY(const MapTileLocation& location, sl_int32& blockOffsetX, sl_int32& blockOffsetY)
{
	sl_int32 tilesNum = 1 << location.level;
	sl_int32 blockX = (sl_int32)((location.x + 180.0f) / 360.0f * tilesNum);
	sl_int32 blockY = (sl_int32)((location.y + 90.0f) / 180.0f * tilesNum);
	sl_int32 fragmentNum = 1;
	if (location.level <= 18 && location.level > 10) {
		fragmentNum = 1 << (location.level - 10);
	} else if (location.level <= 10 && location.level > 2) {
		fragmentNum = 1 << (location.level - 2);
	}
	blockOffsetX = blockX % fragmentNum;
	blockOffsetY = blockY % fragmentNum;
}

void MapPackage::getPackageFileOffsetXY(const LatLon& location, sl_int32 zoomLevel, sl_int32& blockOffsetX, sl_int32& blockOffsetY)
{
	MapTileLocation newLoc;
	newLoc.x = location.longitude;
	newLoc.y = location.latitude;
	newLoc.level = zoomLevel;
	return getPackageFileOffsetXY(newLoc, blockOffsetX, blockOffsetY);
}

SLIB_MAP_NAMESPACE_END

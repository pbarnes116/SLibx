#include "package.h"
SLIB_MAP_NAMESPACE_START

Package::PackageInformation _g_package_info[] = {
	{ Package::None, _SLT(""), _SLT(""), _SLT("") },
	{ Package::GoogleMapTile, _SLT("Google Earth Tile"), _SLT("GTile"), _SLT(".pkg.gt") },
	{ Package::VWorldMapTile, _SLT("VWorld Tile"), _SLT("VTile"), _SLT(".pkg.vt") },
	{ Package::OpenStreetPOI, _SLT("OSM Node"), _SLT("OPoi"), _SLT(".pkg.opoi") },
	{ Package::OpenStreetWay, _SLT("OSM Way"), _SLT("OWay"), _SLT(".pkg.oway") }
};

Package::PackageInformation Package::getPackageModelInformation(Package::PackageType type)
{
	Package::PackageInformation ret;
	ret.type = Package::None;
	for (sl_int32 i = 0; i < sizeof(_g_package_info) / sizeof(Package::PackageInformation); i++) {
		if (_g_package_info[i].type == type) {
			ret = _g_package_info[i];
		}
	}
	return ret;
}

Package::PackageInformation* Package::getPackageModel(sl_int32& outPackageCount)
{
	outPackageCount = sizeof(_g_package_info) / sizeof(PackageInformation);
	return _g_package_info;
}

void Package::create(const String& filePath)
{
	String dirPath = File::getParentDirectoryPath(m_pkgFile->getPath());
	File dir(dirPath);
	dir.createDirectories();
	m_pkgFile->setPath(filePath);
	if (m_pkgFile->openForWrite()) {
		Memory header = getHeader();
		sl_int32 offsetTableSize = sizeof(sl_int32)* 256 * 256 * 8;
		sl_int32* offsetTable = (sl_int32*)Base::createMemory(offsetTableSize);
		Base::zeroMemory(offsetTable, offsetTableSize);
		m_pkgFile->write(header.getBuf(), header.getSize());
		m_pkgFile->write(offsetTable, offsetTableSize);
		m_pkgFile->seekToBegin();
		m_pkgFile->close();
	}
}

Memory Package::getHeader()
{
	Memory ret = Memory::create(PACKAGE_HEADER_SIZE);
	Base::zeroMemory(ret.getBuf(), ret.getSize());
	String packageIdentify = PACKAGE_IDENTIFY;
	Utf8StringBuffer buffer = packageIdentify.getUtf8();
	m_pkgFile->openForWrite();
	ret.write(buffer.sz, buffer.len);
	return ret;
}

sl_bool Package::check()
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

sl_bool Package::open(const String& dirPath, const Package::PackageType& type, const MapLocation& loc)
{
	Package::PackageInformation currentPkgInfo = getPackageModelInformation(type);
	if (currentPkgInfo.type == type && type != Package::None) {
		String pkgFilePath = dirPath + _SLT("/") + currentPkgInfo.subFolderName;
		if (loc.pkgLocation.folderType == MapLocation::MapPackageTypeLevel11_18) {
			pkgFilePath = pkgFilePath + _SLT("/") + "Z11-18";
		}
		else if (loc.pkgLocation.folderType == MapLocation::MapPackageTypeLevel3_10) {
			pkgFilePath = pkgFilePath + _SLT("/") + "Z3-10";
		}
		pkgFilePath = pkgFilePath + _SLT("/") + loc.pkgLocation.y + _SLT("/") + loc.pkgLocation.x + currentPkgInfo.pkgExtension;
		return open(pkgFilePath);
	}
	return sl_false;
}

sl_bool Package::open(const String& filePath)
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

sl_int32 Package::getOffset(sl_int32 x, sl_int32 y, sl_int32 zoom)
{
	sl_int32 ret = -1;
	sl_int32 offsetToTable = PACKAGE_HEADER_SIZE + (zoom * 256 * 256 + y * 256 + x) * sizeof(sl_int32);
	if (offsetToTable + 4 < m_pkgFile->getSize()) {
		m_pkgFile->seek(offsetToTable, File::positionBegin);
		m_pkgFile->read(&ret, sizeof(sl_int32));
		m_pkgFile->seekToBegin();
	}
	return ret;
}

sl_int32 Package::getNextItemOffset(const Memory& item)
{
	sl_int32 ret = -1;
	if (item.isNotEmpty()) {
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64();
		ret = reader.readInt32();
	}
	return ret;
}

Memory Package::getDataFromItem(const Memory& item)
{
	Memory ret;
	if (item.isNotEmpty())
	{
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64();
		sl_int64 itemNextOffset = reader.readInt32();
		sl_int32 position = reader.getPosition();
		ret = Memory::create(item.getSize() - position);
		reader.read(ret.getBuf(), ret.getSize());
	}
	return ret;
}

Memory Package::getItem(sl_int32 offset)
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

Memory Package::createItem(const Memory& data, sl_int32 oldItemOffset)
{
	Memory ret;
	MemoryWriter writer;
	sl_int64 curTime = Time::now().getSecondsCount();
	sl_int32 pkgSize = sizeof(sl_int64) + sizeof(sl_int32) + data.getSize(); //update version, next offset for old version
	writer.writeInt32(PACKAGE_ITEM_IDENTIFY);
	writer.writeInt32(pkgSize);
	writer.writeInt64(curTime);
	writer.writeInt32(oldItemOffset);
	writer.write(data.getBuf(), data.getSize());
	ret = writer.getData();
	return ret;
}

sl_bool Package::write(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, const Memory& data)
{
	sl_bool ret = sl_false;
	if (m_flagOpen && data.isNotEmpty()) {
		if (offsetZoom > 0) {
			sl_int32 offset = getOffset(offsetX, offsetY, offsetZoom);
			if (offset != -1) {
				Memory packageItem = createItem(data, offset);
				sl_int32 itemPosition = (sl_int32)m_pkgFile->getSize();
				sl_int32 tblOffset = PACKAGE_HEADER_SIZE + (offsetZoom * 256 * 256 + offsetY * 256 + offsetX) * sizeof(sl_int32);
				m_pkgFile->seek(tblOffset, File::positionBegin);
				m_pkgFile->write(&itemPosition, sizeof(sl_int32));
				m_pkgFile->seekToEnd();
				m_pkgFile->write(packageItem.getBuf(), packageItem.getSize());
				ret = sl_true;
			}
		}
	}
	return ret;
}

sl_bool Package::write(const MapLocation& loc, const Memory& data)
{
	sl_bool ret = sl_false;
	sl_int32 offsetZoom = 0;
	if (loc.pkgLocation.folderType == MapLocation::MapPackageTypeLevel11_18) {
		offsetZoom -= 11;
	}
	else if (loc.pkgLocation.folderType == MapLocation::MapPackageTypeLevel3_10) {
		offsetZoom -= 3;
	}
	ret = write(loc.pkgLocation.offsetX, loc.pkgLocation.offsetY, offsetZoom, data);
	return ret;
}

Memory Package::read(sl_int32 offsetX, sl_int32 offsetY, sl_int32 offsetZoom, sl_int32 version)
{
	Memory ret;
	sl_int32 currentVersion = 0;
	sl_int32 itemOffset = getOffset(offsetX, offsetY, offsetZoom);
	Memory item;
	if (itemOffset != -1 && currentVersion < version) {
		item = getItem(itemOffset);
		itemOffset = getNextItemOffset(item);
		currentVersion++;
	}
	if (currentVersion == version && item.isNotEmpty()) {
		ret = getDataFromItem(item);
	}
	return ret;
}

Memory Package::read(const MapLocation& loc, sl_int32 version)
{
	Memory ret;
	if (m_flagOpen) {
		ret = read(loc.pkgLocation.x, loc.pkgLocation.y, loc._zoom, version);
	}
	return ret;
}
SLIB_MAP_NAMESPACE_END
#include "../../../inc/slibx/map/package.h"

#include <slib/core/io.h>
#include <slib/core/scoped_pointer.h>

#define PACKAGE_ITEM_IDENTIFY	0xFEFF0823
#define PACKAGE_HEADER_SIZE		32
#define PACKAGE_IDENTIFY		"SMAP-PACKAGE V1.0"

SLIB_MAP_NAMESPACE_BEGIN

void MapPackage::create(const String& filePath)
{
	String dirPath = File::getParentDirectoryPath(filePath);
	File::createDirectories(dirPath);
	Ref<File> file = File::openForWrite(filePath);
	if (file.isNotNull()) {
		Memory header = getHeader();
		sl_int32 offsetTableSize = sizeof(sl_int32)* m_nTilesXNum * m_nTilesYNum;
		SLIB_SCOPED_BUFFER(sl_int32, 4096, offsetTable, offsetTableSize);
		Base::zeroMemory(offsetTable, offsetTableSize * sizeof(sl_int32));
		file->write(header.getBuf(), header.getSize());
		file->write(offsetTable, offsetTableSize);
		file->seekToBegin();
		file->close();
	}
}

Memory MapPackage::getHeader()
{
	Memory ret = Memory::create(PACKAGE_HEADER_SIZE);
	MemoryWriter writer(ret);

	Base::zeroMemory(ret.getBuf(), ret.getSize());
	String8 packageIdentify = PACKAGE_IDENTIFY;
	writer.write(packageIdentify.getBuf(), packageIdentify.getLength());
	writer.writeInt32CVLI(m_nTilesXNum);
	writer.writeInt32CVLI(m_nTilesYNum);
	return ret;
}

sl_bool MapPackage::checkHeader()
{
	Ref<File> file = m_pkgFile;
	if (file.isNotNull()) {
		Memory originalHeader = getHeader();
		Memory header = Memory::create(PACKAGE_HEADER_SIZE);
		file->read(header.getBuf(), header.getSize());
		if (Base::compareMemory(originalHeader.getBuf(), header.getBuf(), PACKAGE_HEADER_SIZE) == 0) {
			return sl_true;
		}
	}
	return sl_false;
}

sl_bool MapPackage::open(const String& filePath, sl_bool flagReadOnly)
{
	close();

	if (!File::exists(filePath) && !flagReadOnly) {
		create(filePath);
	}
	
	Ref<File> file;
	if (flagReadOnly) {
		file = File::open(filePath, fileMode_Read);
	} else {
		file = File::open(filePath, fileMode_RandomAccess);
	}
	
	if (file.isNotNull()) {
		m_pkgFile = file;
		if (checkHeader()) {
			m_flagOpen = sl_true;
			return sl_true;
		}
	}
	return sl_false;
}

sl_int32 MapPackage::getTableOffset(sl_int32 offsetX, sl_int32 offsetY)
{
	sl_int32 ret = PACKAGE_HEADER_SIZE + (offsetY * m_nTilesXNum + offsetX) * sizeof(sl_int32);
	return ret;
}

sl_int32 MapPackage::getItemOffset(sl_int32 x, sl_int32 y)
{
	sl_int32 ret = -1;
	Ref<File> file = m_pkgFile;
	if (file.isNotNull()) {
		sl_int32 offsetToTable = getTableOffset(x, y);
		if (offsetToTable + 4 < file->getSize()) {
			file->seek(offsetToTable, seekPosition_Begin);
			file->readInt32(&ret);
			file->seekToBegin();
		}
	}
	return ret;
}

Memory MapPackage::getItem(sl_int32 itemOffset)
{
	Memory ret;
	Ref<File> file = m_pkgFile;
	if (file.isNotNull()) {
		if (itemOffset + 4 < file->getSize()) {
			file->seek(itemOffset, seekPosition_Begin);
			sl_int32 identify = 0;
			file->readInt32(&identify);
			if (identify == PACKAGE_ITEM_IDENTIFY) {
				sl_int32 itemSize = 0;
				file->readInt32(&itemSize);
				ret = Memory::create(itemSize);
				file->read(ret.getBuf(), itemSize);
			}
		}
	}
	return ret;
}

static SLIB_INLINE void writeString(MemoryWriter& writer, const String& _str) {
	String8 str = _str;
	writer.writeUint32CVLI(str.getLength());
	if (str.getLength() > 0) {
		writer.write(str.getBuf(), str.getLength());
	}
}

static SLIB_INLINE String readString(MemoryReader& reader)
{
	sl_uint32 nameLen = reader.readUint32CVLI();
	String ret;
	if (nameLen > 0){
		SLIB_SCOPED_BUFFER(char, 1024, strName, nameLen);
		reader.read(strName, nameLen);
		ret = String::fromUtf8(strName, nameLen);
	}
	return ret;
}

static inline void writeItemData(MemoryWriter& writer, const String& key, const Memory& data)
{
	writeString(writer, key);
	writer.writeSizeCVLI(data.getSize());
	writer.write(data.getBuf(), data.getSize());
}

static inline Memory readItemData(MemoryReader& reader, String& key)
{
	Memory ret;
	key = readString(reader);
	sl_int32 itemSize = reader.readInt32CVLI();
	if (itemSize > 0) {
		ret = Memory(itemSize);
		reader.read(ret.getBuf(), itemSize);
	}
	
	return ret;
}

Map<String, Memory> MapPackage::getDataFromItem(const Memory& encData)
{
	Map<String, Memory> ret;
	Memory item;
	if (encData.isNotEmpty()) {
		Memory decData = Memory::create(encData.getSize() + 256);
		sl_size decSize = m_encryption.decrypt_CBC_PKCS7Padding(encData.getBuf(), encData.getSize(), decData.getBuf());
		item = Memory(decData.getBuf(), decSize);
	}
	if (item.isNotEmpty()) {
		MemoryReader reader(item);
		sl_int64 itemUpdateTime = reader.readInt64CVLI();
		SLIB_UNUSED(itemUpdateTime);
		sl_int32 itemNextOffset = reader.readInt32CVLI();
		SLIB_UNUSED(itemNextOffset);
		sl_int32 itemCount = reader.readInt32CVLI();
		for (sl_int32 i = 0; i < itemCount; i++) {
			String key;
			Memory itemData = readItemData(reader, key);
			if (itemData.isNotEmpty()) {
				ret.put(key, itemData);
			}
		}
	}
	return ret;
}

Memory MapPackage::createItem(const Map<String, Memory>& itemData, sl_int32 oldItemOffset)
{
	MemoryWriter writer;

	sl_int64 curTime = Time::now().getSecondsCount();

	writer.writeInt64CVLI(curTime);
	writer.writeInt32CVLI(oldItemOffset);
	
	sl_size itemCount = itemData.getCount();
	writer.writeSizeCVLI(itemCount);

	Iterator< Pair<String, Memory> > itemIter = itemData.iterator();
	Pair<String, Memory> pairValue;
	while (itemIter.next(&pairValue)) {
		writeItemData(writer, pairValue.key, pairValue.value);
	}
	
	Memory item = writer.getData();

	Memory encryption = Memory::create(item.getSize() + 256);
	sl_size encryptSize = m_encryption.encrypt_CBC_PKCS7Padding(item.getBuf(), item.getSize(), encryption.getBuf());

	Memory ret(encryption.getBuf(), encryptSize);
	return ret;
}

sl_bool MapPackage::write(sl_int32 offsetX, sl_int32 offsetY, const Map<String, Memory>& itemData)
{
	sl_bool ret = sl_false;
	Ref<File> file = m_pkgFile;
	if (file.isNotNull()) {
		if (m_flagOpen && itemData.getCount() > 0) {
			Memory packageItem = createItem(itemData, 0);
			sl_int32 itemPosition = (sl_int32)(file->getSize());
			sl_int32 tblOffset = getTableOffset(offsetX, offsetY);
			
			file->seek(tblOffset, seekPosition_Begin);
			if (!file->writeInt32(itemPosition)) {
				return sl_false;
			}
			
			file->seekToEnd();
			
			if (!file->writeUint32(PACKAGE_ITEM_IDENTIFY)) {
				return sl_false;
			}
			sl_uint32 packageSize = (sl_uint32)(packageItem.getSize());
			if (!file->writeUint32(packageSize)) {
				return sl_false;
			}
			
			if (file->writeFromMemory(packageItem) != packageItem.getSize()) {
				return sl_false;
			}
			ret = sl_true;
		}
	}
	return ret;
}

Memory MapPackage::read(sl_int32 offsetX, sl_int32 offsetY, const String& subName)
{
	Memory ret;
	sl_int32 itemOffset = getItemOffset(offsetX, offsetY);
	if (itemOffset != -1) {
		Map<String, Memory> items = getDataFromItem(getItem(itemOffset));
		Memory defaultItemValue;
		ret = items.getValue(subName, defaultItemValue);
	}
	return ret;
}

Memory MapPackage::read(const String& dirPath, const MapTileLocationi& loc, const String& subName)
{	
	sl_int32 offsetX, offsetY;
	String path = dirPath + "/" + getPackageFilePathAndOffset(loc, offsetX, offsetY);
	Memory ret;
	if (open(path, sl_true)) {
		ret = read(offsetX, offsetY, subName);
	}
	close();
	return ret;
}

sl_bool MapPackage::write(const String& dirPath, const MapTileLocationi& loc, const Map<String, Memory>& itemData)
{
	sl_int32 offsetX, offsetY;
	String path = dirPath + "/" +getPackageFilePathAndOffset(loc, offsetX, offsetY);
	sl_bool ret = sl_false;
	if (open(path, sl_false)) {
		ret = write(offsetX, offsetY, itemData);
	}
	close();
	return ret;
}

String MapPackage::getPackageFilePathAndOffset(const MapTileLocationi& location, sl_int32& outX, sl_int32& outY)
{
	String zoomFolderPath = "";

	sl_int32 packageX = location.x / m_nTilesXNum;
	sl_int32 packageY = location.y / m_nTilesYNum;
	outX = location.x % m_nTilesXNum;
	outY = location.y % m_nTilesYNum;

	String filePath = String::fromInt32(packageX) + ".pkg";
	String pkgPath = String::fromInt32(location.level) + "/" + String::fromInt32(packageY);
	
	return pkgPath + "/" + filePath;
}

SLIB_MAP_NAMESPACE_END

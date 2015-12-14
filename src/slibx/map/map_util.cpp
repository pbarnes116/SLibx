#include "../../../inc/slibx/map/util.h"
#include "../../../inc/slibx/map/data.h"

#include <slib/core/hashtable.h>
#include <slib/core/scoped_pointer.h>

SLIB_MAP_NAMESPACE_BEGIN
String MapTilePath::makeGenericStylePath(const MapTileLocationi& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += "/";
	packagePath += location.y;
	String filePath;
	filePath += location.x;
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + "/" + filePath;
}

String MapTilePath::makeVWStylePath(const MapTileLocationi& location, String* _packagePath, String* _filePath)
{
	String packagePath;
	packagePath += location.level;
	packagePath += "/";
	String y;
	if (location.level <= 11) {
		y = String::fromUint32(location.y, 10, 4);
	} else {
		y = String::fromUint32(location.y, 10, 8);
	}
	packagePath += y;
	String filePath = y;
	filePath += "_";
	if (location.level <= 10) {
		filePath += String::fromUint32(location.x, 10, 4);
	} else {
		filePath += String::fromUint32(location.x, 10, 8);
	}
	if (_packagePath) {
		*_packagePath = packagePath;
	}
	if (_filePath) {
		*_filePath = filePath;
	}
	return packagePath + "/" + filePath;
}

Ref<Image> MapPictureUtil::capturePictureFromTiles(IMapTileDataLoader* loader, sl_uint32 width, sl_uint32 height, sl_uint32 level, double X0, double Y0, double X1, double Y1, sl_bool flagOpaque, sl_bool& outFlagContainsAlpha)
{
	Ref<Image> ret;
	
	double la0 = Y0;
	double lo0 = X0;
	double la1 = Y1;
	double lo1 = X1;
	
	sl_int32 iTileLaBegin = (sl_int32)(la0);
	sl_int32 iTileLoBegin = (sl_int32)(lo0);
	sl_int32 iTileLaEnd = (sl_int32)(la1);
	sl_int32 iTileLoEnd = (sl_int32)(lo1);
	
	sl_bool flagExistOne = false;
	sl_bool flagExistEvery = true;

	sl_int32 widthSrc = (iTileLoEnd - iTileLoBegin + 1) * 256;
	sl_int32 heightSrc = (iTileLaEnd - iTileLaBegin + 1) * 256;
	if (widthSrc <= 0 || heightSrc <= 0) {
		return ret;
	}
	
	sl_uint32 n = widthSrc * heightSrc;
	SLIB_SCOPED_ARRAY(Color, colorsSrc, n);
	Base::zeroMemory(colorsSrc, n * 4);
	
	for (sl_int32 iTileLa = iTileLaBegin; iTileLa <= iTileLaEnd; iTileLa++) {
		for (sl_int32 iTileLo = iTileLoBegin; iTileLo <= iTileLoEnd; iTileLo++) {
			Memory mem = loader->loadData(MapTileLocationi(level, iTileLa, iTileLo), ".jpg");
			sl_bool flagExist = sl_false;
			if (mem.isNotNull()) {
				Ref<Image> image = Image::loadFromMemory(mem);
				if (image->getWidth() == 256 && image->getHeight() == 256) {
					flagExist = sl_true;
					Color* tile = image->getColors();
					for (int y = 0; y < 256; y++) {
						Base::copyMemory(colorsSrc + ((iTileLaEnd - iTileLa) * 256 + y) * widthSrc + (iTileLo - iTileLoBegin) * 256, tile + y * 256, 256 * 4);
					}
				}
			}
			if (flagExist) {
				flagExistOne = true;
			} else {
				flagExistEvery = false;
			}
		}
	}
	if (!flagExistOne) {
		return ret;
	}
	if (flagOpaque) {
		if (!flagExistEvery) {
			return ret;
		}
	}
	
	ret = Image::create(width, height);
	if (ret.isNull()) {
		return ret;
	}
	
	Color* colorsDst = ret->getColors();
	double rlo0 = (lo0 - iTileLoBegin) * width;
	double rla1 = heightSrc - (la0 - iTileLaBegin) * height;
	double rlo1 = (lo1 - iTileLoBegin) * width;
	double rla0 = heightSrc - (la1 - iTileLaBegin) * height;
	
	outFlagContainsAlpha = sl_false;
	// Parallelogram
	{
		for (sl_uint32 x = 0; x < width; x++) {
			for (sl_uint32 y = 0; y < height; y++) {
				double dx = rlo0 + x * (rlo1 - rlo0) / width;
				double dy = rla0 + y * (rla1 - rla0) / height;
				int sa = 0, sr = 0, sg = 0, sb = 0;
				if (dx >= 0 && dx < widthSrc && dy >= 0 && dy < heightSrc) {
					sl_uint32 sx = (sl_uint32)dx;
					sl_uint32 sy = (sl_uint32)dy;
					sl_uint32 t = sy * widthSrc + sx;
					int c00B = colorsSrc[t].b;
					int c00G = colorsSrc[t].g;
					int c00R = colorsSrc[t].r;
					int c00A = colorsSrc[t].a;
					int c01B;
					int c01G;
					int c01R;
					int c01A;
					if (dx < widthSrc - 1) {
						c01B = colorsSrc[t+1].b;
						c01G = colorsSrc[t+1].g;
						c01R = colorsSrc[t+1].r;
						c01A = colorsSrc[t+1].a;
					} else {
						c01B = c00B;
						c01G = c00G;
						c01R = c00R;
						c01A = c00A;
					}
					int c10B;
					int c10G;
					int c10R;
					int c10A;
					if (dy < heightSrc - 1) {
						c10B = colorsSrc[t+widthSrc].b;
						c10G = colorsSrc[t+widthSrc].g;
						c10R = colorsSrc[t+widthSrc].r;
						c10A = colorsSrc[t+widthSrc].a;
					} else {
						c10B = c00B;
						c10G = c00G;
						c10R = c00R;
						c10A = c00A;
					}
					int c11B;
					int c11G;
					int c11R;
					int c11A;
					if (dx < widthSrc - 1 && dy < heightSrc - 1) {
						c11B = colorsSrc[t+widthSrc+1].b;
						c11G = colorsSrc[t+widthSrc+1].g;
						c11R = colorsSrc[t+widthSrc+1].r;
						c11A = colorsSrc[t+widthSrc+1].a;
					} else {
						c11B = c00B;
						c11G = c00G;
						c11R = c00R;
						c11A = c00A;
					}
					int rx = (int)((dx - sx) * 256);
					int ry = (int)((dy - sy) * 256);
					sa = ((c00A * (255 - rx) * (255 - ry)) + (c01A * rx * (255 - ry)) + (c10A * (255 - rx) * ry) + (c11A * rx * ry)) / 65025;
					if (sa != 0) {
						sr = ((c00R * c00A * (255 - rx) * (255 - ry)) + (c01R * c01A * rx * (255 - ry)) + (c10R * c10A * (255 - rx) * ry) + (c11R * c11A * rx * ry)) / 65025 / sa;
						sg = ((c00G * c00A * (255 - rx) * (255 - ry)) + (c01G * c01A * rx * (255 - ry)) + (c10G * c10A * (255 - rx) * ry) + (c11G * c11A * rx * ry)) / 65025 / sa;
						sb = ((c00B * c00A * (255 - rx) * (255 - ry)) + (c01B * c01A * rx * (255 - ry)) + (c10B * c10A * (255 - rx) * ry) + (c11B * c11A * rx * ry)) / 65025 / sa;
					} else {
						sr = 0;
						sg = 0;
						sb = 0;
					}
				}
				Color c((sl_uint8)sr, (sl_uint8)sg, (sl_uint8)sb, (sl_uint8)sa);
				colorsDst[y * width + x] = c;
				if (c.a < 255) {
					outFlagContainsAlpha = sl_true;
				}
			}
		}
	}
	return ret;
}

SLIB_MAP_NAMESPACE_END

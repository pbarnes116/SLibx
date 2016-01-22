#ifndef CHECKHEADER_SLIB_MAP_OBJECT
#define CHECKHEADER_SLIB_MAP_OBJECT

#include "definition.h"

#include <slib/math/geograph.h>
#include <slib/render/texture.h>
#include <slib/graphics/freetype.h>

SLIB_MAP_NAMESPACE_BEGIN

class SLIB_EXPORT MapMarker : public Referable
{
public:
	SafeString key;

	GeoLocation location;
	SafeString text;
	Color textColor;
	SafeRef<FreeType> textFont;
	sl_uint32 textFontSize;

	Size iconSize;
	SafeRef<Texture> iconTexture;
	Rectangle iconTextureRectangle; // texture whole coordinate
	sl_bool flagVisible;

public:
	SafeRef<Texture> _textureText;

public:
	void invalidate()
	{
		_textureText.setNull();
	}
};


class SLIB_EXPORT MapIcon : public Referable
{
public:
	SafeString key;
	GeoLocation location;
	Size iconSize;
	sl_real rotationAngle;
	SafeRef<Texture> iconTexture;
	Rectangle iconTextureRectangle; // texture whole coordinate
	
	sl_bool flagVisible;

};


class SLIB_EXPORT MapPolygon : public Referable
{
public:
	String key;
	SafeList<GeoLocation> points;
	Color color;
	float width;
};

SLIB_MAP_NAMESPACE_END

#endif

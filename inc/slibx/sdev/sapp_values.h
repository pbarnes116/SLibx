#ifndef CHECKHEADER_SLIB_SDEV_SAPP_VALUES
#define CHECKHEADER_SLIB_SDEV_SAPP_VALUES

#include "definition.h"

#include <slib/core/string.h>
#include <slib/math/vector2.h>
#include <slib/math/vector3.h>
#include <slib/math/vector4.h>
#include <slib/graphics/constants.h>
#include <slib/graphics/color.h>
#include <slib/graphics/font.h>
#include <slib/ui/constants.h>

SLIB_SDEV_NAMESPACE_BEGIN

class SAppStringValue
{
public:
	sl_bool flagDefined;
	sl_bool flagReferResource;
	String valueOrName;
	
public:
	SAppStringValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppDimensionValue
{
public:
	sl_bool flagDefined;
	enum {
		FILL, WRAP, WEIGHT, PX, SW, SH, SMIN, SMAX, VW, VH, VMIN, VMAX, SP
	};
	int unit;
	sl_real amount;
	
public:
	SAppDimensionValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
	sl_bool checkGlobal();
	
	sl_bool checkSP();
	
	sl_bool checkPosition();
	
	sl_bool checkSize();
	
	sl_bool checkScalarSize();
	
	sl_bool checkMargin();
	
	sl_bool checkForWindow();
	
	sl_bool checkForWindowSize();
	
	sl_bool checkForRootViewPosition();
	
	sl_bool checkForRootViewSize();
	
	sl_bool checkForRootViewScalarSize();
	
	sl_bool checkForRootViewMargin();

	sl_bool isNeededOnLayoutFunction();
	
	static sl_bool isRelativeUnit(int unit);
	
	static sl_bool isGlobalUnit(int unit);
	
	static sl_bool isViewportUnit(int unit);
	
};

class SAppDimensionFloatValue : public SAppDimensionValue
{
public:
	String getAccessString();
	
};

class SAppBooleanValue
{
public:
	sl_bool flagDefined;
	sl_bool value;
	
public:
	SAppBooleanValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);

};

class SAppFloatValue
{
public:
	sl_bool flagDefined;
	float value;
	
public:
	SAppFloatValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppInt32Value
{
public:
	sl_bool flagDefined;
	sl_int32 value;
	
public:
	SAppInt32Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppUint32Value
{
public:
	sl_bool flagDefined;
	sl_uint32 value;
	
public:
	SAppUint32Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppInt64Value
{
public:
	sl_bool flagDefined;
	sl_int64 value;
	
public:
	SAppInt64Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppUint64Value
{
public:
	sl_bool flagDefined;
	sl_uint64 value;
	
public:
	SAppUint64Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppColorValue
{
public:
	sl_bool flagDefined;
	Color value;
	
public:
	SAppColorValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppVector2Value
{
public:
	sl_bool flagDefined;
	Vector2 value;
	
public:
	SAppVector2Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppVector3Value
{
public:
	sl_bool flagDefined;
	Vector3 value;
	
public:
	SAppVector3Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppVector4Value
{
public:
	sl_bool flagDefined;
	Vector4 value;
	
public:
	SAppVector4Value();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppVisibilityValue
{
public:
	sl_bool flagDefined;
	Visibility value;
	
public:
	SAppVisibilityValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppPenStyleValue
{
public:
	sl_bool flagDefined;
	PenStyle value;
	
public:
	SAppPenStyleValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

// scrollBars (none, horizontal, vertical, both)
class SAppScrollBarsValue
{
public:
	sl_bool horizontalScrollBar;
	sl_bool verticalScrollBar;
	
public:
	SAppScrollBarsValue();
	
public:
	sl_bool parse(const String& str);
	
};

class SAppNameValue
{
public:
	sl_bool flagDefined;
	String value;
	
public:
	SAppNameValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppDrawableValue
{
public:
	sl_bool flagDefined;
	sl_bool flagNull;
	sl_bool flagColor;
	
	Color color;
	String resourceName;
	
	sl_bool flagWhole;
	sl_real x;
	sl_real y;
	sl_real width;
	sl_real height;
	
	enum {
		FUNC_NONE, FUNC_NINEPATCH, FUNC_THREEPATCH_HORIZONTAL, FUNC_THREEPATCH_VERTICAL
	};
	int func;
	SAppDimensionValue patchLeftWidthDst;
	SAppDimensionValue patchRightWidthDst;
	SAppDimensionValue patchTopHeightDst;
	SAppDimensionValue patchBottomHeightDst;
	sl_real patchLeftWidth;
	sl_real patchRightWidth;
	sl_real patchTopHeight;
	sl_real patchBottomHeight;
	
public:
	SAppDrawableValue();
	
public:
	String getAccessString();
	
	String getImageAccessString();
	
	sl_bool parse(const String& str);

	sl_bool parseWhole(const String& str);
	
};

class SAppMenuValue
{
public:
	sl_bool flagDefined;
	sl_bool flagNull;
	String resourceName;
	
public:
	SAppMenuValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);

};

class SAppAlignLayoutValue
{
public:
	sl_bool flagDefined;
	sl_bool flagAlignParent;
	String referingView;
	
public:
	SAppAlignLayoutValue();
	
public:
	sl_bool parse(const String& str);
	
};

// Scrolling (horizontal, vertical, both)
class SAppScrollingValue
{
public:
	sl_bool horizontal;
	sl_bool vertical;
	
public:
	SAppScrollingValue();
	
public:
	sl_bool parse(const String& str);
	
};

class SAppLayoutOrientationValue
{
public:
	sl_bool flagDefined;
	LayoutOrientation value;
	
public:
	SAppLayoutOrientationValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppAlignmentValue
{
public:
	sl_bool flagDefined;
	Alignment value;
	
public:
	SAppAlignmentValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

class SAppScaleModeValue
{
public:
	sl_bool flagDefined;
	ScaleMode value;
	
public:
	SAppScaleModeValue();
	
public:
	String getAccessString();
	
	sl_bool parse(const String& str);
	
};

SLIB_SDEV_NAMESPACE_END

#endif

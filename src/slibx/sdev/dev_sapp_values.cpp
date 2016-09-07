#include "../../../inc/slibx/sdev/sapp_values.h"

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/variant.h>

SLIB_SDEV_NAMESPACE_BEGIN

/************************************************
				String
************************************************/

SAppStringValue::SAppStringValue()
: flagDefined(sl_false), flagReferResource(sl_false)
{
}

String SAppStringValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::String::null()";
	}
	if (valueOrName.isNull()) {
		return "slib::String::null()";
	}
	if (flagReferResource) {
		return String::format("string::%s::get()", valueOrName);
	} else {
		return String::format("\"%s\"", valueOrName.applyBackslashEscapes(sl_true, sl_false, sl_true));
	}
}

sl_bool SAppStringValue::parse(const String& _str)
{
	String str = _str;
	if (str.isNull()) {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str.startsWith('@')) {
		str = str.substring(1);
		if (str == "null") {
			flagReferResource = sl_false;
			valueOrName = String::null();
			flagDefined = sl_true;
			return sl_true;
		}
		if (str.startsWith('@')) {
			flagReferResource = sl_false;
			valueOrName = str;
		} else {
			if (str.startsWith("string/")) {
				str = str.substring(7);
			} else {
				return sl_false;
			}
			str = str.trim();
			if (!(SDevUtil::checkName(str.getData(), str.getLength()))) {
				return sl_false;
			}
			flagReferResource = sl_true;
			valueOrName = str;
		}
	} else {
		flagReferResource = sl_false;
		valueOrName = str;
	}
	flagDefined = sl_true;
	return sl_true;
}


/************************************************
				Dimension
************************************************/

SAppDimensionValue::SAppDimensionValue()
: flagDefined(sl_false), unit(PX), amount(0)
{
}

String SAppDimensionValue::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	switch (unit) {
		case CUSTOM:
			return String::format("%ff*getCustomUnitLength()", amount);
		case PX:
			return String::format("%ff", amount);
		case SW:
			return String::format("%ff*slib::UIResource::getScreenWidth()", amount);
		case SH:
			return String::format("%ff*slib::UIResource::getScreenHeight()", amount);
		case SMIN:
			return String::format("%ff*slib::UIResource::getScreenMinimum()", amount);
		case SMAX:
			return String::format("%ff*slib::UIResource::getScreenMaximum()", amount);
		case VW:
			return String::format("%ff*CONTENT_WIDTH", amount);
		case VH:
			return String::format("%ff*CONTENT_HEIGHT", amount);
		case VMIN:
			return String::format("%ff*SLIB_MIN(CONTENT_WIDTH, CONTENT_HEIGHT)", amount);
		case VMAX:
			return String::format("%ff*SLIB_MAX(CONTENT_WIDTH, CONTENT_HEIGHT)", amount);
	}
	return "0";
}

sl_bool SAppDimensionValue::parse(const String& _str)
{
	do {
		String str = _str.trim();
		if (str.isEmpty()) {
			flagDefined = sl_false;
			unit = PX;
			amount = 0;
			return sl_true;
		}
		str = str.toLower();
		if (str == "fill" || str == "*") {
			amount = 1;
			unit = FILL;
			break;
		}
		if (str == "wrap") {
			amount = 1;
			unit = WRAP;
			break;
		}
		const sl_char8* sz = str.getData();
		sl_size len = str.getLength();
		float f;
		sl_reg ret = String::parseFloat(&f, sz, 0, len);
		if (ret == SLIB_PARSE_ERROR) {
			return sl_false;
		}
		sl_size pos = ret;
		while (pos < len) {
			if (SLIB_CHAR_IS_SPACE_TAB(sz[pos])) {
				pos++;
			} else {
				break;
			}
		}
		if (pos >= len) {
			amount = f;
			unit = CUSTOM;
			break;
		}
		sl_bool flagPercent = sl_false;
		if (sz[pos] == '%') {
			flagPercent = sl_true;
			f /= 100;
			pos++;
		}
		while (pos < len) {
			if (SLIB_CHAR_IS_SPACE_TAB(sz[pos])) {
				pos++;
			} else {
				break;
			}
		}
		if (pos == len) {
			amount = f;
			if (flagPercent) {
				unit = WEIGHT;
			} else {
				unit = CUSTOM;
			}
			break;
		} else if (pos + 1 == len) {
			if (sz[pos] == '*') {
				amount = f;
				unit = FILL;
				break;
			}
		} else if (pos + 2 == len) {
			if (sz[pos] == 's') {
				if (sz[pos + 1] == 'w') {
					amount = f;
					unit = SW;
					break;
				} else if (sz[pos + 1] == 'h') {
					amount = f;
					unit = SH;
					break;
				}
			} else if (sz[pos] == 'v') {
				if (sz[pos + 1] == 'w') {
					amount = f;
					unit = VW;
					break;
				} else if (sz[pos + 1] == 'h') {
					amount = f;
					unit = VH;
					break;
				}
			} else if (sz[pos] == 'p') {
				if (sz[pos + 1] == 'x') {
					if (!flagPercent) {
						amount = f;
						unit = PX;
						break;
					}
				}
			}
		} else if (pos + 4 == len) {
			if (sz[pos] == 's') {
				if (sz[pos + 1] == 'm') {
					if (sz[pos + 2] == 'a' && sz[pos + 3] == 'x') {
						amount = f;
						unit = SMAX;
						break;
					} else if (sz[pos + 2] == 'i' && sz[pos + 3] == 'n') {
						amount = f;
						unit = SMIN;
						break;
					}
				}
			} else if (sz[pos] == 'v') {
				if (sz[pos + 1] == 'm') {
					if (sz[pos + 2] == 'a' && sz[pos + 3] == 'x') {
						amount = f;
						unit = VMAX;
						break;
					} else if (sz[pos + 2] == 'i' && sz[pos + 3] == 'n') {
						amount = f;
						unit = VMIN;
						break;
					}
				}
			}
		}
		return sl_false;
		
	} while (0);
	
	if (amount == 0) {
		unit = PX;
	}
	flagDefined = sl_true;
	return sl_true;
}

sl_bool SAppDimensionValue::checkGlobal()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == CUSTOM) {
		unit = PX;
	}
	return isGlobalUnit(unit);
}

sl_bool SAppDimensionValue::checkCustomUnit()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == CUSTOM) {
		unit = PX;
	}
	return amount > 0 && !isRelativeUnit(unit);
}

sl_bool SAppDimensionValue::checkPosition()
{
	if (!flagDefined) {
		return sl_true;
	}
	return !isRelativeUnit(unit);
}

sl_bool SAppDimensionValue::checkSize()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == FILL || unit == WRAP) {
		return sl_true;
	}
	return amount >= 0;
}

sl_bool SAppDimensionValue::checkScalarSize()
{
	if (!flagDefined) {
		return sl_true;
	}
	return amount >= 0 && !isRelativeUnit(unit);
}

sl_bool SAppDimensionValue::checkMargin()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == WEIGHT) {
		return amount >= 0;
	}
	return checkPosition();
}

sl_bool SAppDimensionValue::checkForWindow()
{
	return checkGlobal();
}

sl_bool SAppDimensionValue::checkForWindowSize()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == CUSTOM) {
		unit = PX;
	}
	return amount >= 0 && isGlobalUnit(unit);
}

sl_bool SAppDimensionValue::checkForRootViewPosition()
{
	return checkGlobal();
}

sl_bool SAppDimensionValue::checkForRootViewSize()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == FILL || unit == WRAP) {
		return sl_true;
	}
	if (unit == CUSTOM) {
		unit = PX;
	}
	if (unit == WEIGHT || isGlobalUnit(unit)) {
		return amount >= 0;
	}
	return sl_false;
}

sl_bool SAppDimensionValue::checkForRootViewMargin()
{
	if (!flagDefined) {
		return sl_true;
	}
	if (unit == WEIGHT) {
		return amount >= 0;
	}
	return isGlobalUnit(unit);
}

sl_bool SAppDimensionValue::isNeededOnLayoutFunction()
{
	if (!flagDefined) {
		return sl_false;
	}
	return isViewportUnit(unit);
}

sl_bool SAppDimensionValue::isRelativeUnit(int unit)
{
	return unit == FILL || unit == WRAP || unit == WEIGHT;
}

sl_bool SAppDimensionValue::isGlobalUnit(int unit)
{
	return unit == PX || unit == SW || unit == SH || unit == SMIN || unit == SMAX;
}

sl_bool SAppDimensionValue::isViewportUnit(int unit)
{
	return unit == VW || unit == VH || unit == VMIN || unit == VMAX || unit == CUSTOM;
}

/************************************************
					Boolean
************************************************/

SAppBooleanValue::SAppBooleanValue()
: flagDefined(sl_false), value(sl_false)
{
}

String SAppBooleanValue::getAccessString()
{
	if (!flagDefined) {
		return "sl_false";
	}
	if (value) {
		return "sl_true";
	} else {
		return "sl_false";
	}
}

sl_bool SAppBooleanValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "true") {
		value = sl_true;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "false") {
		value = sl_false;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
				Float
************************************************/

SAppFloatValue::SAppFloatValue()
: flagDefined(sl_false), value(0)
{
}

String SAppFloatValue::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	return String::format("%ff", value);
}

sl_bool SAppFloatValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	float f;
	if (str.parseFloat(&f)) {
		value = f;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
				Integer
************************************************/

SAppInt32Value::SAppInt32Value()
: flagDefined(sl_false), value(0)
{
}

String SAppInt32Value::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	return String::format("%d", value);
}

sl_bool SAppInt32Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_int32 f;
	if (str.parseInt32(10, &f)) {
		value = f;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}

SAppUint32Value::SAppUint32Value()
: flagDefined(sl_false), value(0)
{
}

String SAppUint32Value::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	return String::format("%d", value);
}

sl_bool SAppUint32Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_uint32 f;
	if (str.parseUint32(10, &f)) {
		value = f;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}

SAppInt64Value::SAppInt64Value()
: flagDefined(sl_false), value(0)
{
}

String SAppInt64Value::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	return String::format("%d", value);
}

sl_bool SAppInt64Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_int64 f;
	if (str.parseInt64(10, &f)) {
		value = f;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}

SAppUint64Value::SAppUint64Value()
: flagDefined(sl_false), value(0)
{
}

String SAppUint64Value::getAccessString()
{
	if (!flagDefined) {
		return "0";
	}
	return String::format("%d", value);
}

sl_bool SAppUint64Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_uint64 f;
	if (str.parseUint64(10, &f)) {
		value = f;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}

/************************************************
				Color
************************************************/

SAppColorValue::SAppColorValue()
: flagDefined(sl_false)
{	
}

String SAppColorValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::Color::zero()";
	}
	return String::format("slib::Color(%d, %d, %d, %d)", value.r, value.g, value.b, value.a);
}

sl_bool SAppColorValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	Color c;
	if (Color::parse(str, &c)) {
		value = c;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
				Vector2
************************************************/

SAppVector2Value::SAppVector2Value()
: flagDefined(0)
{
}

String SAppVector2Value::getAccessString()
{
	if (!flagDefined) {
		return "slib::Vector2::zero()";
	}
	return String::format("slib::Vector2(%ff, %ff)", value.x, value.y);
}

sl_bool SAppVector2Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_size pos = 0;
	sl_size len = str.getLength();
	const sl_char8* sz = str.getData();
	float f[2];
	for (sl_size i = 0; i < 2; i++) {
		sl_reg iRet = String::parseFloat(f+i, sz, pos, len);
		if (iRet == SLIB_PARSE_ERROR) {
			return sl_false;
		}
		pos = iRet;
		for (; pos < len; pos++) {
			if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
				break;
			}
		}
		if (i == 1) {
			if (pos == len) {
				value.x = f[0];
				value.y = f[1];
				flagDefined = sl_true;
				return sl_true;
			} else {
				return sl_false;
			}
		} else {
			if (pos >= len) {
				return sl_false;
			}
			if (sz[pos] != ',') {
				return sl_false;
			}
			pos++;
			for (; pos < len; pos++) {
				if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
					break;
				}
			}
			if (pos >= len) {
				return sl_false;
			}
		}
	}
	return sl_false;
}


/************************************************
				Vector3
************************************************/

SAppVector3Value::SAppVector3Value()
: flagDefined(sl_false)
{
}

String SAppVector3Value::getAccessString()
{
	if (!flagDefined) {
		return "slib::Vector3::zero()";
	}
	return String::format("slib::Vector3(%ff, %ff, %ff)", value.x, value.y, value.z);
}

sl_bool SAppVector3Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_size pos = 0;
	sl_size len = str.getLength();
	const sl_char8* sz = str.getData();
	float f[3];
	for (sl_size i = 0; i < 3; i++) {
		sl_reg iRet = String::parseFloat(f+i, sz, pos, len);
		if (iRet == SLIB_PARSE_ERROR) {
			return sl_false;
		}
		pos = iRet;
		for (; pos < len; pos++) {
			if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
				break;
			}
		}
		if (i == 2) {
			if (pos == len) {
				value.x = f[0];
				value.y = f[1];
				value.z = f[2];
				flagDefined = sl_true;
				return sl_true;
			} else {
				return sl_false;
			}
		} else {
			if (pos >= len) {
				return sl_false;
			}
			if (sz[pos] != ',') {
				return sl_false;
			}
			pos++;
			for (; pos < len; pos++) {
				if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
					break;
				}
			}
			if (pos >= len) {
				return sl_false;
			}
		}
	}
	return sl_false;
}


/************************************************
				Vector4
************************************************/

SAppVector4Value::SAppVector4Value()
: flagDefined(sl_false)
{
}

String SAppVector4Value::getAccessString()
{
	if (!flagDefined) {
		return "slib::Vector4::zero()";
	}
	return String::format("slib::Vector4(%ff, %ff, %ff, %ff)", value.x, value.y, value.z, value.w);
}

sl_bool SAppVector4Value::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	sl_size pos = 0;
	sl_size len = str.getLength();
	const sl_char8* sz = str.getData();
	float f[4];
	for (sl_size i = 0; i < 4; i++) {
		sl_reg iRet = String::parseFloat(f+i, sz, pos, len);
		if (iRet == SLIB_PARSE_ERROR) {
			return sl_false;
		}
		pos = iRet;
		for (; pos < len; pos++) {
			if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
				break;
			}
		}
		if (i == 3) {
			if (pos == len) {
				value.x = f[0];
				value.y = f[1];
				value.z = f[2];
				value.w = f[3];
				flagDefined = sl_true;
				return sl_true;
			} else {
				return sl_false;
			}
		} else {
			if (pos >= len) {
				return sl_false;
			}
			if (sz[pos] != ',') {
				return sl_false;
			}
			pos++;
			for (; pos < len; pos++) {
				if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
					break;
				}
			}
			if (pos >= len) {
				return sl_false;
			}
		}
	}
	return sl_false;
}


/************************************************
				Visibility
************************************************/

SAppVisibilityValue::SAppVisibilityValue()
: flagDefined(sl_false), value(Visibility::Visible)
{
}

String SAppVisibilityValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::Visibility::Visible";
	}
	if (value == Visibility::Gone) {
		return "slib::Visibility::Gone";
	} else if (value == Visibility::Hidden) {
		return "slib::Visibility::Hidden";
	} else {
		return "slib::Visibility::Visible";
	}
}

sl_bool SAppVisibilityValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "visible") {
		value = Visibility::Visible;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "gone") {
		value = Visibility::Gone;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "hidden") {
		value = Visibility::Hidden;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
			PenStyle
************************************************/

SAppPenStyleValue::SAppPenStyleValue()
: flagDefined(sl_false), value(PenStyle::Solid)
{
}

String SAppPenStyleValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::PenStyle::Solid";
	}
	if (value == PenStyle::Dot) {
		return "slib::PenStyle::Dot";
	} else if (value == PenStyle::Dash) {
		return "slib::PenStyle::Dash";
	} else if (value == PenStyle::DashDot) {
		return "slib::PenStyle::DashDot";
	} else if (value == PenStyle::DashDotDot) {
		return "slib::PenStyle::DashDotDot";
	} else {
		return "slib::PenStyle::Solid";
	}
}

sl_bool SAppPenStyleValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "solid") {
		value = PenStyle::Solid;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "dot") {
		value = PenStyle::Dot;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "dash") {
		value = PenStyle::Dash;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "dashdot") {
		value = PenStyle::DashDot;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "dashdotdot") {
		value = PenStyle::DashDotDot;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
				ScrollBars
************************************************/

SAppScrollBarsValue::SAppScrollBarsValue()
: horizontalScrollBar(sl_false), verticalScrollBar(sl_false)
{
}

sl_bool SAppScrollBarsValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		horizontalScrollBar = sl_false;
		verticalScrollBar = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "none") {
		horizontalScrollBar = sl_false;
		verticalScrollBar = sl_false;
		return sl_true;
	} else if (str == "horizontal") {
		horizontalScrollBar = sl_true;
		verticalScrollBar = sl_false;
		return sl_true;
	} else if (str == "vertical") {
		horizontalScrollBar = sl_false;
		verticalScrollBar = sl_true;
		return sl_true;
	} else if (str == "both") {
		horizontalScrollBar = sl_true;
		verticalScrollBar = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
				Name
************************************************/

SAppNameValue::SAppNameValue()
: flagDefined(sl_false)
{
}

String SAppNameValue::getAccessString()
{
	return value;
}

sl_bool SAppNameValue::parse(const String& _str)
{
	String str = _str.trim();
	str = str.trim();
	if (str.isEmpty()) {
		value.setEmpty();
		flagDefined = sl_false;
		return sl_true;
	}
	if (!(SDevUtil::checkName(str.getData(), str.getLength()))) {
		return sl_false;
	}
	value = str;
	flagDefined = sl_true;
	return sl_true;
}

/************************************************
				Drawable
************************************************/

SAppDrawableValue::SAppDrawableValue()
: flagDefined(sl_false), flagNull(sl_false), flagWhole(sl_false), func(FUNC_NONE)
{
}

String SAppDrawableValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::Ref<slib::Drawable>::null()";
	}
	if (flagNull) {
		return "slib::Ref<slib::Drawable>::null()";
	}
	String str;
	if (flagWhole) {
		str = String::format("drawable::%s::get()", resourceName);
	} else {
		str = String::format("slib::Drawable::createSubDrawable(drawable::%s::get(), %ff, %ff, %ff, %ff)", resourceName, x, y, width, height);
	}
	if (func == FUNC_NINEPATCH) {
		str = String::format("slib::NinePatchDrawable::create(%s, %s, %s, %s, %s, %ff, %ff, %ff, %ff)", ninePatch_leftWidthDst.getAccessString(), ninePatch_rightWidthDst.getAccessString(), ninePatch_topHeightDst.getAccessString(), ninePatch_bottomHeightDst.getAccessString(), str, ninePatch_leftWidth, ninePatch_rightWidth, ninePatch_topHeight, ninePatch_bottomHeight);
	}
	return str;
}

String SAppDrawableValue::getImageAccessString()
{
	if (!flagDefined || flagNull || !flagWhole || func != FUNC_NONE) {
		return "slib::Ref<slib::Image>::null()";
	}
	return String::format("drawable::%s::getImage()", resourceName);
}

sl_bool SAppDrawableValue::parse(const String& _str)
{
	String str = _str;
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str == "@null") {
		flagDefined = sl_true;
		flagNull = sl_true;
		return sl_true;
	}
	if (!(str.startsWith("@drawable/"))) {
		return sl_false;
	}
	str = str.substring(10);
	
	sl_char8* sz = str.getData();
	sl_size len = str.getLength();
	sl_size pos = 0;
	
	while (pos < len) {
		if (SLIB_CHAR_IS_C_NAME(sz[pos])) {
			pos++;
		} else {
			break;
		}
	}
	if (!(SDevUtil::checkName(sz, pos))) {
		return sl_false;
	}
	
	resourceName = String(sz, pos);
	flagNull = sl_false;
	flagWhole = sl_true;
	func = FUNC_NONE;
	
	for (; pos < len; pos++) {
		if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
			break;
		}
	}
	
	if (pos < len) {
		
		if (sz[pos] == '[') {
			
			pos++;
			
			float f[4];
			for (sl_size i = 0; i < 4; i++) {
				for (; pos < len; pos++) {
					if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
						break;
					}
				}
				if (pos >= len) {
					return sl_false;
				}
				sl_bool flagPlus = sl_false;
				if (sz[pos] == '+') {
					if (i >= 2) {
						flagPlus = sl_true;
						pos++;
						for (; pos < len; pos++) {
							if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
								break;
							}
						}
						if (pos >= len) {
							return sl_false;
						}
					} else {
						return sl_false;
					}
				}
				sl_reg iRet = String::parseFloat(f+i, sz, pos, len);
				if (iRet == SLIB_PARSE_ERROR) {
					return sl_false;
				}
				if (i >= 2) {
					if (!flagPlus) {
						f[i] -= f[i-2];
					}
					if (f[i] < 0) {
						return sl_false;
					}
				}
				pos = iRet;
				for (; pos < len; pos++) {
					if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
						break;
					}
				}
				if (pos >= len) {
					return sl_false;
				}
				if (i == 3) {
					if (sz[pos] != ']') {
						return sl_false;
					}
				} else {
					if (sz[pos] != ',') {
						return sl_false;
					}
				}
				pos++;
			}
			
			flagWhole = sl_false;
			x = f[0];
			y = f[1];
			width = f[2];
			height = f[3];
		}
		
		for (; pos < len; pos++) {
			if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
				break;
			}
		}
		
		if (sz[pos] == ',') {
			pos++;
			for (; pos < len; pos++) {
				if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
					break;
				}
			}
			if (pos >= len) {
				return sl_false;
			}
			if (Base::equalsMemory(sz + pos, "nine-patch", 10)) {
				func = FUNC_NINEPATCH;
				pos += 10;
			} else {
				return sl_false;
			}
			if (func == FUNC_NINEPATCH) {
				for (; pos < len; pos++) {
					if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
						break;
					}
				}
				if (pos >= len) {
					return sl_false;
				}
				if (sz[pos] != '(') {
					return sl_false;
				}
				pos++;
				
				SAppDimensionValue f[8];
				sl_size i = 0;
				for (; i < 8; i++) {
					for (; pos < len; pos++) {
						if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
							break;
						}
					}
					if (pos >= len) {
						return sl_false;
					}
					sl_size posStart = pos;
					for (; pos < len; pos++) {
						if (!(SLIB_CHAR_IS_ALNUM(sz[pos]) || sz[pos] == '.' || sz[pos] == '%' || sz[pos] == '\t' || sz[pos] == ' ')) {
							break;
						}
					}
					String s = String(sz + posStart, pos - posStart);
					if (s.isEmpty()) {
						return sl_false;
					}
					if (!(f[i].parse(s))) {
						return sl_false;
					}
					if (!(f[i].flagDefined)) {
						return sl_false;
					}
					for (; pos < len; pos++) {
						if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
							break;
						}
					}
					if (pos >= len) {
						return sl_false;
					}
					if (sz[pos] != ',') {
						i++;
						break;
					}
					pos++;
				}
				if (pos >= len) {
					return sl_false;
				}
				if (sz[pos] != ')') {
					return sl_false;
				}
				pos++;
				if (i != 4 && i != 8) {
					return sl_false;
				}
				if (i == 4) {
					f[4] = f[0];
					f[5] = f[1];
					f[6] = f[2];
					f[7] = f[3];
				}
				if (f[4].unit != SAppDimensionValue::CUSTOM || f[4].amount < 0) {
					return sl_false;
				}
				if (f[5].unit != SAppDimensionValue::CUSTOM || f[5].amount < 0) {
					return sl_false;
				}
				if (f[6].unit != SAppDimensionValue::CUSTOM || f[6].amount < 0) {
					return sl_false;
				}
				if (f[7].unit != SAppDimensionValue::CUSTOM || f[7].amount < 0) {
					return sl_false;
				}
				if (!(f[0].checkGlobal()) || f[0].amount < 0) {
					return sl_false;
				}
				if (!(f[1].checkGlobal()) || f[1].amount < 0) {
					return sl_false;
				}
				if (!(f[2].checkGlobal()) || f[2].amount < 0) {
					return sl_false;
				}
				if (!(f[3].checkGlobal()) || f[3].amount < 0) {
					return sl_false;
				}
				ninePatch_leftWidthDst = f[0];
				ninePatch_rightWidthDst = f[1];
				ninePatch_topHeightDst = f[2];
				ninePatch_bottomHeightDst = f[3];
				ninePatch_leftWidth = f[4].amount;
				ninePatch_rightWidth = f[5].amount;
				ninePatch_topHeight = f[6].amount;
				ninePatch_bottomHeight = f[7].amount;
			}
		}
		
		for (; pos < len; pos++) {
			if (!(SLIB_CHAR_IS_SPACE_TAB(sz[pos]))) {
				return sl_false;
			}
		}
		
		if (pos < len) {
			return sl_false;
		}
		
	}
	
	flagDefined = sl_true;
	
	return sl_true;
	
}

sl_bool SAppDrawableValue::parseWhole(const String& _str)
{
	String str = _str;
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str == "@null") {
		flagDefined = sl_true;
		flagNull = sl_true;
		return sl_true;
	}
	if (!(str.startsWith("@drawable/"))) {
		return sl_false;
	}
	str = str.substring(10).trim();
	if (!(SDevUtil::checkName(str.getData(), str.getLength()))) {
		return sl_false;
	}
	func = FUNC_NONE;
	flagWhole = sl_true;
	resourceName = str;
	flagNull = sl_false;
	flagDefined = sl_true;
	return sl_true;
}

/************************************************
				Menu
************************************************/

SAppMenuValue::SAppMenuValue()
: flagDefined(sl_false), flagNull(sl_false)
{
}

String SAppMenuValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::Ref<slib::Menu>::null()";
	}
	if (flagNull) {
		return "slib::Ref<slib::Menu>::null()";
	}
	return String::format("menu::%s::get()->root", resourceName);
}

sl_bool SAppMenuValue::parse(const String& _str)
{
	String str = _str;
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str == "@null") {
		flagDefined = sl_true;
		flagNull = sl_true;
		return sl_true;
	}
	if (!(str.startsWith("@menu/"))) {
		return sl_false;
	}
	str = str.substring(6).trim();
	if (!(SDevUtil::checkName(str.getData(), str.getLength()))) {
		return sl_false;
	}
	resourceName = str;
	flagNull = sl_false;
	flagDefined = sl_true;
	return sl_true;
}


/************************************************
				AlignLayout
************************************************/

SAppAlignLayoutValue::SAppAlignLayoutValue()
: flagDefined(sl_false), flagAlignParent(sl_false)
{
}

sl_bool SAppAlignLayoutValue::parse(const String& _str)
{
	String str = _str.trim();
	str = str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str == "false") {
		flagDefined = sl_false;
		return sl_true;
	}
	if (str == "true") {
		flagAlignParent = sl_true;
		flagDefined = sl_true;
		return sl_true;
	}
	if (!(SDevUtil::checkName(str.getData(), str.getLength()))) {
		return sl_false;
	}
	referingView = str;
	flagAlignParent = sl_false;
	flagDefined = sl_true;
	return sl_true;
}


/************************************************
				Scrolling
************************************************/

SAppScrollingValue::SAppScrollingValue()
: horizontal(sl_false), vertical(sl_false)
{
}

sl_bool SAppScrollingValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		horizontal = sl_false;
		vertical = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "horizontal") {
		horizontal = sl_true;
		vertical = sl_false;
		return sl_true;
	} else if (str == "vertical") {
		horizontal = sl_false;
		vertical = sl_true;
		return sl_true;
	} else if (str == "both") {
		horizontal = sl_true;
		vertical = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
			LayoutOrientation
************************************************/

SAppLayoutOrientationValue::SAppLayoutOrientationValue()
: flagDefined(sl_false), value(LayoutOrientation::Vertical)
{
}

String SAppLayoutOrientationValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::LayoutOrientation::Vertical";
	}
	if (value == LayoutOrientation::Horizontal) {
		return "slib::LayoutOrientation::Horizontal";
	} else {
		return "slib::LayoutOrientation::Vertical";
	}
}

sl_bool SAppLayoutOrientationValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "horizontal") {
		value = LayoutOrientation::Horizontal;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "vertical") {
		value = LayoutOrientation::Vertical;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


/************************************************
					Alignment
************************************************/

SAppAlignmentValue::SAppAlignmentValue()
: flagDefined(sl_false), value(Alignment::Center)
{
}

String SAppAlignmentValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::LayoutOrientation::Vertical";
	}
	switch (value) {
		case Alignment::TopLeft:
			return "slib::Alignment::TopLeft";
		case Alignment::TopCenter:
			return "slib::Alignment::TopCenter";
		case Alignment::TopRight:
			return "slib::Alignment::TopRight";
		case Alignment::MiddleLeft:
			return "slib::Alignment::MiddleLeft";
		case Alignment::MiddleCenter:
			return "slib::Alignment::MiddleCenter";
		case Alignment::MiddleRight:
			return "slib::Alignment::MiddleRight";
		case Alignment::BottomLeft:
			return "slib::Alignment::BottomLeft";
		case Alignment::BottomCenter:
			return "slib::Alignment::BottomCenter";
		case Alignment::BottomRight:
			return "slib::Alignment::BottomRight";
	}
	return "slib::Alignment::MiddleCenter";
}

sl_bool SAppAlignmentValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	Alignment v = 0;
	ListLocker<String> items(str.split("|"));
	for (sl_size i = 0; i < items.count; i++) {
		String item = items[i].trim();
		if (item.isEmpty()) {
			return sl_false;
		}
		if (item == "top") {
			v |= Alignment::Top;
		} else if (item == "middle") {
		} else if (item == "bottom") {
			v |= Alignment::Bottom;
		} else if (item == "left") {
			v |= Alignment::Left;
		} else if (item == "center") {
		} else if (item == "right") {
			v |= Alignment::Right;
		} else {
			return sl_false;
		}
	}
	flagDefined = sl_true;
	value = v;
	return sl_true;
}


/************************************************
			ScaleMode
************************************************/

SAppScaleModeValue::SAppScaleModeValue()
: flagDefined(sl_false), value(ScaleMode::None)
{
}

String SAppScaleModeValue::getAccessString()
{
	if (!flagDefined) {
		return "slib::ScaleMode::None";
	}
	switch (value) {
		case ScaleMode::Stretch:
			return "slib::ScaleMode::Stretch";
		case ScaleMode::Contain:
			return "slib::ScaleMode::Contain";
		case ScaleMode::Cover:
			return "slib::ScaleMode::Cover";
		default:
			break;
	}
	return "slib::ScaleMode::None";
}

sl_bool SAppScaleModeValue::parse(const String& _str)
{
	String str = _str.trim();
	if (str.isEmpty()) {
		flagDefined = sl_false;
		return sl_true;
	}
	str = str.toLower();
	if (str == "stretch") {
		value = ScaleMode::Stretch;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "contain") {
		value = ScaleMode::Contain;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "cover") {
		value = ScaleMode::Cover;
		flagDefined = sl_true;
		return sl_true;
	} else if (str == "none") {
		value = ScaleMode::None;
		flagDefined = sl_true;
		return sl_true;
	}
	return sl_false;
}


SLIB_SDEV_NAMESPACE_END

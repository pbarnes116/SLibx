#include "../../../inc/slibx/sdev/util.h"

SLIB_SDEV_NAMESPACE_BEGIN

template <class CT>
sl_bool _SDevUtil_checkName(const CT* sz, sl_reg len)
{
	if (len == 0) {
		return sl_false;
	}
	CT ch = sz[0];
	if (!(SLIB_CHAR_IS_ALPHA(ch)) && ch != '_') {
		return sl_false;
	}
	if (len < 0) {
		for (sl_size i = 1; ; i++) {
			ch = sz[i];
			if (ch == 0) {
				break;
			}
			if (!(SLIB_CHAR_IS_C_NAME(ch))) {
				return sl_false;
			}
		}
	} else {
		for (sl_reg i = 1; i < len; i++) {
			ch = sz[i];
			if (!(SLIB_CHAR_IS_C_NAME(ch))) {
				return sl_false;
			}
		}
	}
	return sl_true;
}

sl_bool SDevUtil::checkName(const sl_char8* sz, sl_reg len)
{
	return _SDevUtil_checkName(sz, len);
}

sl_bool SDevUtil::checkName(const sl_char16* sz, sl_reg len)
{
	return _SDevUtil_checkName(sz, len);
}

String SDevUtil::generateBytesArrayDefinition(const void* data, sl_size size, sl_size countPerLine, sl_size tabCount)
{
	if (size == 0) {
		return String::getEmpty();
	}
	sl_size len = size * 5 - 1;
	sl_size nLines;
	if (countPerLine == 0) {
		countPerLine = size;
		nLines = 1;
	} else {
		nLines = size / countPerLine;
		if (size % countPerLine) {
			nLines++;
		}
		len += 2 * (nLines - 1);
	}
	len += tabCount * nLines;
	String s = String::allocate(len);
	if (s.isEmpty()) {
		return s;
	}
	sl_char8* sz = s.getData();
	sl_size pos = 0;
	sl_size col = 0;
	for (sl_size i = 0; i < size; i++) {
		if (i > 0) {
			sz[pos++] = ',';
		}
		if (col == countPerLine) {
			col = 0;
			sz[pos++] = '\r';
			sz[pos++] = '\n';
		}
		if (col == 0) {
			for (sl_size iTab = 0; iTab < tabCount; iTab++) {
				sz[pos++] = '\t';
			}
		}
		sz[pos++] = '0';
		sz[pos++] = 'x';
		sl_uint32 n = ((sl_uint8*)data)[i];
		sz[pos++] = _StringConv_radixPatternLower[(n >> 4) & 15];
		sz[pos++] = _StringConv_radixPatternLower[n & 15];
		col++;
	}
	sz[pos++] = 0;
	return s;
}

SLIB_SDEV_NAMESPACE_END

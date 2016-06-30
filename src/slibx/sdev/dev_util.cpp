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

SLIB_SDEV_NAMESPACE_END

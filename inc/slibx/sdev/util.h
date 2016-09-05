#ifndef CHECKHEADER_SLIB_SDEV_UTIL
#define CHECKHEADER_SLIB_SDEV_UTIL

#include "definition.h"

#include <slib/core/string.h>

SLIB_SDEV_NAMESPACE_BEGIN

class SDevUtil
{
public:
	static sl_bool checkName(const sl_char8* sz, sl_reg len = -1);
	
	static sl_bool checkName(const sl_char16* sz, sl_reg len = -1);
	
	static String generateBytesArrayDefinition(const void* data, sl_size size, sl_size countPerLine = 0, sl_size tabCount = 0);
	
};

SLIB_SDEV_NAMESPACE_END

#endif

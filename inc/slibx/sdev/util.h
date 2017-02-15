#ifndef CHECKHEADER_SLIB_SDEV_UTIL
#define CHECKHEADER_SLIB_SDEV_UTIL

#include "definition.h"

#include <slib/core/string.h>

namespace slib
{

	class SDevUtil
	{
	public:
		static sl_bool checkName(const sl_char8* sz, sl_reg len = -1);
		
		static sl_bool checkName(const sl_char16* sz, sl_reg len = -1);
		
		static String generateBytesArrayDefinition(const void* data, sl_size size, sl_size countPerLine = 0, sl_size tabCount = 0);
		
	};

}

#endif

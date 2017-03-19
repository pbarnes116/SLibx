/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
		
		static void applyCopyrightNoticeToSourceFile(const String& pathFile, const String& copyrightNotice);
		
		static void applyCopyrightNoticeToAllSourceFilesInPath(const String& pathDir, const String& copyrightNotice);
		
	};

}

#endif

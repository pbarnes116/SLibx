/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slibx/sdev/util.h"

#include <slib/core/file.h>

namespace slib
{

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
	
	void SDevUtil::applyCopyrightNoticeToSourceFile(const String& pathFile, const String& copyrightNotice)
	{
		Memory mem = File::readAllBytes(pathFile);
		if (mem.isNotNull()) {
			char* sz = (char*)(mem.getData());
			sl_size n = mem.getSize();
			sl_size posEndCopyright = 0;
			if (n > 8 && sz[0] == '/' && sz[1] == '*') {
				const char t[] = "Copyright";
				sl_size m = sizeof(t) - 1;
				sl_bool flagFoundCopyright = sl_false;
				sl_bool flagFoundEnd = sl_false;
				sl_size i = 2;
				for (; i < n; i++) {
					if (!flagFoundCopyright) {
						if (i + m <= n) {
							if (Base::compareMemory((sl_int8*)(sz + i), (sl_int8*)t, m) == 0) {
								flagFoundCopyright = sl_true;
							}
						}
					}
					if (i + 6 <= n) {
						if (sz[i] == '*' && sz[i+1] == '/' && sz[i+2] == '\r' && sz[i+3] == '\n' && sz[i+4] == '\r' && sz[i+5] == '\n') {
							flagFoundEnd = sl_true;
							break;
						}
					}
				}
				if (flagFoundEnd && flagFoundCopyright) {
					posEndCopyright = i + 6;
				}
			}
			MemoryBuffer out;
			out.addStatic(copyrightNotice.getData(), copyrightNotice.getLength());
			out.addStatic("\r\n\r\n", 4);
			out.addStatic((char*)(mem.getData()) + posEndCopyright, n - posEndCopyright);
			mem = out.merge();
			File::writeAllBytes(pathFile, mem);
		}
	}
	
	void SDevUtil::applyCopyrightNoticeToAllSourceFilesInPath(const String& pathDir, const String& copyrightNotice)
	{
		for (auto& file : File::getAllDescendantFiles(pathDir)) {
			String name = File::getFileNameOnly(file);
			if (name.isNotEmpty() && !(name.startsWith('.'))) {
				applyCopyrightNoticeToSourceFile(pathDir + "/" + file, copyrightNotice);
			}
		}
	}

}

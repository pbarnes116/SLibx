/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_STREAMER_CODEC
#define CHECKHEADER_SLIB_STREAMER_CODEC

#include "definition.h"

#include "graph.h"

#include <slib/media/codec_opus.h>

namespace slib
{
	
	namespace streamer
	{
	
		class AudioOpusEncodeFilter : public Filter
		{
		public:
			SLIB_INLINE AudioOpusEncodeFilter(const Ref<OpusEncoder>& encoder)
			{
				m_encoder = encoder;
			}
			
			List<Packet> filter(const Packet& input);
			
		private:
			Ref<OpusEncoder> m_encoder;
		};
		
		class AudioOpusDecodeFilter : public Filter
		{
		public:
			SLIB_INLINE AudioOpusDecodeFilter(const Ref<OpusDecoder>& decoder)
			{
				m_decoder = decoder;
				setMaxSamplesPerFrame(1600);
			}
			
			List<Packet> filter(const Packet& input);
			
		public:
			SLIB_PROPERTY(sl_uint32, MaxSamplesPerFrame);
			
		private:
			Ref<OpusDecoder> m_decoder;
			
		};
		
	}
	
}

#endif

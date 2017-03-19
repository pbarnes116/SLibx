/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_STREAMER_MEDIA
#define CHECKHEADER_SLIB_STREAMER_MEDIA

#include "definition.h"

#include "graph.h"

#include <slib/media/audio_player.h>
#include <slib/media/audio_recorder.h>

namespace slib
{
	
	namespace streamer
	{
		
		struct AudioRecordSourceParam : public AudioRecorderParam
		{
			sl_uint32 nSamplesPerFrame;
		};
		
		class AudioRecordSource : public Source
		{
		public:
			SLIB_INLINE AudioRecordSource()
			{
				setVolume(1.0f);
			}
			
		public:
			SLIB_PROPERTY(float, Volume);
			
		public:
			
			static Ref<AudioRecordSource> create(const AudioRecordSourceParam& param);
			
		};
		
		struct AudioPlaySinkParam : public AudioPlayerBufferParam
		{
			Ref<AudioPlayer> player;
		};
		
		class AudioPlaySink : public Sink
		{
		public:
			SLIB_INLINE AudioPlaySink() {}
			
			static Ref<AudioPlaySink> create(const AudioPlaySinkParam& param);
			
		};
		
	}
	
}

#endif

#include "../../../inc/slibx/streamer/codec.h"

#include <slib/core/scoped.h>

namespace slib
{
	
	namespace streamer
	{
		
		List<Packet> AudioOpusEncodeFilter::filter(const Packet& input)
		{
			Ref<OpusEncoder> encoder = m_encoder;
			if (encoder.isNull()) {
				return sl_null;
			}
			if (input.format != Packet::formatAudio_PCM_S16
				|| input.data.getSize() % 2 != 0
				|| input.audioParam.nChannels != encoder->getChannelsCount()
				|| input.audioParam.nSamplesPerSecond != encoder->getSamplesCountPerSecond()) {
				return List<Packet>::null();
			}
			AudioData data;
			data.format = AudioFormat::Int16_Mono;
			data.data = input.data.getData();
			data.count = (sl_uint32)(input.data.getSize()) / 2;
			Memory dataOut = encoder->encode(data);
			if (dataOut.isNull()) {
				return sl_null;
			}
			Packet output;
			output.format = Packet::formatAudio_OPUS;
			output.audioParam = input.audioParam;
			output.data = dataOut;
			return List<Packet>::createFromElement(output);
		}
		
		List<Packet> AudioOpusDecodeFilter::filter(const Packet& input)
		{
			Ref<OpusDecoder> decoder = m_decoder;
			if (decoder.isNull()) {
				return sl_null;
			}
			if (input.format != Packet::formatAudio_OPUS && input.format != Packet::formatRaw) {
				return sl_null;
			}
			if (input.format == Packet::formatAudio_OPUS) {
				if (input.audioParam.nChannels != decoder->getChannelsCount()
					|| input.audioParam.nSamplesPerSecond != decoder->getSamplesCountPerSecond()) {
					return sl_null;
				}
			}
			
			sl_uint32 nOutput = getMaxSamplesPerFrame();
			SLIB_SCOPED_BUFFER(sl_int16, 4096, enc, nOutput);
			if (!enc) {
				return sl_null;
			}
			AudioData dataOutput;
			dataOutput.format = AudioFormat::Int16_Mono;
			dataOutput.data = enc;
			dataOutput.count = nOutput;
			nOutput = decoder->decode(input.data.getData(), (sl_uint32)(input.data.getSize()), dataOutput);
			if (!nOutput) {
				return sl_null;
			}
			Packet output;
			output.format = Packet::formatAudio_PCM_S16;
			output.audioParam.nChannels = decoder->getChannelsCount();
			output.audioParam.nSamplesPerSecond = decoder->getSamplesCountPerSecond();
			output.data = Memory::create(enc, nOutput * 2);
			return List<Packet>::createFromElement(output);
		}
		
	}
	
}


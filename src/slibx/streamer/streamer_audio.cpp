#include "../../../inc/slibx/streamer/audio.h"

namespace slib
{
	
	namespace streamer
	{
		
		class _AudioRecordSourceImpl : public AudioRecordSource
		{
		public:
			Ref<AudioRecorder> m_recorder;
			sl_uint32 m_nSamplesPerFrame;
			sl_uint32 m_nSamplesPerSecond;
			Ref<Event> m_event;
			
		private:
			_AudioRecordSourceImpl()
			{
			}
			
			~_AudioRecordSourceImpl()
			{
				m_recorder->release();
			}
	
		public:
			static Ref<_AudioRecordSourceImpl> create(const AudioRecordSourceParam& _param)
			{
				
				AudioRecordSourceParam param = _param;
				Ref<Event> ev = param.event;
				if (ev.isNull()) {
					ev = Event::create();
					param.event = ev;
				}
				
				Ref<AudioRecorder> recorder = AudioRecorder::create(param);
				if (recorder.isNotNull()) {
					Ref<_AudioRecordSourceImpl> ret = new _AudioRecordSourceImpl();
					if (ret.isNotNull()) {
						ret->m_recorder = recorder;
						ret->m_nSamplesPerFrame = param.nSamplesPerFrame;
						ret->m_nSamplesPerSecond = param.samplesPerSecond;
						ret->m_event = ev;
						return ret;
					}
					recorder->release();
				}
				return sl_null;
			}
			
			// override
			sl_bool receivePacket(Packet* out)
			{
				sl_uint32 n = m_nSamplesPerFrame;
				Memory mem = Memory::create(n * 2);
				if (mem.isNotNull()) {
					AudioData data;
					data.format = AudioFormat::Int16_Mono;
					data.data = mem.getData();
					data.count = n;
					if (m_recorder->read(data)) {
						float volume = getVolume();
						sl_int16* s = (sl_int16*)(mem.getData());
						for (sl_uint32 i = 0; i < n; i++) {
							s[i] = (sl_int16)(s[i] * volume);
						}
						out->data = mem;
						out->format = Packet::formatAudio_PCM_S16;
						out->audioParam.nChannels = 1;
						out->audioParam.nSamplesPerSecond = m_nSamplesPerSecond;
						return sl_true;
					}
				}
				return sl_false;
			}
			
			// override
			Ref<Event> getEvent()
			{
				return m_event;
			}
			
		};
		
		Ref<AudioRecordSource> AudioRecordSource::create(const AudioRecordSourceParam& param)
		{
			return Ref<AudioRecordSource>::from(_AudioRecordSourceImpl::create(param));
		}
		
		
		class _AudioPlaySinkImpl : public AudioPlaySink
		{
		public:
			Ref<AudioPlayerBuffer> m_playerBuffer;
			sl_uint32 m_nSamplesPerSecond;
			sl_uint32 m_nChannels;
			
		private:
			_AudioPlaySinkImpl()
			{
			}
			
			~_AudioPlaySinkImpl()
			{
				m_playerBuffer->release();
			}
			
		public:
			static Ref<_AudioPlaySinkImpl> create(const AudioPlaySinkParam& param)
			{
				Ref<AudioPlayer> player = param.player;
				if (player.isNotNull()) {
					Ref<AudioPlayerBuffer> playerBuffer = player->createBuffer(param);
					if (playerBuffer.isNotNull()) {
						Ref<_AudioPlaySinkImpl> ret = new _AudioPlaySinkImpl();
						if (ret.isNotNull()) {
							ret->m_playerBuffer = playerBuffer;
							ret->m_nSamplesPerSecond = param.samplesPerSecond;
							ret->m_nChannels = param.channelsCount;
							return ret;
						}
						playerBuffer->release();
					}
				}
				return sl_null;
			}
			
			sl_bool sendPacket(const Packet& input)
			{
				if (input.format != Packet::formatAudio_PCM_S16) {
					return sl_false;
				}
				if (input.audioParam.nSamplesPerSecond != m_nSamplesPerSecond) {
					return sl_false;
				}
				if (input.audioParam.nChannels != m_nChannels) {
					return sl_false;
				}
				sl_uint32 n = (sl_uint32)(input.data.getSize()) / 2;
				if (n > 0) {
					AudioData data;
					data.format = AudioFormat::Int16_Mono;
					data.data = input.data.getData();
					data.count = n;
					m_playerBuffer->write(data);
					return sl_true;
				}
				return sl_false;
			}
			
		};
		
		Ref<AudioPlaySink> AudioPlaySink::create(const AudioPlaySinkParam& param)
		{
			return Ref<AudioPlaySink>::from(_AudioPlaySinkImpl::create(param));
		}

	}
	
}

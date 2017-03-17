
#ifndef CHECKHEADER_SLIB_STREAMER_GRAPH
#define CHECKHEADER_SLIB_STREAMER_GRAPH

#include "definition.h"

#include <slib/core/object.h>
#include <slib/core/memory.h>
#include <slib/core/queue.h>
#include <slib/core/thread.h>

/***********************************

- Graph
Graph = Source -> Filter -> Filter -> ... -> Filter -> Sink
             Packet    Packet    Packet           Packet

Source should be attached for only one graph.

- Station
Station provides linked sink and sources.

                     -> Source1
				    /
	Sink -> Station  -> Source2
	                \
					 -> Source3


************************************/

namespace slib
{
	
	namespace streamer
	{

		struct Packet
		{
			enum Format {
				formatRaw = 0
				, formatAudio_PCM_S16 = 10
				, formatAudio_OPUS = 11
			};
			Format format;

			struct AudioParam
			{
				sl_uint32 nSamplesPerSecond;
				sl_uint32 nChannels;
			};
			AudioParam audioParam;
			struct NetworkParam
			{
				String addressFrom;
				String addressTo;
			};
			NetworkParam networkParam;

			Memory data;
		};

		class Source : public Object
		{
			SLIB_DECLARE_OBJECT
			
		public:
			Source();
			
			~Source();
			
		public:
			virtual Ref<Event> getEvent() = 0;
			
			virtual sl_bool receivePacket(Packet* out) = 0;

		};

		class Sink : public Object
		{
			SLIB_DECLARE_OBJECT
			
		public:
			Sink();
			
			~Sink();
			
		public:
			virtual sl_bool sendPacket(const Packet& packet) = 0;
			
		};

		class Filter : public Object
		{
			SLIB_DECLARE_OBJECT
			
		public:
			Filter();
			
			~Filter();
			
		public:
			virtual List<Packet> filter(const Packet& input) = 0;
			
		};

		class Graph : public Object
		{
			SLIB_DECLARE_OBJECT
			
		protected:
			Graph();
			
			~Graph();

		public:
			static Ref<Graph> create();

			sl_bool start();
			
			void release();

			virtual void feedPacket(const Packet& packet);

		protected:
			virtual void run();

		public:
			SLIB_INLINE Ref<Source> getSource()
			{
				return m_source;
			}
			SLIB_INLINE void setSource(const Ref<Source>& source)
			{
				m_source = source;
			}

			SLIB_INLINE Ref<Sink> getSink()
			{
				return m_sink;
			}
			SLIB_INLINE void setSink(const Ref<Sink>& sink)
			{
				m_sink = sink;
			}

			SLIB_INLINE List< Ref<Filter> > getFilters()
			{
				return m_filters;
			}
			SLIB_INLINE void addFilter(const Ref<Filter>& filter)
			{
				if (filter.isNotNull()) {
					m_filters.add(filter);
				}
			}

			SLIB_INLINE Ref<Thread> getThread()
			{
				return m_thread;
			}

		private:
			Ref<Source> m_source;
			Ref<Sink> m_sink;
			List< Ref<Filter> > m_filters;
			Ref<Thread> m_thread;
			
		};

	}
}

#endif

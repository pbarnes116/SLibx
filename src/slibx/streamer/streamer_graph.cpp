/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slibx/streamer/graph.h"

namespace slib
{
	
	namespace streamer
	{
		
		SLIB_DEFINE_OBJECT(Source, Object)
		
		Source::Source()
		{
		}
		
		Source::~Source()
		{
		}
		
		
		SLIB_DEFINE_OBJECT(Sink, Object)
		
		Sink::Sink()
		{
		}
		
		Sink::~Sink()
		{
		}
		
		
		SLIB_DEFINE_OBJECT(Filter, Object)
		
		Filter::Filter()
		{
		}
		
		Filter::~Filter()
		{
		}

		
		SLIB_DEFINE_OBJECT(Graph, Object)
		
		Graph::Graph()
		{
		}

		Graph::~Graph()
		{
			release();
		}

		sl_bool Graph::start()
		{
			if (m_source.isNull()) {
				return sl_false;
			}
			if (m_sink.isNull()) {
				return sl_false;
			}
			m_thread = Thread::start(SLIB_FUNCTION_REF(Graph, run, this));
			if (m_thread.isNotNull()) {
				return sl_true;
			}
			return sl_false;
		}

		void Graph::release()
		{
			m_source.setNull();
			m_sink.setNull();
			m_filters.removeAll();
			m_filters.setNull();
			Ref<Thread> thread = m_thread;
			if (thread.isNotNull()) {
				thread->finishAndWait();
			}
		}

		void Graph::run()
		{
			Ref<Source> source = m_source;
			if (source.isNull()) {
				return;
			}
			
			Ref<Event> ev = source->getEvent();
			if (ev.isNull()) {
				return;
			}

			while (! Thread::isStoppingCurrent()) {
				Packet packet;
				if (ev->wait()) {
					while (!Thread::isStoppingCurrent() && source->receivePacket(&packet)) {
						feedPacket(packet);
					}
				} else {
					Thread::sleep(50);
				}
			}
		}

		void Graph::feedPacket(const Packet& packet)
		{
			Ref<Sink> sink = m_sink;
			if (sink.isNull()) {
				return;
			}
			List<Packet> packets;
			{
				packets.add(packet);
				ListLocker< Ref<Filter> > filters(m_filters.duplicate());
				for (sl_size i = 0; i < filters.count; i++) {
					Ref<Filter> filter = filters[i];
					if (filter.isNotNull()) {
						List<Packet> outs;
						ListLocker<Packet> ins(packets);
						for (sl_size k = 0; k < ins.count; k++) {
							List<Packet> out = filter->filter(ins[k]);
							outs.addAll(out);
						}
						packets = outs;
					}
				}
			}
			{
				ListLocker<Packet> outs(packets);
				for (sl_size i = 0; i < outs.count; i++) {
					sink->sendPacket(outs[i]);
				}
			}
		}

		Ref<Graph> Graph::create()
		{
			Ref<Graph> ret = new Graph;
			return ret;
		}
		
	}
	
}

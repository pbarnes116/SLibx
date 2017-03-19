/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slibx/streamer/network.h"

#include <slib/core/loop_queue.h>

namespace slib
{
	
	namespace streamer
	{
		
		static Ref<Socket> _StreamerNetwork_createSocket(const SocketAddress addressBind, sl_bool flagBroadcast)
		{
			Ref<Socket> socket = Socket::openUdp();
			if (socket.isNull()) {
				return Ref<Socket>::null();
			}
			if (!socket->bind(addressBind)) {
				return Ref<Socket>::null();
			}
			socket->setNonBlockingMode(sl_true);
			if (flagBroadcast) {
				socket->setOption_Broadcast(sl_true);
			}
			return socket;
		}
		static Ref<Socket> _StreamerNetwork_createSocketForMulticast(const SocketAddress& addressBind, const IPv4Address& addressMulticast)
		{
			Ref<Socket> socket = Socket::openUdp();
			if (socket.isNull()) {
				return Ref<Socket>::null();
			}
			if (!socket->bind(addressBind)) {
				return Ref<Socket>::null();
			}
			if (!socket->setOption_IpAddMembership(addressMulticast, IPv4Address::Any)) {
				return Ref<Socket>::null();
			}
			socket->setNonBlockingMode(sl_true);
			return socket;
		}
		
		
		sl_bool NetworkUdpSink::sendPacket(const Packet& packet)
		{
			Ref<Socket> socket = getSocket();
			if (socket.isNull()) {
				return sl_false;
			}
			SocketAddress target;
			if (! resolveTarget(packet, target)) {
				return sl_false;
			}
			sl_int32 n = (sl_int32)(packet.data.getSize());
			sl_int32 nSend = socket->sendTo(target, packet.data.getData(), n);
			return n == nSend;
		}
		
		sl_bool NetworkUdpSink::resolveTarget(const Packet& packet, SocketAddress& target)
		{
			if (packet.networkParam.addressTo.isNotEmpty()) {
				SocketAddress address;
				if (address.parse(packet.networkParam.addressTo)) {
					target = address;
					return sl_true;
				}
			}
			SocketAddress address = getDefaultTarget();
			if (address.isValid()) {
				target = address;
				return sl_true;
			}
			return sl_false;
		}
		
		Ref<NetworkUdpSink> NetworkUdpSink::create(const Ref<Socket>& socket)
		{
			Ref<NetworkUdpSink> ret = new NetworkUdpSink();
			if (ret.isNotNull()) {
				ret->setSocket(socket);
			}
			return ret;
		}
		
		Ref<NetworkUdpSink> NetworkUdpSink::create(SocketAddress addressBind, SocketAddress defaultTarget, sl_bool flagBroadcast)
		{
			Ref<Socket> socket = _StreamerNetwork_createSocket(addressBind, flagBroadcast);
			if (socket.isNull()) {
				return Ref<NetworkUdpSink>::null();
			}
			Ref<NetworkUdpSink> ret = create(socket);
			if (ret.isNotNull()) {
				ret->setDefaultTarget(defaultTarget);
			}
			return ret;
		}
		
		Ref<NetworkUdpSink> NetworkUdpSink::createMulticast(SocketAddress addressBind, SocketAddress defaultTarget, const IPv4Address& group)
		{
			Ref<Socket> socket = _StreamerNetwork_createSocketForMulticast(addressBind, group);
			if (socket.isNull()) {
				return Ref<NetworkUdpSink>::null();
			}
			Ref<NetworkUdpSink> ret = create(socket);
			if (ret.isNotNull()) {
				ret->setDefaultTarget(defaultTarget);
			}
			return ret;
		}
		
		
		class _NetworkUdpSourceImpl : public NetworkUdpSource
		{
		public:
			Ref<Thread> m_thread;
			LoopQueue<Packet> m_queue;
			Ref<Event> m_event;
			
			_NetworkUdpSourceImpl()
			{
				m_queue.setQueueSize(1000);
				m_event = Event::create();
			}
			
			~_NetworkUdpSourceImpl()
			{
				if (m_thread.isNotNull()) {
					m_thread->finish();
				}
			}
			
			// override
			sl_bool receivePacket(Packet* out)
			{
				return m_queue.get(*out);
			}
			
			// override
			Ref<Event> getEvent()
			{
				return m_event;
			}
			
			void onPacket(char* buf, sl_int32 n, SocketAddress& address)
			{
				Packet packet;
				packet.format = Packet::formatRaw;
				packet.networkParam.addressFrom = address.toString();
				packet.data = Memory::create(buf, n);
				m_queue.add(packet, sl_true);
				m_event->set();
			}
			
#define SIZE_BUF 2000
			static void run(Ref<Socket> socket, WeakRef<_NetworkUdpSourceImpl> wr)
			{
				char buf[SIZE_BUF];
				while (!Thread::isStoppingCurrent()) {
					
					SocketAddress address;
					sl_int32 n = socket->receiveFrom(address, buf, SIZE_BUF);
					if (n > 0) {
						Ref<_NetworkUdpSourceImpl> object = wr;
						if (object.isNotNull()) {
							object->onPacket(buf, n, address);
						} else {
							return;
						}
					} else {
						Thread::sleep(5);
					}
				}
			}
		};
		
		Ref<NetworkUdpSource> NetworkUdpSource::create(const Ref<Socket>& _socket)
		{
			Ref<Socket> socket = _socket;
			Ref<_NetworkUdpSourceImpl> ret = new _NetworkUdpSourceImpl();
			if (ret.isNotNull()) {
				ret->setSocket(socket);
				WeakRef<_NetworkUdpSourceImpl> wr = ret;
				ret->m_thread = Thread::start(Function<void()>::bind(&_NetworkUdpSourceImpl::run, socket, wr));
			}
			return Ref<NetworkUdpSource>::from(ret);
		}
		
		Ref<NetworkUdpSource> NetworkUdpSource::create(SocketAddress addressBind, sl_bool flagBroadcast)
		{
			Ref<Socket> socket = _StreamerNetwork_createSocket(addressBind, flagBroadcast);
			if (socket.isNull()) {
				return Ref<NetworkUdpSource>::null();
			}
			return create(socket);
		}
		
		Ref<NetworkUdpSource> NetworkUdpSource::createMulticast(SocketAddress addressBind, const IPv4Address& group)
		{
			Ref<Socket> socket = _StreamerNetwork_createSocketForMulticast(addressBind, group);
			if (socket.isNull()) {
				return Ref<NetworkUdpSource>::null();
			}
			return create(socket);
		}
		
		
	}

}

/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slibx/snet/srouter.h"

#include <slib/network/event.h>
#include <slib/network/ethernet.h>
#include <slib/network/arp.h>
#include <slib/network/os.h>
#include <slib/crypto/zlib.h>
#include <slib/core/string_buffer.h>
#include <slib/core/scoped.h>
#include <slib/core/log.h>

#define TAG "SRouter"

#define MESSAGE_SIZE 102400
#define PACKET_SIZE 65536

namespace slib
{

	SRouterInterfaceParam::SRouterInterfaceParam()
	{
		fragment_expiring_seconds = 3600;
		mtu_outgoing = 0;
		
		use_nat = sl_false;
		nat_ip.setZero();
		nat_port_begin = 30000;
		nat_port_end = 60000;
	}

	void SRouterInterfaceParam::parseConfig(const Variant& varConfig)
	{
		fragment_expiring_seconds = varConfig.getItem("fragment_expiring_seconds").getUint32(fragment_expiring_seconds);
		mtu_outgoing = varConfig.getItem("mtu_outgoing").getUint32(mtu_outgoing);

		use_nat = varConfig.getItem("use_nat").getBoolean(use_nat);
		nat_ip.parse(varConfig.getItem("nat_ip").getString());
		nat_port_begin = (sl_uint16)(varConfig.getItem("nat_port_begin").getUint32(nat_port_begin));
		nat_port_end = (sl_uint16)(varConfig.getItem("nat_port_end").getUint32(nat_port_end));
	}

	SLIB_DEFINE_OBJECT(SRouterInterface, Object)

	SRouterInterface::SRouterInterface()
	{
		m_mtuOutgoing = 0;
		m_flagUseNat = sl_false;
	}

	Ref<SRouter> SRouterInterface::getRouter()
	{
		return m_router;
	}

	void SRouterInterface::setRouter(const Ref<SRouter>& router)
	{
		m_router = router;
	}

	void SRouterInterface::initWithParam(const SRouterInterfaceParam& param)
	{
		ObjectLocker lock(this);

		m_mtuOutgoing = param.mtu_outgoing;

		m_flagUseNat = param.use_nat;
		if (m_flagUseNat) {
			m_natParam.targetAddress = param.nat_ip;
			m_natParam.tcpPortBegin = param.nat_port_begin;
			m_natParam.tcpPortEnd = param.nat_port_end;
			m_natParam.udpPortBegin = param.nat_port_begin;
			m_natParam.udpPortEnd = param.nat_port_end;
			if (m_natParam.targetAddress.isZero()) {
				m_flagNatDynamicTarget = sl_true;
			} else {
				m_flagNatDynamicTarget = sl_false;
				m_nat.setup(m_natParam);
			}
		}
		m_fragmentation.setupExpiringDuration(param.fragment_expiring_seconds * 1000, param.dispatchLoop);
	}

	void SRouterInterface::setNatIp(const IPv4Address& ip)
	{
		ObjectLocker lock(this);
		if (m_natParam.targetAddress != ip) {
			m_natParam.targetAddress = ip;
			m_nat.setup(m_natParam);
		}
	}

	void SRouterInterface::writeIPv4Packet(const void* packet, sl_uint32 size)
	{
		char stack[PACKET_SIZE];
		Memory memNat;
		Memory memCombined;
		IPv4Packet* header = (IPv4Packet*)(packet);
		if (m_mtuOutgoing > 0) {
			if (IPv4Fragmentation::isNeededCombine(packet, size, sl_true)) {
				memCombined = m_fragmentation.combineFragment(packet, size, sl_true);
				header = (IPv4Packet*)(memCombined.getData());
				size = (sl_uint32)(memCombined.getSize());
			}
			if (!header) {
				return;
			}
		}
		if (header->getDestinationAddress().isHost()) {
			if (m_flagUseNat) {
				if (size < PACKET_SIZE) {
					Base::copyMemory(stack, packet, size);
					packet = stack;
				} else {
					memNat = Memory::create(packet, size);
					if (memNat.isEmpty()) {
						return;
					}
					packet = memNat.getData();
				}
				header = (IPv4Packet*)(packet);
				if (!(m_nat.translateOutgoingPacket(header, header->getContent(), header->getContentSize()))) {
					return;
				}
			}
		}
		if (m_mtuOutgoing > 0) {
			ListElements<Memory> packets(m_fragmentation.makeFragments(header, header->getContent(), header->getContentSize(), m_mtuOutgoing));
			for (sl_size i = 0; i < packets.count; i++) {
				_writeIPv4Packet(packets[i].getData(), (sl_uint32)(packets[i].getSize()));
			}
		} else {
			_writeIPv4Packet(header, header->getTotalSize());
		}
		
	}


	SRouterDeviceParam::SRouterDeviceParam()
	{
		use_pcap = sl_false;
		use_raw_socket = sl_false;
		is_ethernet = sl_true;
		subnet_broadcast.setZero();
		gateway_mac.setZero();
	}

	void SRouterDeviceParam::parseConfig(const Variant& varConfig)
	{
		SRouterInterfaceParam::parseConfig(varConfig);

	#if defined(SLIB_PLATFORM_IS_WINDOWS)
		iface_name = varConfig.getItem("iface_win").getString(iface_name);
	#else
		iface_name = varConfig.getItem("iface_unix").getString(iface_name);
	#endif
		use_pcap = varConfig.getItem("use_pcap").getBoolean(use_pcap);
		use_raw_socket = varConfig.getItem("use_raw_socket").getBoolean(use_raw_socket);
		is_ethernet = varConfig.getItem("is_ethernet").getBoolean(is_ethernet);
		subnet_broadcast.parse(varConfig.getItem("subnet_broadcast").getString());
		gateway_mac.parse(varConfig.getItem("gateway_mac").getString());

	}

	SLIB_DEFINE_OBJECT(SRouterDevice, SRouterInterface)

	SRouterDevice::SRouterDevice()
	{
		m_ipAddressDevice.setZero();
		m_macAddressDevice.setZero();
		m_subnetBroadcast.setZero();
		m_macAddressGateway.setZero();
	}

	SRouterDevice::~SRouterDevice()
	{
		release();
	}

	Ref<SRouterDevice> SRouterDevice::create(const SRouterDeviceParam& param)
	{
		Ref<SRouterDevice> ret = new SRouterDevice;
		if (ret.isNotNull()) {
			ret->m_device = param.device;
			ret->m_deviceName = param.iface_name;
			NetworkInterfaceInfo dev;
			if (Network::findInterface(param.iface_name, &dev)) {
				if (ret->m_device.isNull()) {
					NetCaptureParam ncp;
					ncp.deviceName = dev.name;
					ncp.listener = ret.get();
					ncp.sizeBuffer = 1024 * 1024 * 20;
					ncp.flagPromiscuous = sl_true;
					if (param.is_ethernet) {
						ncp.preferedLinkDeviceType = NetworkLinkDeviceType::Ethernet;
					} else {
						ncp.preferedLinkDeviceType = NetworkLinkDeviceType::Raw;
					}
					ncp.flagAutoStart = sl_false;

					if (param.use_pcap) {
						ret->m_device = NetCapture::createPcap(ncp);
					} else if (param.use_raw_socket) {
						ret->m_device = NetCapture::createRawIPv4(ncp);
					} else {
	#if defined(SLIB_PLATFORM_IS_LINUX)
						ret->m_device = NetCapture::createRawPacket(ncp);
	#else
						ret->m_device = NetCapture::createPcap(ncp);
	#endif
					}
					if (ret->m_device.isNull()) {
						LogError(TAG, "Can not capture on network device - %s(%s)", param.iface_name, dev.name);
						return Ref<SRouterDevice>::null();
					}
				}
				ret->m_macAddressDevice = dev.macAddress;
			} else {
				LogError(TAG, "Network device is not found - %s", param.iface_name);
				return Ref<SRouterDevice>::null();
			}
			ret->initWithParam(param);
			ret->m_subnetBroadcast = param.subnet_broadcast;
			ret->m_macAddressGateway = param.gateway_mac;
			return ret;
		}
		return Ref<SRouterDevice>::null();
	}

	void SRouterDevice::release()
	{
		if (m_device.isNotNull()) {
			m_device->release();
		}
	}

	void SRouterDevice::start()
	{
		if (m_device.isNotNull()) {
			m_device->start();
		}
	}

	void SRouterDevice::_writeIPv4Packet(const void* packet, sl_uint32 size)
	{
		Ref<NetCapture> dev = m_device;
		if (dev.isNull()) {
			return;
		}
		if (dev->getLinkType() == NetworkLinkDeviceType::Ethernet) {
			IPv4Packet* ip = (IPv4Packet*)(packet); 
			MacAddress macSource = m_macAddressDevice;
			if (macSource.isZero()) {
				return;
			}
			IPv4Address ipDst = ip->getDestinationAddress();
			MacAddress macTarget;
			if (m_subnetBroadcast.isNotZero() && ipDst == m_subnetBroadcast) {
				macTarget.setBroadcast();
			} else if (ipDst.isHost()) {
				macTarget = m_macAddressGateway;
				if (macTarget.isZero()) {
					m_tableMac.getMacAddress(ipDst, &macTarget);
					if (macTarget.isZero()) {
						return;
					}
				}
			} else if (ipDst.isBroadcast()) {
				macTarget.setBroadcast();
			} else if (ipDst.isMulticast()) {
				macTarget.makeMulticast(ipDst);
			} else {
				return;
			}
			SLIB_SCOPED_BUFFER(char, PACKET_SIZE, bufFrame, size + EthernetFrame::HeaderSize);
			EthernetFrame* frame = (EthernetFrame*)bufFrame;
			frame->setSourceAddress(macSource);
			frame->setDestinationAddress(macTarget);
			frame->setProtocol(NetworkLinkProtocol::IPv4);
			Base::copyMemory(bufFrame + EthernetFrame::HeaderSize, packet, size);
			dev->sendPacket(bufFrame, EthernetFrame::HeaderSize + size);
		} else {
			dev->sendPacket(packet, size);
		}
	}

	void SRouterDevice::processReadL2Frame(void* _frame, sl_uint32 lenFrame)
	{
		Ref<SRouter> router = getRouter();
		if (router.isNull()) {
			return;
		}
		Ref<NetCapture> dev = m_device;
		if (dev.isNull()) {
			return;
		}
		IPv4Packet* ip = 0;
		sl_uint32 lenIP = 0;
		if (dev->getLinkType() == NetworkLinkDeviceType::Ethernet) {
			EthernetFrame* frame = (EthernetFrame*)(_frame);
			if (m_macAddressGateway.isZero() && m_macAddressDevice != frame->getSourceAddress()) {
				m_tableMac.parseEthernetFrame(_frame, lenFrame, sl_true, sl_false);
			}
			if (lenFrame <= EthernetFrame::HeaderSize) {
				return;
			}

			if (router->forwardEthernetFrame(this, frame, lenFrame)) {
				return;
			}

			if (frame->getProtocol() != NetworkLinkProtocol::IPv4) {
				return;
			}
			MacAddress macDst = frame->getDestinationAddress();
			if (macDst == m_macAddressDevice || macDst.isBroadcast() || macDst.isMulticast()) {
				if (IPv4Packet::check(frame->getContent(), lenFrame - EthernetFrame::HeaderSize)) {
					ip = (IPv4Packet*)(frame->getContent());
					lenIP = ip->getTotalSize();
				}
			}
		} else {
			if (IPv4Packet::check(_frame, lenFrame)) {
				ip = (IPv4Packet*)(_frame);
				lenIP = ip->getTotalSize();
			}
		}
		if (ip && lenIP > 0) {
			router->forwardIPv4Packet(this, ip, lenIP, sl_true);
		}
	}

	void SRouterDevice::addMacAddress(const IPv4Address& ipAddress, const MacAddress& macAddress)
	{
		m_tableMac.add(ipAddress, macAddress);
	}

	void SRouterDevice::onCapturePacket(NetCapture* capture, NetCapturePacket* packet)
	{
		processReadL2Frame(packet->data, packet->length);
	}

	void SRouterDevice::writeL2Frame(const void* packet, sl_uint32 size)
	{
		Ref<NetCapture> dev = m_device;
		if (dev.isNull()) {
			return;
		}
		dev->sendPacket(packet, size);
	}

	String SRouterDevice::getStatus()
	{
		return m_deviceName + " " + m_ipAddressDevice.toString() + " " + m_macAddressDevice.toString();
	}

	void SRouterDevice::_idle()
	{
		NetworkInterfaceInfo dev;
		if (Network::findInterface(m_deviceName, &dev)) {
			m_macAddressDevice = dev.macAddress;
			if (dev.addresses_IPv4.isNotEmpty()) {
				IPv4AddressInfo addr;
				dev.addresses_IPv4.getAt(0, &addr);
				m_ipAddressDevice = addr.address;
				if (m_flagNatDynamicTarget) {
					setNatIp(m_ipAddressDevice);
				}
			}
		}
	}


	SRouterRemoteParam::SRouterRemoteParam()
	{
		flagTcp = sl_false;
		flagCompressPacket = sl_true;
		tcp_send_buffer_size = 1024000;
	}

	void SRouterRemoteParam::parseConfig(const Variant& varConfig)
	{
		SRouterInterfaceParam::parseConfig(varConfig);

		String protocol = varConfig.getItem("protocol").getString();
		if (protocol == "tcp") {
			flagTcp = sl_true;
		}
		host_address.setHostAddress(varConfig.getItem("host").getString());
		key = varConfig.getItem("key").getString();
		flagCompressPacket = varConfig.getItem("flag_compress_packet").getBoolean(flagCompressPacket);
		tcp_send_buffer_size = varConfig.getItem("tcp_send_buffer_size").getUint32(tcp_send_buffer_size);
	}

	SLIB_DEFINE_OBJECT(SRouterRemote, SRouterInterface)

	SRouterRemote::SRouterRemote()
	{
		m_flagTcp = sl_false;
		m_flagDynamicConnection = sl_true;
		m_timeLastKeepAliveSend.setZero();
		m_timeLastKeepAliveReceive.setZero();
	}

	Ref<SRouterRemote> SRouterRemote::create(const SRouterRemoteParam& param)
	{
		Ref<SRouterRemote> ret = new SRouterRemote();

		if (ret.isNotNull()) {

			ret->m_flagTcp = param.flagTcp;
			ret->m_address = param.host_address;
			ret->m_key = param.key;
			ret->m_flagCompressPacket = param.flagCompressPacket;

			ret->m_aes.setKey_SHA256(param.key);
			ret->m_flagDynamicConnection = ret->m_address.isInvalid();
			ret->m_tcpSendBufferSize = param.tcp_send_buffer_size;

			ret->initWithParam(param);

			return ret;
		}

		return Ref<SRouterRemote>::null();

	}

	String SRouterRemote::getStatus()
	{
		if (m_timeLastKeepAliveReceive.isNotZero()) {
			return String::format("Last Keep Alive - %ds", (Time::now() - m_timeLastKeepAliveReceive).getSecondsCount());
		} else {
			return "Not Connected";
		}
	}

	void SRouterRemote::reconnect()
	{
		if (m_flagTcp && !m_flagDynamicConnection) {
			TcpDatagramClientParam tp;
			tp.serverAddress = m_address;
			tp.listener = WeakRef<SRouter>(m_router);
			tp.flagAutoReconnect = sl_true;
			tp.autoReconnectIntervalSeconds = 5;
			tp.maxWaitingBytesForSending = m_tcpSendBufferSize;
			m_tcp = TcpDatagramClient::create(tp);
		}
	}

	void SRouterRemote::_idle()
	{
		Time now = Time::now();
		if ((now - m_timeLastKeepAliveSend).getMillisecondsCount() > 5000) {
			Ref<SRouter> router = getRouter();
			if (router.isNotNull()) {
				router->_sendRouterKeepAlive(this);
			}
			m_timeLastKeepAliveSend = now;
		}
	}

	void SRouterRemote::_writeIPv4Packet(const void* packet, sl_uint32 size)
	{
		Ref<SRouter> router = getRouter();
		if (router.isNotNull()) {
			if (m_flagCompressPacket) {
				router->_sendCompressedRawIPv4PacketToRemote(this, packet, size);
			} else {
				router->_sendRawIPv4PacketToRemote(this, packet, size);
			}
		}
	}


	SRouterRoute::SRouterRoute()
	{
		flagCheckProtocol = sl_false;
		flagTcp = sl_true;
		flagUdp = sl_true;
		flagIcmp = sl_true;

		flagCheckDstIp = sl_false;
		dst_ip_begin.setZero();
		dst_ip_end.setZero();

		flagCheckDstPort = sl_false;
		dst_port_begin = 0;
		dst_port_end = 0;

		flagCheckSrcIp = sl_false;
		src_ip_begin.setZero();
		src_ip_end.setZero();

		flagCheckDstIp = sl_false;
		src_port_begin = 0;
		src_port_end = 0;

		targets = sl_null;
		countTargets = 0;

		flagBreak = sl_true;
	}

	sl_bool SRouterRoute::parseConfig(SRouter* router, const Variant& conf)
	{
		Variant protocols = conf.getItem("protocols");
		if (protocols.isNotNull()) {
			flagCheckProtocol = sl_true;
			flagTcp = sl_false;
			flagUdp = sl_false;
			flagIcmp = sl_false;
			for (sl_size i = 0; i < protocols.getElementsCount(); i++) {
				String protocol = protocols.getElement(i).getString().toLower();
				if (protocol == "tcp") {
					flagTcp = sl_true;
				} else if (protocol == "udp") {
					flagUdp = sl_true;
				} else if (protocol == "icmp") {
					flagIcmp = sl_true;
				}
			}
		} else {
			flagCheckProtocol = sl_false;
		}

		String str;

		str = conf.getItem("dst_ip").getString();
		if (str.isNotEmpty()) {
			if (!(SocketAddress::parseIPv4Range(str, &dst_ip_begin, &dst_ip_end))) {
				return sl_false;
			}
			flagCheckDstIp = sl_true;
		} else {
			flagCheckDstIp = sl_false;
		}

		str = conf.getItem("dst_port").getString();
		if (str.isNotEmpty()) {
			if (!(SocketAddress::parsePortRange(str, &dst_port_begin, &dst_port_end))) {
				return sl_false;
			}
			flagCheckDstPort = sl_true;
		} else {
			flagCheckDstPort = sl_false;
		}

		str = conf.getItem("src_ip").getString();
		if (str.isNotEmpty()) {
			if (!(SocketAddress::parseIPv4Range(str, &src_ip_begin, &src_ip_end))) {
				return sl_false;
			}
			flagCheckSrcIp = sl_true;
		} else {
			flagCheckSrcIp = sl_false;
		}

		str = conf.getItem("src_port").getString();
		if (str.isNotEmpty()) {
			if (!(SocketAddress::parsePortRange(str, &src_port_begin, &src_port_end))) {
				return sl_false;
			}
			flagCheckSrcPort = sl_true;
		} else {
			flagCheckSrcPort = sl_false;
		}

		Variant varTargets = conf.getItem("targets");
		if (varTargets.isNotNull()) {
			sl_uint32 n = (sl_uint32)(varTargets.getElementsCount());
			arrTargets = Array< Ref<SRouterInterface> >::create(n);
			targets = arrTargets.getData();
			countTargets = (sl_uint32)(arrTargets.getCount());
			for (sl_uint32 i = 0; i < n; i++) {
				String target = varTargets.getElement(i).getString();
				targets[i] = router->getInterface(target);
				if (targets[i].isNull()) {
					LogError(TAG, "Failed to resolve route target: %s", target);
					return sl_false;
				}
			}
		}

		return sl_true;
	}


	SRouterArpProxy::SRouterArpProxy()
	{
		ip_begin.setZero();
		ip_end.setZero();
	}

	sl_bool SRouterArpProxy::parseConfig(SRouter* router, const Variant& conf)
	{
		String str = conf.getItem("ip").getString();
		if (str.isNotEmpty()) {
			if (SocketAddress::parseIPv4Range(str, &ip_begin, &ip_end)) {
				str = conf.getItem("device").getString();
				device = router->getDevice(str);
				if (device.isNotNull()) {
					return sl_true;
				} else {
					LogError(TAG, "Failed to resolve ARP proxy device: %s", str);
				}
			}
		}
		return sl_false;
	}


	SRouterParam::SRouterParam()
	{
		udp_server_port = 0;
		tcp_server_port = 0;
		tcp_send_buffer_size = 1024000;
	}

	SRouter::SRouter()
	{
		m_flagInit = sl_false;
		m_flagRunning = sl_false;
	}

	SRouter::~SRouter()
	{
		release();
	}

	Ref<SRouter> SRouter::create(const SRouterParam& param)
	{
		Ref<DispatchLoop> dispatchLoop = DispatchLoop::create(sl_false);
		Ref<AsyncIoLoop> ioLoop = AsyncIoLoop::create(sl_false);

		if (dispatchLoop.isNotNull() && ioLoop.isNotNull()) {

			Ref<SRouter> ret = new SRouter;
			
			if (ret.isNotNull()) {

				ret->m_name = param.name;

				ret->m_dispatchLoop = dispatchLoop;
				ret->m_ioLoop = ioLoop;

				AsyncUdpSocketParam up;
				up.bindAddress.port = param.udp_server_port;
				up.listener.setPointer(ret.get());
				up.packetSize = MESSAGE_SIZE + 32;
				up.ioLoop = ioLoop;
				up.flagAutoStart = sl_false;
				Ref<AsyncUdpSocket> udpServer = AsyncUdpSocket::create(up);
				if (udpServer.isNotNull()) {
					ret->m_udpServer = udpServer;
				} else {
					LogError(TAG, "Failed to create UDP server on port %d", param.udp_server_port);
					return Ref<SRouter>::null();
				}
				if (param.tcp_server_port > 0) {
					TcpDatagramServerParam tp;
					tp.bindAddress.port = param.tcp_server_port;
					tp.ioLoop = ioLoop;
					tp.listener = ret.get();
					tp.maxWaitingBytesForSending = param.tcp_send_buffer_size;
					tp.flagAutoStart = sl_false;
					Ref<TcpDatagramServer> tcpServer = TcpDatagramServer::create(tp);
					if (tcpServer.isNotNull()) {
						ret->m_tcpServer = tcpServer;
					} else {
						LogError(TAG, "Failed to create TCP server on port %d", param.tcp_server_port);
						return Ref<SRouter>::null();
					}
				}
				ret->m_aesPacket.setKey_SHA256(param.server_key);

				ret->m_timerIdle = Timer::createWithLoop(dispatchLoop, SLIB_FUNCTION_CLASS(SRouter, _onIdle, ret.get()), 1000);

				ret->m_flagInit = sl_true;

				return ret;
			}
		}
		return Ref<SRouter>::null();
	}

	Ref<SRouter> SRouter::createFromConfiguration(const Variant& varConfig)
	{
		SRouterParam param;
		param.name = varConfig.getItem("name").getString();

		sl_uint32 fragment_expiring_seconds = varConfig.getItem("fragment_expiring_seconds").getUint32(3600);

		param.udp_server_port = varConfig.getItem("udp_server_port").getUint32(param.udp_server_port);
		param.tcp_server_port = varConfig.getItem("tcp_server_port").getUint32(param.tcp_server_port);
		param.server_key = varConfig.getItem("server_key").getString();

		param.tcp_send_buffer_size = varConfig.getItem("tcp_send_buffer_size").getUint32(param.tcp_send_buffer_size);

		Ref<SRouter> ret = SRouter::create(param);

		if (ret.isNotNull()) {
			// add devices
			{
				for (auto item : varConfig["devices"].getVariantMap()) {
					SRouterDeviceParam dp;
					dp.fragment_expiring_seconds = fragment_expiring_seconds;
					dp.dispatchLoop = ret->m_dispatchLoop;
					dp.parseConfig(item.value);
					Ref<SRouterDevice> device = SRouterDevice::create(dp);
					if (device.isNotNull()) {
						ret->registerDevice(item.key, device);
					}
				}
			}
			// add remotes
			{
				for (auto item : varConfig["remotes"].getVariantMap()) {
					SRouterRemoteParam rp;
					rp.fragment_expiring_seconds = fragment_expiring_seconds;
					rp.dispatchLoop = ret->m_dispatchLoop;
					rp.tcp_send_buffer_size = param.tcp_send_buffer_size;
					rp.parseConfig(item.value);
					Ref<SRouterRemote> remote = SRouterRemote::create(rp);
					if (remote.isNotNull()) {
						ret->registerRemote(item.key, remote);
					}
				}
			}
			// add routes
			{
				for (auto item : varConfig["routes"].getVariantList()) {
					if (item.isNotNull()) {
						SRouterRoute route;
						if (route.parseConfig(ret.get(), item)) {
							ret->m_listRoutes.add_NoLock(route);
						} else {
							LogError(TAG, "Failed to parse route element: %s", item.toJsonString());
							return Ref<SRouter>::null();
						}
					}
				}
			}
			// add arp proxies
			{
				for (auto item : varConfig["arp_proxies"].getVariantList()) {
					if (item.isNotNull()) {					
						SRouterArpProxy arp;
						if (arp.parseConfig(ret.get(), item)) {
							ret->m_listArpProxies.add_NoLock(arp);
						} else {
							LogError(TAG, "Failed to parse ARP proxy element: %s", item.toJsonString());
							return Ref<SRouter>::null();
						}
					}
				}
			}

			ret->start();

			return ret;
		}
		return Ref<SRouter>::null();
	}

	void SRouter::release()
	{
		MutexLocker lock(getLocker());
		if (!m_flagInit) {
			return;
		}
		m_flagInit = sl_false;

		m_flagRunning = sl_false;
		if (m_dispatchLoop.isNotNull()) {
			m_dispatchLoop->release();
		}
		if (m_ioLoop.isNotNull()) {
			m_ioLoop->release();
		}
		if (m_udpServer.isNotNull()) {
			m_udpServer->close();
		}
		if (m_tcpServer.isNotNull()) {
			m_tcpServer->close();
		}
		{
			ListElements< Ref<SRouterDevice> > devices(m_mapDevices.getAllValues());
			for (sl_size i = 0; i < devices.count; i++) {
				if (devices[i].isNotNull()) {
					devices[i]->release();
				}
			}
		}

	}

	void SRouter::start()
	{
		MutexLocker lock(getLocker());
		if (!m_flagInit) {
			return;
		}
		if (m_flagRunning) {
			return;
		}

		if (m_udpServer.isNotNull()) {
			m_udpServer->start();
		}
		if (m_tcpServer.isNotNull()) {
			m_tcpServer->start();
		}
		m_dispatchLoop->start();
		m_ioLoop->start();

		{
			ListElements< Ref<SRouterDevice> > devices(m_mapDevices.getAllValues());
			for (sl_size i = 0; i < devices.count; i++) {
				if (devices[i].isNotNull()) {
					devices[i]->start();
				}
			}
		}

		m_flagRunning = sl_true;
	}

	Ref<SRouterInterface> SRouter::getInterface(const String& name)
	{
		return m_mapInterfaces.getValue(name, Ref<SRouterInterface>::null());
	}

	void SRouter::registerInterface(const String& name, const Ref<SRouterInterface>& _iface)
	{
		Ref<SRouterInterface> iface = _iface;
		if (iface.isNotNull()) {
			m_mapInterfaces.put(name, iface);
			iface->setRouter(this);
		}
	}

	Ref<SRouterDevice> SRouter::getDevice(const String& name)
	{
		return m_mapDevices.getValue(name, Ref<SRouterDevice>::null());
	}

	void SRouter::registerDevice(const String& name, const Ref<SRouterDevice>& device)
	{
		if (device.isNotNull()) {
			m_mapDevices.put(name, device);
			registerInterface(name, device);
		}
	}

	Ref<SRouterRemote> SRouter::getRemote(const String& name)
	{
		return m_mapRemotes.getValue(name, Ref<SRouterRemote>::null());
	}

	void SRouter::registerRemote(const String& name, const Ref<SRouterRemote>& remote)
	{
		if (remote.isNotNull()) {
			registerInterface(name, remote);
			m_mapRemotes.put(name, remote);
			if (!(remote->m_flagDynamicConnection)) {
				if (remote->m_flagTcp) {
					Ref<TcpDatagramClient> tcp = remote->m_tcp;
					if (tcp.isNull()) {
						remote->reconnect();
						tcp = remote->m_tcp;
					}
					if (tcp.isNotNull()) {
						m_mapRemotesByTcpClient.put(tcp.get(), remote);
					}
				} else {
					m_mapRemotesBySocketAddress.put(remote->m_address, remote);
				}
			}
		}
	}

	void SRouter::addRoute(const SRouterRoute& route)
	{
		m_listRoutes.add(route);
	}


	SLIB_INLINE static sl_bool _SRouter_checkMatchRouteProtocol(const SRouterRoute& route, const IPv4Packet* ip)
	{
		if (route.flagCheckProtocol) {
			switch (ip->getProtocol()) {
				case NetworkInternetProtocol::TCP:
					if (route.flagTcp) {
						return sl_true;
					}
					break;
				case NetworkInternetProtocol::UDP:
					if (route.flagUdp) {
						return sl_true;
					}
					break;
				case NetworkInternetProtocol::ICMP:
					if (route.flagIcmp) {
						return sl_true;
					}
					break;
				default:
					break;
			}
			return sl_false;
		}
		return sl_true;
	}

	SLIB_INLINE static sl_bool _SRouter_checkMatchRouteDstIp(const SRouterRoute& route, const IPv4Packet* ip)
	{
		if (route.flagCheckDstIp) {
			IPv4Address addrDst = ip->getDestinationAddress();
			if (addrDst < route.dst_ip_begin || addrDst > route.dst_ip_end) {
				return sl_false;
			}
		}
		return sl_true;
	}

	SLIB_INLINE static sl_bool _SRouter_checkMatchRouteSrcIp(const SRouterRoute& route, const IPv4Packet* ip)
	{
		if (route.flagCheckSrcIp) {
			IPv4Address addrSrc = ip->getSourceAddress();
			if (addrSrc < route.src_ip_begin || addrSrc > route.src_ip_end) {
				return sl_false;
			}
		}
		return sl_true;
	}

	SLIB_INLINE static sl_bool _SRouter_checkMatchRoutePorts(const SRouterRoute& route, const IPv4Packet* ip)
	{
		if (route.flagCheckDstPort || route.flagCheckSrcPort) {
			sl_uint16 portSrc;
			sl_uint16 portDst;
			if (ip->getPortsForTcpUdp(portSrc, portDst)) {
				if (route.flagCheckDstPort) {
					if (portDst < route.dst_port_begin || portDst > route.dst_port_end) {
						return sl_false;
					}
				}
				if (route.flagCheckSrcPort) {
					if (portSrc < route.src_port_begin || portSrc > route.src_port_end) {
						return sl_false;
					}
				}
			}
		}
		return sl_true;
	}

	void SRouter::forwardIPv4Packet(SRouterInterface* deviceSource, void* packet, sl_uint32 size, sl_bool flagCheckedHeader)
	{
		IPv4Packet* header = (IPv4Packet*)(packet);
		if (!flagCheckedHeader) {
			if (!(header->check(packet, size))) {
				return;
			}
		}

		if (!header) {
			return;
		}
		if (deviceSource->m_flagUseNat) {
			if (header->getDestinationAddress().isHost()) {
				if (!(deviceSource->m_nat.translateIncomingPacket(header, header->getContent(), header->getContentSize()))) {
					return;
				}
			} else {
				return;
			}
		}

		PtrLocker<SRouterListener> listener(m_listener);
		if (listener.isNotNull()) {
			if (listener->onForwardIPv4Packet(this, deviceSource, packet, size)) {
				return;
			}
		}

		if (IPv4Packet::check(packet, size)) {

			IPv4Packet* ip = (IPv4Packet*)packet;

			ListLocker<SRouterRoute> routes(m_listRoutes);

			for (sl_size i = 0; i < routes.count; i++) {
				
				const SRouterRoute& route = routes[i];
				
				if (_SRouter_checkMatchRouteProtocol(route, ip)) {
					if (_SRouter_checkMatchRouteDstIp(route, ip)) {
						if (_SRouter_checkMatchRouteSrcIp(route, ip)) {
							if (_SRouter_checkMatchRoutePorts(route, ip)) {

								for (sl_uint32 k = 0; k < route.countTargets; k++) {
									Ref<SRouterInterface> device = (route.arrTargets.getData())[k];
									if (device.isNotNull() && device != deviceSource) {
										device->writeIPv4Packet(packet, size);
									}
								}

								if (route.flagBreak) {
									return;
								}

							}
						}
					}
				}
			
			}
			
		}

	}

	sl_bool SRouter::forwardEthernetFrame(SRouterDevice* deviceSource, void* packet, sl_uint32 size)
	{
		PtrLocker<SRouterListener> listener(m_listener);
		if (listener.isNotNull()) {
			if (listener->onForwardEthernetFrame(this, deviceSource, packet, size)) {
				return sl_true;
			}
		}

		EthernetFrame* frame = (EthernetFrame*)(packet);
		if (size > EthernetFrame::HeaderSize) {
			if (frame->getProtocol() == NetworkLinkProtocol::ARP) {
				
				ArpPacket* arpIn = (ArpPacket*)(frame->getContent());
				sl_uint32 nArp = size - EthernetFrame::HeaderSize;
				
				if (nArp >= ArpPacket::SizeForIPv4 &&
					arpIn->isValidEthernetIPv4() &&
					arpIn->getOperation() == ArpOperation::Request)
				{
					sl_uint32 nSizeFrame = EthernetFrame::HeaderSize + ArpPacket::SizeForIPv4;
					IPv4Address addr = arpIn->getTargetIPv4Address();

					ListLocker<SRouterArpProxy> arpProxies(m_listArpProxies);

					for (sl_size i = 0; i < arpProxies.count; i++) {

						SRouterArpProxy& arpProxy = arpProxies[i];

						if (addr >= arpProxy.ip_begin && addr <= arpProxy.ip_end) {

							MacAddress deviceMacAddress = arpProxy.device->m_macAddressDevice;

							if (deviceMacAddress.isNotZero()) {
								char bufOut[1024];
								Base::copyMemory(bufOut, frame, nSizeFrame);

								EthernetFrame* frameOut = (EthernetFrame*)(bufOut);
								ArpPacket* arpOut = (ArpPacket*)(frameOut->getContent());
								frameOut->setDestinationAddress(frame->getSourceAddress());
								frameOut->setSourceAddress(deviceMacAddress);
								arpOut->setOperation(ArpOperation::Reply);
								arpOut->setTargetMacAddress(arpIn->getSenderMacAddress());
								arpOut->setTargetIPv4Address(arpIn->getSenderIPv4Address());
								arpOut->setSenderMacAddress(deviceMacAddress);
								arpOut->setSenderIPv4Address(addr);

								arpProxy.device->writeL2Frame(bufOut, nSizeFrame);
							}

							return sl_true;
						}

					}

				}
			}
		}

		return sl_false;
	}

	void SRouter::_sendRemoteMessage(SRouterRemote* remote, sl_uint8 method, const void* data, sl_uint32 n)
	{
		char buf[MESSAGE_SIZE];
		char bufEnc[MESSAGE_SIZE + 32];
		MemoryWriter writer(buf, MESSAGE_SIZE);
		if (!(writer.writeUint8(method))) {
			return;
		}
		if (!(writer.write(data, n))) {
			return;
		}
		sl_uint32 m = (sl_uint32)(writer.getPosition());
		m = (sl_uint32)(remote->m_aes.encrypt_CBC_PKCS7Padding(buf, m, bufEnc));
		if (m > 0) {
			Ref<TcpDatagramClient> tcp = remote->m_tcp;
			if (tcp.isNotNull()) {
				tcp->send(bufEnc, m);
			}
			if (remote->m_address.isValid()) {
				m_udpServer->sendTo(remote->m_address, bufEnc, m);
			}
		}
	}

	void SRouter::_receiveRemoteMessage(const SocketAddress& address, TcpDatagramClient* client, void* _data, sl_uint32 _size)
	{
		if (_size > MESSAGE_SIZE + 32) {
			return;
		}
		sl_uint8 data[MESSAGE_SIZE + 16];
		sl_uint32 size = (sl_uint32)(m_aesPacket.decrypt_CBC_PKCS7Padding(_data, _size, data));
		if (size == 0) {
			return;
		}

		Ref<SRouterRemote> remote;
		if (client) {
			m_mapRemotesByTcpClient.get(client, &remote);
		} else {
			m_mapRemotesBySocketAddress.get(address, &remote);
		}

		sl_uint8 method = data[0];
		switch (method) {
		case 10: // Compressed Raw IPv4 Packet
			if (remote.isNotNull()) {
				_receiveCompressedRawIPv4PacketFromRemote(remote.get(), data + 1, size - 1);
			}
			break;
		case 11: // Not-Compressed Raw IPv4 Packet
			if (remote.isNotNull()) {
				_receiveRawIPv4PacketFromRemote(remote.get(), data + 1, size - 1);
			}
			break;
		case 50: // Router Keep-Alive Notification
			_receiveRouterKeepAlive(address, client, data + 1, size - 1);
			break;
		}
	}


	void SRouter::_sendCompressedRawIPv4PacketToRemote(SRouterRemote* remote, const void* packet, sl_uint32 size)
	{
		Memory mem = Zlib::compressRaw(packet, size);
		if (mem.isNotEmpty()) {
			_sendRemoteMessage(remote, 10, mem.getData(), (sl_uint32)(mem.getSize()));
		}
	}

	void SRouter::_receiveCompressedRawIPv4PacketFromRemote(SRouterRemote* remote, void* data, sl_uint32 size)
	{
		Memory mem = Zlib::decompressRaw(data, size);
		if (mem.isNotEmpty()) {
			forwardIPv4Packet(remote, mem.getData(), (sl_uint32)(mem.getSize()), sl_false);
		}
	}


	void SRouter::_sendRawIPv4PacketToRemote(SRouterRemote* remote, const void* packet, sl_uint32 size)
	{
		_sendRemoteMessage(remote, 11, packet, size);
	}

	void SRouter::_receiveRawIPv4PacketFromRemote(SRouterRemote* remote, void* data, sl_uint32 size)
	{
		forwardIPv4Packet(remote, data, size, sl_false);
	}


	void SRouter::_sendRouterKeepAlive(SRouterRemote* remote)
	{
		char buf[512];
		MemoryWriter writer(buf, 512);
		if (!(writer.writeStringSection(m_name))) {
			return;
		}
		_sendRemoteMessage(remote, 50, buf, (sl_uint32)(writer.getPosition()));
	}

	void SRouter::_receiveRouterKeepAlive(const SocketAddress& address, TcpDatagramClient* client, void* data, sl_uint32 size)
	{
		MemoryReader reader(data, size);
		String name;
		if (!(reader.readStringSection(&name))) {
			return;
		}
		Ref<SRouterRemote> remote = m_mapRemotes.getValue(name, Ref<SRouterRemote>::null());
		if (remote.isNotNull()) {
			if (remote->m_flagDynamicConnection) {
				remote->m_address = address;
				remote->m_tcp = client;
			}
			if (client) {
				m_mapRemotesByTcpClient.put(client, remote);
			} else {
				m_mapRemotesBySocketAddress.put(address, remote);
			}
			remote->m_timeLastKeepAliveReceive = Time::now();
		}
	}

	void SRouter::onReceiveFrom(AsyncUdpSocket* socket, const SocketAddress& address, void* data, sl_uint32 sizeReceived)
	{
		_receiveRemoteMessage(address, sl_null, data, sizeReceived);
	}

	void SRouter::onReceiveFrom(TcpDatagramClient* client, void* data, sl_uint32 sizeReceived)
	{
		_receiveRemoteMessage(SocketAddress::none(), client, data, sizeReceived);
	}

	void SRouter::_onIdle(Timer* timer)
	{
		{
			MutexLocker lock(m_mapDevices.getLocker());
			for (auto item : m_mapDevices) {
				if (item.value.isNotNull()) {
					item.value->_idle();
				}
			}
		}
		{
			MutexLocker lock(m_mapRemotes.getLocker());
			for (auto item : m_mapRemotes) {
				if (item.value.isNotNull()) {
					item.value->_idle();
				}
			}
		}
	}

	String SRouter::getStatusReport()
	{
		StringBuffer ret;
		ret.add("Devices:\r\n");
		{
			MutexLocker lock(m_mapDevices.getLocker());
			for (auto item : m_mapDevices) {
				if (item.value.isNotNull()) {
					ret.add(item.key);
					ret.add(": ");
					ret.add(item.value->getStatus());
					ret.add("\r\n");
				}
			}
		}
		ret.add("\r\n");
		ret.add("Remotes:\r\n");
		{
			MutexLocker lock(m_mapRemotes.getLocker());
			for (auto item : m_mapRemotes) {
				if (item.value.isNotNull()) {
					ret.add(item.key);
					ret.add(": ");
					ret.add(item.value->getStatus());
					ret.add("\r\n");
				}
			}
		}
		ret.add("\r\n");
		return ret.merge();
	}

}

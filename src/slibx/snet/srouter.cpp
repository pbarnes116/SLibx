#include "../../../inc/slibx/snet/srouter.h"

#include <slib/network/event.h>
#include <slib/network/ethernet.h>
#include <slib/crypto/zlib.h>
#include <slib/core/scoped_pointer.h>
#include <slib/core/log.h>

#define TAG "SRouter"

#define MESSAGE_SIZE 102400
#define PACKET_SIZE 65536

SLIB_SNET_NAMESPACE_BEGIN
void SRouterInterfaceParam::parseConfig(const Variant& varConfig)
{
	fragment_expiring_seconds = varConfig.getField("fragment_expiring_seconds").getUint32(fragment_expiring_seconds);
	mtu_outgoing = varConfig.getField("mtu_outgoing").getUint32(mtu_outgoing);

	use_nat = varConfig.getField("use_nat").getBoolean(use_nat);
	nat_ip.parse(varConfig.getField("nat_ip").getString());
	nat_port_begin = varConfig.getField("nat_port_begin").getUint32(nat_port_begin);
	nat_port_end = varConfig.getField("nat_port_end").getUint32(nat_port_end);
}

SRouterInterface::SRouterInterface()
{
	m_mtuOutgoing = 1450;
	m_flagUseNat = sl_false;
}

SRouterInterface::~SRouterInterface()
{
}

Ref<SRouter> SRouterInterface::getRouter()
{
	return m_router.lock();
}

void SRouterInterface::setRouter(const Ref<SRouter>& router)
{
	m_router = router;
}

void SRouterInterface::initWithParam(const SRouterInterfaceParam& param)
{
	m_mtuOutgoing = param.mtu_outgoing;

	m_flagUseNat = param.use_nat;
	m_tableNat.setTargetAddress(param.nat_ip);
	m_tableNat.setTargetPortBegin(param.nat_port_begin);
	m_tableNat.setTargetPortEnd(param.nat_port_end);

	m_fragmentation.setupExpiringDuration(param.fragment_expiring_seconds * 1000);

	setListener(param.listener);
}

void SRouterInterface::setNAT_IP(const IPv4Address& ip)
{
	m_tableNat.setTargetAddress(ip);
}

void SRouterInterface::writePacket(const void* packet, sl_uint32 size)
{
	SLIB_SCOPED_BUFFER(char, PACKET_SIZE, buf, size);
	Base::copyMemory(buf, packet, size);
	
	PtrLocker<ISRouterInterfaceListener> listener(getListener());
	if (listener.isNotNull()) {
		if (!(listener->onWritePacket(this, buf, size))) {
			return;
		}
	}
	packet = buf;

	IPv4HeaderFormat* header = (IPv4HeaderFormat*)(packet);
	if (header->getDestinationAddress().isHost()) {
		if (m_flagUseNat) {
			if (!(m_tableNat.translateOutgoingPacket(header, header->getContent(), header->getContentSize()))) {
				return;
			}
		}
	}
	ListLocker<Memory> packets(m_fragmentation.makeFragments(header, header->getContent(), header->getContentSize(), m_mtuOutgoing));
	for (sl_size i = 0; i < packets.count(); i++) {
		_writePacket(packets[i].getBuf(), (sl_uint32)(packets[i].getSize()));
	}
}

void SRouterInterface::forwardPacket(const void* ip, sl_uint32 size)
{
	Memory mem = m_fragmentation.combineFragment(ip, size);
	if (mem.isNotEmpty()) {
		IPv4HeaderFormat* header = (IPv4HeaderFormat*)(mem.getBuf());
		if (m_flagUseNat) {
			if (header->getDestinationAddress().isHost()) {
				if (!(m_tableNat.translateIncomingPacket(header, header->getContent(), header->getContentSize()))) {
					return;
				}
			} else {
				return;
			}
		}
		Ref<SRouter> router = getRouter();
		if (router.isNotNull()) {
			router->forwardPacket(mem.getBuf(), (sl_uint32)(mem.getSize()));
		}
	}
}

void SRouterDeviceParam::parseConfig(const Variant& varConfig)
{
	SRouterInterfaceParam::parseConfig(varConfig);

#if defined(SLIB_PLATFORM_IS_WINDOWS)
	iface_name = varConfig.getField("iface_win").getString(iface_name);
#else
	iface_name = varConfig.getField("iface_unix").getString(iface_name);
#endif
	use_raw_socket = varConfig.getField("use_raw_socket").getBoolean(use_raw_socket);
	is_ethernet = varConfig.getField("is_ethernet").getBoolean(is_ethernet);
	gateway_mac.parse(varConfig.getField("gateway_mac").getString());

}

SRouterDevice::SRouterDevice()
{
	m_ipAddressDevice.setZero();
	m_macAddressDevice.setZero();
	m_macAddressGateway.setZero();
}

SRouterDevice::~SRouterDevice()
{
}

Ref<SRouterDevice> SRouterDevice::create(const SRouterDeviceParam& param)
{
	Ref<SRouterDevice> ret = new SRouterDevice;
	if (ret.isNotNull()) {
		ret->m_device = param.device;
		ret->m_deviceName = param.iface_name;
		NetworkDevice dev;
		if (dev.findDevice(param.iface_name)) {
			if (ret->m_device.isNull()) {
				NetCaptureParam ncp;
				ncp.deviceName = dev.name;
				ncp.listener = WeakRef<SRouterDevice>(ret);
				ncp.sizeBuffer = 1024 * 1024 * 20;
				ncp.flagPromiscuous = sl_true;
				if (param.is_ethernet) {
					ncp.preferedLinkDeviceType = networkLinkDevice_Ethernet;
				} else {
					ncp.preferedLinkDeviceType = networkLinkDevice_Raw;
				}
#if defined(SLIB_PLATFORM_IS_LINUX)
				if (param.use_raw_socket) {
					ret->m_device = NetCapture::createRawIPv4(ncp);
				} else {
					ret->m_device = NetCapture::createRawPacket(ncp);
				}
#else
				ret->m_device = NetCapture::createPcap(ncp);
#endif
				if (ret->m_device.isNull()) {
					SLIB_LOG_ERROR(TAG, "Can not capture on network device - " + param.iface_name + "(" + dev.name + ")");
					return Ref<SRouterDevice>::null();
				}
			}
			ret->m_macAddressDevice = dev.macAddress;
		} else {
			SLIB_LOG_ERROR(TAG, "Network device is not found - " + param.iface_name);
			return Ref<SRouterDevice>::null();
		}
		ret->initWithParam(param);
		ret->m_macAddressGateway = param.gateway_mac;
		return ret;
	}
	return Ref<SRouterDevice>::null();
}

void SRouterDevice::_writePacket(const void* packet, sl_uint32 size)
{
	Ref<NetCapture> dev = m_device;
	if (dev.isNull()) {
		return;
	}
	if (dev->getLinkType() == networkLinkDevice_Ethernet) {
		IPv4HeaderFormat* ip = (IPv4HeaderFormat*)(packet); 
		MacAddress macSource = m_macAddressDevice;
		if (macSource.isZero()) {
			return;
		}
		MacAddress macTarget = m_macAddressGateway;
		if (macTarget.isZero()) {
			m_tableMac.getMacAddress(ip->getDestinationAddress(), &macTarget);
			if (macTarget.isZero()) {
				return;
			}
		}
		SLIB_SCOPED_BUFFER(char, PACKET_SIZE, bufFrame, size + EthernetFrameFormat::getHeaderSize());
		EthernetFrameFormat* frame = (EthernetFrameFormat*)bufFrame;
		frame->setSourceAddress(macSource);
		frame->setDestinationAddress(macTarget);
		frame->setProtocol(networkLinkProtocol_IPv4);
		Base::copyMemory(bufFrame + EthernetFrameFormat::getHeaderSize(), packet, size);
		dev->sendPacket(bufFrame, EthernetFrameFormat::getHeaderSize() + size);
	} else {
		dev->sendPacket(packet, size);
	}
}

void SRouterDevice::processReadFrame(const void* _frame, sl_uint32 lenFrame)
{
	Ref<NetCapture> dev = m_device;
	if (dev.isNull()) {
		return;
	}
	IPv4HeaderFormat* ip = 0;
	sl_uint32 lenIP = 0;
	if (dev->getLinkType() == networkLinkDevice_Ethernet) {
		EthernetFrameFormat* frame = (EthernetFrameFormat*)(_frame);
		if (m_macAddressGateway.isZero() && m_macAddressDevice != frame->getSourceAddress()) {
			m_tableMac.parseEthernetFrame(_frame, lenFrame, sl_true, sl_false);
		}
		if (lenFrame <= EthernetFrameFormat::getHeaderSize()) {
			return;
		}
		if (frame->getProtocol() != networkLinkProtocol_IPv4) {
			return;
		}
		if (frame->getDestinationAddress() == m_macAddressDevice) {
			if (IPv4HeaderFormat::check(frame->getContent(), lenFrame - EthernetFrameFormat::getHeaderSize())) {
				ip = (IPv4HeaderFormat*)(frame->getContent());
				lenIP = ip->getTotalSize();
			}
		}
	} else {
		if (IPv4HeaderFormat::check(_frame, lenFrame)) {
			ip = (IPv4HeaderFormat*)(_frame);
			lenIP = ip->getTotalSize();
		}
	}
	if (ip && lenIP > 0) {
		forwardPacket(ip, lenIP);
	}
}

void SRouterDevice::addMacAddress(const IPv4Address& ipAddress, const MacAddress& macAddress)
{
	m_tableMac.add(ipAddress, macAddress);
}

void SRouterDevice::onCapturePacket(NetCapture* capture, NetCapturePacket* packet)
{
	processReadFrame(packet->data, packet->length);
}

String SRouterDevice::getStatus()
{
	return m_deviceName + " " + m_ipAddressDevice.toString() + " " + m_macAddressDevice.toString();
}

void SRouterDevice::_idle()
{
	NetworkDevice dev;
	if (dev.findDevice(m_deviceName)) {
		m_macAddressDevice = dev.macAddress;
		if (dev.addresses_IPv4.isNotEmpty()) {
			IPv4AddressInfo addr;
			dev.addresses_IPv4.getItem(0, &addr);
			m_ipAddressDevice = addr.address;
			m_tableNat.setTargetAddress(addr.address);
		}
	}
}

void SRouterRemoteParam::parseConfig(const Variant& varConfig)
{
	SRouterInterfaceParam::parseConfig(varConfig);

	host_address.setHostAddress(varConfig.getField("host").getString());
	key = varConfig.getField("key").getString();
}

SRouterRemote::SRouterRemote()
{
	m_flagDynamicAddress = sl_true;
	m_timeLastKeepAliveSend.setZero();
	m_timeLastKeepAliveReceive.setZero();
}

SRouterRemote::~SRouterRemote()
{
}

Ref<SRouterRemote> SRouterRemote::create(const SRouterRemoteParam& param)
{
	Ref<SRouterRemote> ret = new SRouterRemote();
	if (ret.isNotNull()) {
		ret->m_address = param.host_address;
		ret->m_key = param.key;
		ret->m_aes.setKey_SHA256(param.key);
		ret->m_flagDynamicAddress = ret->m_address.isInvalid();
		ret->initWithParam(param);
	}
	return ret;
}

String SRouterRemote::getStatus()
{
	if (m_timeLastKeepAliveReceive.isNotZero()) {
		return String("Last Keep Alive - ") + (Time::now() - m_timeLastKeepAliveReceive).getSecondsCount() + "s";
	} else {
		return "Not Connected";
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

void SRouterRemote::_writePacket(const void* packet, sl_uint32 size)
{
	if (m_address.isValid()) {
		Ref<SRouter> router = getRouter();
		if (router.isNotNull()) {
			router->_sendRawPacketToRemote(this, packet, size);
		}
	}
}

SRouterRoute::SRouterRoute()
{
	dst_ip_begin.setZero();
	dst_ip_end.setZero();
	src_ip_begin.setZero();
	src_ip_end.setZero();

	flagBreak = sl_true;
}

SRouterRoute::SRouterRoute(const SRouterRoute& other)
{
	dst_ip_begin = other.dst_ip_begin;
	dst_ip_end = other.dst_ip_end;
	src_ip_begin = other.src_ip_begin;
	src_ip_end = other.src_ip_end;

	interfaceName = other.interfaceName;
	interfaceDevice = other.interfaceDevice;

	flagBreak = other.flagBreak;
}

SRouterRoute& SRouterRoute::operator= (const SRouterRoute& other)
{
	dst_ip_begin = other.dst_ip_begin;
	dst_ip_end = other.dst_ip_end;
	src_ip_begin = other.src_ip_begin;
	src_ip_end = other.src_ip_end;

	interfaceName = other.interfaceName;
	interfaceDevice = other.interfaceDevice;

	flagBreak = other.flagBreak;
	return *this;
}

sl_bool SRouterRoute::operator==(const SRouterRoute& other) const
{
	return dst_ip_begin == other.dst_ip_begin
		&& dst_ip_end == other.dst_ip_end
		&& src_ip_begin == other.src_ip_begin
		&& src_ip_end == other.src_ip_end;
}

SRouter::SRouter()
{
	m_flagClosed = sl_false;
}

SRouter::~SRouter()
{
	release();
}

void SRouter::release()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	m_flagClosed = sl_true;
	Ref<Thread> thread = m_threadUdp;
	if (thread.isNotNull()) {
		thread->finishAndWait();
	}
	m_threadUdp.setNull();
	Ref<Socket> udp = m_udp;
	if (udp.isNotNull()) {
		udp->close();
	}
	m_udp.setNull();
}

Ref<SRouter> SRouter::create(const SRouterParam& param)
{
	Ref<SRouter> ret;
	ret = new SRouter;
	if (ret.isNotNull()) {
		ret->m_name = param.name;
		ret->m_aes.setKey_SHA256(param.key);
		if (param.port != 0) {
			ret->m_udp = Socket::openUdp_IPv4();
			if (ret->m_udp.isNotNull()) {
				ret->m_udp->setNonBlockingMode(sl_true);
				ret->m_udp->setOption_ReuseAddress(sl_true);
				if (ret->m_udp->bind(SocketAddress(param.port))) {
					ret->m_threadUdp = Thread::start(SLIB_CALLBACK_CLASS(SRouter, _runUdp, ret.get()));
				} else {
					SLIB_LOG(TAG, String("Failed to bind UDP socket on port: ") + param.port);
				}
			} else {
				SLIB_LOG(TAG, "Failed to open UDP socket");
			}
		}
	}
	return ret;
}

Ref<SRouter> SRouter::createFromConfiguration(const Variant& varConfig)
{
	SRouterParam param;
	param.name = varConfig.getField("name").getString();
	param.key = varConfig.getField("key").getString();
	param.port = varConfig.getField("port").getUint32(param.port);

	sl_uint32 fragment_expiring_seconds = varConfig.getField("fragment_expiring_seconds").getUint32(param.port);

	Ref<SRouter> ret = SRouter::create(param);
	if (ret.isNotNull()) {
		// add devices
		{
			ListLocker< Pair<String, Variant> > varDevices(varConfig.getField("devices").getVariantMap().pairs());
			for (sl_size i = 0; i < varDevices.count(); i++) {
				SRouterDeviceParam dp;
				dp.fragment_expiring_seconds = fragment_expiring_seconds;
				dp.parseConfig(varDevices[i].value);
				Ref<SRouterDevice> device = SRouterDevice::create(dp);
				if (device.isNotNull()) {
					ret->registerDevice(varDevices[i].key, device);
				}
			}
		}
		// add remotes
		{
			ListLocker< Pair<String, Variant> > varRemotes(varConfig.getField("remotes").getVariantMap().pairs());
			for (sl_size i = 0; i < varRemotes.count(); i++) {
				SRouterRemoteParam rp;
				rp.fragment_expiring_seconds = fragment_expiring_seconds;
				rp.parseConfig(varRemotes[i].value);
				Ref<SRouterRemote> remote = SRouterRemote::create(rp);
				if (remote.isNotNull()) {
					ret->registerRemote(varRemotes[i].key, remote);
				}
			}
		}
		// add routes
		{
			ListLocker<Variant> varRoutes(varConfig.getField("routes").getVariantList());
			for (sl_size i = 0; i < varRoutes.count(); i++) {

				SRouterRoute route;

				route.dst_ip_begin.parse(varRoutes[i].getField("dst_ip_begin").getString());
				route.dst_ip_end.parse(varRoutes[i].getField("dst_ip_end").getString());
				route.src_ip_begin.parse(varRoutes[i].getField("src_ip_begin").getString());
				route.src_ip_end.parse(varRoutes[i].getField("src_ip_end").getString());

				route.interfaceName = varRoutes[i].getField("interface").getString();

				route.flagBreak = varRoutes[i].getField("break").getBoolean(route.flagBreak);

				ret->addRoute(route, sl_false);
			}
		}
	}
	return ret;
}

Ref<SRouterInterface> SRouter::getInterface(const String& name)
{
	return m_mapInterfaces.getValue(name, Ref<SRouterInterface>::null());
}

void SRouter::registerInterface(const String& name, const Ref<SRouterInterface>& _iface)
{
	Ref<SRouterInterface> iface = _iface;
	if (iface.isNotNull()) {
		ListLocker<SRouterRoute> routes(m_listRoutes);
		for (sl_size i = 0; i < routes.count(); i++) {
			if (routes[i].interfaceName == name) {
				routes[i].interfaceDevice = iface;
			}
		}
		iface->setRouter(this);
	}
	m_mapInterfaces.put(name, iface);	
}

Ref<SRouterDevice> SRouter::getDevice(const String& name)
{
	return m_mapDevices.getValue(name, Ref<SRouterDevice>::null());
}

void SRouter::registerDevice(const String& name, const Ref<SRouterDevice>& device)
{
	m_mapDevices.put(name, device);
	registerInterface(name, device);
}

Ref<SRouterRemote> SRouter::getRemote(const String& name)
{
	return m_mapRemotes.getValue(name, Ref<SRouterRemote>::null());
}

void SRouter::registerRemote(const String& name, const Ref<SRouterRemote>& remote)
{
	m_mapRemotes.put(name, remote);
	registerInterface(name, remote);
}

void SRouter::addRoute(const SRouterRoute& _route, sl_bool flagReplace)
{
	SRouterRoute route = _route;
	if (route.interfaceDevice.isNull()) {
		route.interfaceDevice = getInterface(route.interfaceName);
	}
	if (flagReplace) {
		m_listRoutes.addIfNotExist(route);
	} else {
		m_listRoutes.add(route);
	}
}

void SRouter::forwardPacket(const void* packet, sl_uint32 size)
{
	if (m_flagClosed) {
		return;
	}

	if (IPv4HeaderFormat::check(packet, size)) {

		IPv4HeaderFormat* ip = (IPv4HeaderFormat*)packet;

		ListLocker<SRouterRoute> routes(m_listRoutes);
		for (sl_size i = 0; i < routes.count(); i++) {
			const SRouterRoute& route = routes[i];
			sl_bool flagMatch = sl_true;
			IPv4Address addrDst = ip->getDestinationAddress();
			IPv4Address addrSrc = ip->getSourceAddress();
			if (route.dst_ip_begin.isNotZero() && route.dst_ip_end.isNotZero()) {
				if (addrDst < route.dst_ip_begin || addrDst > route.dst_ip_end) {
					flagMatch = sl_false;
				}
			}
			if (flagMatch) {
				if (route.src_ip_begin.isNotZero() && route.src_ip_end.isNotZero()) {
					if (addrSrc < route.src_ip_begin || addrSrc > route.src_ip_end) {
						flagMatch = sl_false;
					}
				}
			}
			if (flagMatch) {
				if (route.interfaceDevice.isNotNull()) {
					route.interfaceDevice->writePacket(packet, size);
				}
				if (route.flagBreak) {
					return;
				}
			}
		}
		
	}
}

void SRouter::_sendRemoteMessage(SRouterRemote* remote, sl_uint8 method, const void* data, sl_uint32 n)
{
	if (m_flagClosed) {
		return;
	}
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
		Ref<Socket> udp = m_udp;
		if (udp.isNotNull()) {
			udp->sendTo(bufEnc, m, remote->m_address);
		}
	}
}

void SRouter::_receiveRemoteMessage(SocketAddress& address, const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	sl_uint8 method = data[0];
	switch (method) {
	case 10: // Raw IP Packet
		_receiveRawPacketFromRemote(address, data + 1, size - 1);
		break;
	case 50: // Router Keep-Alive Notification
		_receiveRouterKeepAlive(address, data + 1, size - 1);
		break;
	}
}

void SRouter::_sendRawPacketToRemote(SRouterRemote* remote, const void* ip, sl_uint32 size)
{
	Memory mem = Zlib::compressRaw(ip, size);
	if (mem.isNotEmpty()) {
		_sendRemoteMessage(remote, 10, mem.getBuf(), (sl_uint32)(mem.getSize()));
	}
}

void SRouter::_receiveRawPacketFromRemote(SocketAddress& address, const void* data, sl_uint32 size)
{
	Memory mem = Zlib::decompressRaw(data, size);
	if (mem.isNotEmpty()) {
		forwardPacket(mem.getBuf(), (sl_uint32)(mem.getSize()));
	}
}

void SRouter::_sendRouterKeepAlive(SRouterRemote* remote)
{
	char buf[512];
	MemoryWriter writer(buf, 512);
	if (!(writer.writeString(m_name))) {
		return;
	}
	_sendRemoteMessage(remote, 50, buf, (sl_uint32)(writer.getPosition()));
}

void SRouter::_receiveRouterKeepAlive(SocketAddress& address, const void* data, sl_uint32 size)
{
	MemoryReader reader(data, size);
	String name;
	if (!(reader.readString(&name))) {
		return;
	}
	Ref<SRouterRemote> remote = m_mapRemotes.getValue(name, Ref<SRouterRemote>::null());
	if (remote.isNotNull()) {
		if (remote->m_flagDynamicAddress) {
			remote->m_address = address;
		}
		remote->m_timeLastKeepAliveReceive = Time::now();
	}
}

void SRouter::_runUdp()
{
	Ref<Socket> udp = m_udp;
	Ref<SocketEvent> ev = SocketEvent::createRead(udp);
	if (ev.isNull()) {
		SLIB_LOG_ERROR(TAG, "Can not create socket event");
		return;
	}
	TimeCounter tc;
	char buf[MESSAGE_SIZE + 32];
	char bufDec[MESSAGE_SIZE + 16];
	while (Thread::isNotStoppingCurrent()) {
		SocketAddress addr;
		while (1) {
			sl_int32 n = udp->receiveFrom(buf, MESSAGE_SIZE, addr);
			if (n > 0) {
				sl_uint32 m = (sl_uint32)(m_aes.decrypt_CBC_PKCS7Padding(buf, n, bufDec));
				if (m > 0) {
					_receiveRemoteMessage(addr, bufDec, m);
				}
			} else {
				break;
			}
		}
		ev->wait(1000);

		if (tc.getEllapsedMilliseconds() > 1000) {
			{
				ListLocker< Ref<SRouterDevice> > devices(m_mapDevices.values());
				for (sl_size i = 0; i < devices.count(); i++) {
					if (devices[i].isNotNull()) {
						devices[i]->_idle();
					}
				}
			}
			{
				ListLocker< Ref<SRouterRemote> > remotes(m_mapRemotes.values());
				for (sl_size i = 0; i < remotes.count(); i++) {
					if (remotes[i].isNotNull()) {
						remotes[i]->_idle();
					}
				}
			}
			tc.reset();
		}
	}
}

String SRouter::getStatusReport()
{
	StringBuffer ret;
	ret.add("Devices:\r\n");
	{
		ListLocker< Pair< String, Ref<SRouterDevice> > > devices(m_mapDevices.pairs());
		for (sl_size i = 0; i < devices.count(); i++) {
			if (devices[i].value.isNotNull()) {
				ret.add(devices[i].key);
				ret.add(": ");
				ret.add(devices[i].value->getStatus());
				ret.add("\r\n");
			}
		}
	}
	ret.add("\r\n");
	ret.add("Remotes:\r\n");
	{
		ListLocker< Pair< String, Ref<SRouterRemote> > > remotes(m_mapRemotes.pairs());
		for (sl_size i = 0; i < remotes.count(); i++) {
			if (remotes[i].value.isNotNull()) {
				ret.add(remotes[i].key);
				ret.add(": ");
				ret.add(remotes[i].value->getStatus());
				ret.add("\r\n");
			}
		}
	}
	ret.add("\r\n");
	return ret.merge();
}

SLIB_SNET_NAMESPACE_END

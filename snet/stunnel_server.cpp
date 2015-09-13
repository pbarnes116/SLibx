#include "../../../inc/slibx/snet/stunnel_server.h"

#include "../../../inc/slib/network/ethernet.h"
#include "../../../inc/slib/crypto/zlib.h"
#include "../../../inc/slib/core/log.h"

#define TAG "STunnelServer"

SLIB_SNET_NAMESPACE_BEGIN
STunnelServer::STunnelServer()
{
}

STunnelServer::~STunnelServer()
{
	release();
}

Ref<STunnelServer> STunnelServer::create(const STunnelServerParam& param)
{
	Ref<STunnelServer> ret = new STunnelServer;
	if (ret.isNotNull()) {
		if (!(ret->initialize(param))) {
			ret.setNull();
		}
	}
	return ret;
}

sl_bool STunnelServer::initialize(const STunnelServerParam& param)
{
	m_param = param;
	Ref<AsyncLoop> asyncLoop = param.asyncLoop;
	if (asyncLoop.isNull()) {
		asyncLoop = AsyncLoop::getDefault();
		if (asyncLoop.isNull()) {
			return sl_false;
		}
	}
	m_asyncLoop = asyncLoop;
	if (param.nat_enabled) {
		// initialize NAT device
		if (param.nat_device.isNotEmpty()) {
			NetworkDevice dev;
			if (dev.findDevice(param.nat_device)) {
				SLIB_LOG(TAG, "Starting NAT on device - " + param.nat_device + ":" + param.nat_port_begin + "~" + param.nat_port_end);
				NetCaptureParam ncp;
				ncp.deviceName = dev.name;
				ncp.listener = WeakRef<STunnelServer>(this);
				ncp.sizeBuffer = 1024 * 1024 * 20;
				MacAddress natGateway = param.nat_gateway;
				if (natGateway.isZero()) {
					ncp.preferedLinkDeviceType = networkLinkDevice_Raw;
				} else {
					ncp.preferedLinkDeviceType = networkLinkDevice_Ethernet;
				}
#if defined(SLIB_PLATFORM_IS_LINUX)
				if (param.nat_raw_socket) {
					m_captureNat = NetCapture::createRawIPv4(ncp);
				} else {
					m_captureNat = NetCapture::createRawPacket(ncp);
				}
#else
				m_captureNat = NetCapture::createPcap(ncp);
#endif
				if (m_captureNat.isNotNull()) {
					sl_uint32 linkType = m_captureNat->getLinkType();
					if (linkType == networkLinkDevice_Ethernet || linkType == networkLinkDevice_Raw) {
						m_threadNat = Thread::start(SLIB_CALLBACK_CLASS(STunnelServer, runNAT, this));
						m_tableNat.setTargetPortBegin(param.nat_port_begin);
						m_tableNat.setTargetPortEnd(param.nat_port_end);
						m_addressNatDev = dev.macAddress;
						m_addressNatGateway = natGateway;
						SLIB_LOG(TAG, "NAT started");
					} else {
						SLIB_LOG_ERROR(TAG, "NAT device is not ethernet and not linux cooked!");
						return sl_false;
					}
				} else {
					SLIB_LOG_ERROR(TAG, "NAT starting is failed");
					return sl_false;
				}
			} else {
				SLIB_LOG_ERROR(TAG, "NAT device is not found - " + param.nat_device);
				return sl_false;
			}
		} else {
			SLIB_LOG_ERROR(TAG, "NAT device is not set!");
			return sl_false;
		}
	}

	// initialize server
	{
		SLIB_LOG(TAG, String("Starting Server on ") + param.addressBind.toString());
		AsyncSecureStreamServerParam ssp;
		ssp.masterKey = param.masterKey;
		ssp.listener = WeakRef<STunnelServer>(this);
		m_server = AsyncTcpSecureStreamServer::create(ssp, param.addressBind, asyncLoop);
		if (m_server.isNotNull()) {
			SLIB_LOG(TAG, "Server started");
		} else {
			SLIB_LOG_ERROR(TAG, "Server failed");
			return sl_false;
		}
	}

	m_fragmentationIncoming.setupExpiringDuration(param.nat_expiring_duration, asyncLoop);
	m_fragmentationOutgoing.setupExpiringDuration(param.nat_expiring_duration, asyncLoop);

	return sl_true;
}

void STunnelServer::release()
{
	MutexLocker lock(getLocker());
	if (m_threadNat.isNotNull()) {
		m_threadNat->finishAndWait();
		m_threadNat.setNull();
	}
	if (m_captureNat.isNotNull()) {
		m_captureNat->release();
		m_captureNat.setNull();
	}
	if (m_server.isNotNull()) {
		m_server->close();
		m_server.setNull();
	}
	m_asyncLoop.setNull();
}

void STunnelServer::runNAT()
{
	while (Thread::isNotStoppingCurrent()) {
		NetworkDevice dev;
		if (dev.findDevice(m_param.nat_device)) {
			m_addressNatDev = dev.macAddress;
			if (dev.addresses_IPv4.isNotEmpty()) {
				IPv4AddressInfo addr;
				dev.addresses_IPv4.getItem(0, &addr);
				m_tableNat.setTargetAddress(addr.address);
			}
		}
		Thread::sleep(3000);
	}
}

void STunnelServer::onCapturePacket(NetCapture* capture, NetCapturePacket* packet)
{
	sl_uint32 linkType = capture->getLinkType();
	IPv4HeaderFormat* ip = 0;
	sl_uint32 lenIP = 0;
	sl_uint32 lenFrame = packet->length;
	if (linkType == networkLinkDevice_Ethernet) {
		EthernetFrameFormat* frame = (EthernetFrameFormat*)(packet->data);
		if (lenFrame <= EthernetFrameFormat::getHeaderSize()) {
			return;
		}
		if (frame->getProtocol() != networkLinkProtocol_IPv4) {
			return;
		}
		if (frame->getDestinationAddress() == m_addressNatDev) {
			if (IPv4HeaderFormat::check(frame->getContent(), lenFrame - EthernetFrameFormat::getHeaderSize())) {
				ip = (IPv4HeaderFormat*)(frame->getContent());
				lenIP = ip->getTotalSize();
			}
		}
	} else {
		if (IPv4HeaderFormat::check(packet->data, lenFrame)) {
			ip = (IPv4HeaderFormat*)(packet->data);
			lenIP = ip->getTotalSize();
		}
	}
	if (ip && lenIP > 0) {
		Memory mem = m_fragmentationIncoming.combineFragment(ip, lenIP);
		if (mem.isNotEmpty()) {
			Ptr<Referable> userObject;
			ip = (IPv4HeaderFormat*)(mem.getBuf());
			if (m_tableNat.translateIncomingPacket(ip, ip->getContent(), ip->getContentSize(), &userObject)) {
				PtrLocker<STunnelServiceSession> client(Ptr<STunnelServiceSession>::from(userObject));
				if (client.isNotNull()) {
					ListLocker<Memory> fragments(m_fragmentationIncoming.makeFragments(ip, ip->getContent(), ip->getContentSize(), m_param.nat_mtu_incoming));
					for (sl_size i = 0; i < fragments.count(); i++) {
						client->sendRawIP(fragments[i].getBuf(), (sl_uint32)(fragments[i].getSize()));
					}
				}
			}
		}
	}
}

void STunnelServer::writeIPToNAT(STunnelServiceSession* session, const void* ipPacket, sl_uint32 size)
{
	sl_uint32 mtu = m_param.nat_mtu_outgoing;
	if (mtu > 1500) {
		mtu = 1500;
	}
	Ref<NetCapture> dev = m_captureNat;
	if (dev.isNull()) {
		return;
	}
	char bufFrame[2000];
	sl_uint32 lenFrameHeader;
	if (dev->getLinkType() == networkLinkDevice_Ethernet) {
		MacAddress macGateway = m_addressNatGateway;
		MacAddress mac = m_addressNatDev;
		if (mac.isZero()) {
			return;
		}
		EthernetFrameFormat* frameOut = (EthernetFrameFormat*)bufFrame;
		frameOut->setSourceAddress(mac);
		frameOut->setDestinationAddress(macGateway);
		frameOut->setProtocol(networkLinkProtocol_IPv4);
		lenFrameHeader = EthernetFrameFormat::getHeaderSize();
	} else {
		lenFrameHeader = 0;
	}
	Memory mem = m_fragmentationOutgoing.combineFragment(ipPacket, size);
	if (mem.isNotEmpty()) {
		IPv4HeaderFormat* ip = (IPv4HeaderFormat*)(mem.getBuf());
		Ptr<Referable> userObject = WeakRef<STunnelServiceSession>(session);
		if (m_tableNat.translateOutgoingPacket(ip, ip->getContent(), ip->getContentSize(), &userObject)) {
			ListLocker<Memory> fragments(m_fragmentationOutgoing.makeFragments(ip, ip->getContent(), ip->getContentSize(), m_param.nat_mtu_incoming));
			for (sl_size i = 0; i < fragments.count(); i++) {
				sl_uint32 lenIP = (sl_uint32)(fragments[i].getSize());
				Base::copyMemory(bufFrame + lenFrameHeader, fragments[i].getBuf(), lenIP);
				dev->sendPacket(bufFrame, lenFrameHeader + lenIP);
			}
		}
	}
}

void STunnelServer::onConnectedSecureStream(AsyncSecureStream* securedStream, sl_bool flagError)
{
	Ref<STunnelServiceSession> session = new STunnelServiceSession;
	if (session.isNotNull()) {
		session->m_stream = securedStream;
		session->m_server = this;
		session->m_limitQueueDatagram = m_param.session_datagram_queue_limit;
		session->m_bufRead = Memory::create(102400);
		session->m_portsUDP.setupTimer(m_param.nat_expiring_duration, m_asyncLoop.lock());
		if (session->m_bufRead.isNotEmpty()) {
			if (session->m_queueOutputChannels.setChannelsCount(2)) {
				m_sessions.put(session.get(), session);
				session->read();
			}
		}
	}
}

STunnelServiceSession::STunnelServiceSession()
{
	m_flagReading = sl_false;
	m_flagWriting = sl_false;
	m_limitQueueDatagram = 5000;
}

void STunnelServiceSession::close()
{
	MutexLocker lock(getLocker());
	if (m_stream.isNotNull()) {
		m_stream->close();
		m_stream.setNull();
		Ref<STunnelServer> server = m_server.lock();
		if (server.isNotNull()) {
			server->m_sessions.remove(this);
		}
	}
}

Ref<NetCapture> STunnelServiceSession::getNatDevice()
{
	Ref<STunnelServer> server = m_server.lock();
	if (server.isNotNull()) {
		return server->m_captureNat;
	}
	return Ref<NetCapture>::null();
}

void STunnelServiceSession::read()
{
	MutexLocker lock(getLocker());
	if (m_flagReading) {
		return;
	}
	Ref<AsyncSecureStream> stream = m_stream;
	if (stream.isNull()) {
		return;
	}
	m_flagReading = sl_true;
	if (!(stream->readToMemory(m_bufRead, WeakRef<STunnelServiceSession>(this)))) {
		m_flagReading = sl_false;
		close();
	}
}

void STunnelServiceSession::onRead(AsyncStream* stream, void* data, sl_uint32 sizeRead, Referable* ref, sl_bool flagError)
{
	{
		PtrLocker<TCP_Port> port(Ptr<TCP_Port>::from(stream->getUserObject(0)));
		if (port.isNotNull()) {
			_onReadPort_TCP(port.get(), data, sizeRead, flagError);
			return;
		}
	}
	MutexLocker lock(getLocker());
	m_flagReading = sl_false;
	if (sizeRead > 0) {
		Queue<Memory> datagrams;
		if (m_datagram.parse(data, sizeRead, datagrams)) {
			Memory datagram;
			while (datagrams.pop(&datagram)) {
				if (datagram.isNotEmpty()) {
					receiveSessionMessage(datagram.getBuf(), (sl_uint32)(datagram.getSize()));
				}
			}
		} else {
			close();
		}
	}
	if (flagError) {
		close();
	} else {
		read();
	}
}

#define OUTPUT_UNIT 102400
void STunnelServiceSession::write()
{
	MutexLocker lock(getLocker());
	if (m_flagWriting) {
		return;
	}
	Ref<AsyncSecureStream> stream = m_stream;
	if (stream.isNull()) {
		return;
	}
	MemoryBuffer buffer;
	Memory output;
	while (m_queueOutputChannels.popFront(&output)) {
		if (output.isNotEmpty()) {
			buffer.add(output);
			if (buffer.getSize() > OUTPUT_UNIT) {
				break;
			}
		}
	}
	output = buffer.merge();
	if (output.isEmpty()) {
		return;
	}
	m_flagWriting = sl_true;
	if (!(stream->writeFromMemory(output, WeakRef<STunnelServiceSession>(this)))) {
		m_flagWriting = sl_false;
		close();
	}
}

void STunnelServiceSession::onWrite(AsyncStream* stream, void* data, sl_uint32 sizeWritten, Referable* ref, sl_bool flagError)
{
	{
		PtrLocker<TCP_Port> port(Ptr<TCP_Port>::from(stream->getUserObject(0)));
		if (port.isNotNull()) {
			_onWritePort_TCP(port.get(), data, sizeWritten, flagError);
			return;
		}
	}
	MutexLocker lock(getLocker());
	m_flagWriting = sl_false;
	if (flagError) {
		close();
	} else {
		write();
	}
}

void STunnelServiceSession::sendSessionMessage(sl_uint8 method, MemoryBuffer& buf, sl_bool flagDatagram)
{
	Ref<AsyncSecureStream> stream = m_stream;
	if (stream.isNull()) {
		return;
	}
	MemoryWriter writer;
	writer.writeUint8(method);
	writer.getMemoryBuffer().link(buf);
	Memory mem = m_datagram.build(writer.getMemoryBuffer());
	if (mem.isNotEmpty()) {
		if (flagDatagram) {
			m_queueOutputChannels.pushBack(1, mem, m_limitQueueDatagram);
		} else {
			m_queueOutputChannels.pushBack(0, mem);
		}
		write();
	}
}

void STunnelServiceSession::sendSessionMessage(sl_uint8 method, const void* buf, sl_uint32 n, sl_bool flagDatagram)
{
	MemoryBuffer buffer;
	buffer.add(buf, n);
	sendSessionMessage(method, buffer, flagDatagram);
}

void STunnelServiceSession::receiveSessionMessage(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	sl_uint8 method = data[0];
	switch (method) {
	case 10: // Raw IP Packet
		receiveRawIP(data + 1, size - 1);
		break;
	case 20: // Open TCP Port
		receiveOpenPort_TCP(data + 1, size - 1);
		break;
	case 21: // Close TCP Port
		receiveClosePort_TCP(data + 1, size - 1);
		break;
	case 22: // TCP Data
		receiveData_TCP(data + 1, size - 1);
		break;
	case 23: // TCP Data Reply
		receiveDataReply_TCP(data + 1, size - 1);
		break;
	case 32: // UDP Data
		receiveData_UDP(data + 1, size - 1);
		break;
	case 40: // DNS Request
		receiveDNSRequest(data + 1, size - 1);
		break;
	}
}

void STunnelServiceSession::sendRawIP(const void* _ip, sl_uint32 size)
{
	if (size < sizeof(IPv4HeaderFormat)) {
		return;
	}
	IPv4HeaderFormat* ip = (IPv4HeaderFormat*)_ip;
	Memory mem = Zlib::compressRaw(ip, size);
	if (mem.isNotEmpty()) {
		MemoryBuffer buf;
		buf.add(mem);
		sendSessionMessage(10, buf, !(ip->isTCP()));
	}
}

void STunnelServiceSession::receiveRawIP(const void* data, sl_uint32 size)
{
	Ref<STunnelServer> server = m_server.lock();
	if (server.isNotNull()) {
		Memory mem = Zlib::decompressRaw(data, size);
		if (mem.isNotEmpty()) {
			server->writeIPToNAT(this, mem.getBuf(), (sl_uint32)(mem.getSize()));
		}
	}
}

#define MESSAGE_SIZE 65536
#define TCP_STREAM_PENDING_SIZE 655360
#define TCP_STREAM_PENDING_SIZE_ERROR 6553600

void STunnelServiceSession::receiveOpenPort_TCP(const void* data, sl_uint32 size)
{
	MemoryReader reader(data, size);
	sl_uint64 portId;
	if (!(reader.readUint64(&portId))) {
		return;
	}
	String hostAddress;
	if (!(reader.readString(&hostAddress))) {
		return;
	}
	Ref<AsyncLoop> loop = getAsyncLoop();
	if (loop.isNotNull()) {
		loop->addTask(SLIB_CALLBACK_WEAKREF(STunnelServiceSession, _openPort_TCP, this, portId, hostAddress));
	}
}

void STunnelServiceSession::sendClosePort_TCP(sl_uint64 portId)
{
	sl_uint8 buf[8];
	MIO::writeUint64LE(buf, portId);
	sendSessionMessage(21, buf, 8, sl_false);
}

void STunnelServiceSession::receiveClosePort_TCP(const void* data, sl_uint32 size)
{
	if (size == 8) {
		sl_uint64 portId = MIO::readUint64LE(data);
		m_portsTCP.remove(portId);
	}
}

void STunnelServiceSession::sendData_TCP(sl_uint64 portId, const void* data, sl_uint32 size)
{
	if (size == 0) {
		return;
	}
	Memory mem = Zlib::compressRaw(data, size);
	if (mem.isNotEmpty()) {
		MemoryWriter writer;
		writer.writeUint64(portId);
		writer.write(mem);
		sendSessionMessage(22, writer.getMemoryBuffer(), sl_false);
	}
}

void STunnelServiceSession::receiveData_TCP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size > 8) {
		sl_uint64 portId = MIO::readUint64LE(data);
		Ref<TCP_Port> port = m_portsTCP.getValue(portId, Ref<TCP_Port>::null());
		if (port.isNotNull()) {
			Memory mem = Zlib::decompressRaw(data + 8, size - 8);
			if (mem.isNotEmpty()) {
				MutexLocker lock(&(port->lock));
				port->sizeClientPendingData += (sl_uint32)(mem.getSize());
				if (port->sizeClientPendingData > TCP_STREAM_PENDING_SIZE_ERROR) {
					_closePort_TCP(port->id);
				} else {
					if (!port->socket->writeFromMemory(mem, WeakRef<STunnelServiceSession>(this))) {
						_closePort_TCP(port->id);
					}
				}
			} else {
				_closePort_TCP(port->id);
			}			
		}
	}
}

void STunnelServiceSession::sendDataReply_TCP(sl_uint64 portId, sl_uint32 sizeReceived)
{
	sl_uint8 buf[12];
	MIO::writeUint64LE(buf, portId);
	MIO::writeUint64LE(buf + 8, sizeReceived);
	sendSessionMessage(23, buf, 12, sl_false);
}

void STunnelServiceSession::receiveDataReply_TCP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size == 12) {
		sl_uint64 portId = MIO::readUint64LE(data);
		sl_uint32 sizeReceived = MIO::readUint32LE(data + 8);
		Ref<TCP_Port> port = m_portsTCP.getValue(portId, Ref<TCP_Port>::null());
		if (port.isNotNull()) {
			MutexLocker lock(&(port->lock));
			if (port->sizeServerPendingData > sizeReceived) {
				port->sizeServerPendingData -= sizeReceived;
			} else {
				port->sizeServerPendingData = 0;
			}
			_receivePort_TCP(port.get());
		}
	}
}

void STunnelServiceSession::_openPort_TCP(sl_uint64 portId, String hostAddress)
{
	Ref<AsyncLoop> loop = getAsyncLoop();
	if (loop.isNull()) {
		return;
	}
	SocketAddress address;
	if (!(address.setHostAddress(hostAddress))) {
		_closePort_TCP(portId);
		return;
	}
	Ref<TCP_Port> port = new TCP_Port;
	if (port.isNull()) {
		_closePort_TCP(portId);
		return;
	}
	port->id = portId;
	port->memReceive = Memory::create(MESSAGE_SIZE);
	if (port->memReceive.isNull()) {
		_closePort_TCP(portId);
		return;
	}
	port->socket = AsyncTcpSocket::create(loop);
	if (port->socket.isNull()) {
		_closePort_TCP(portId);
		return;
	}
	port->socket->setUserObject(0, WeakRef<TCP_Port>(port));
	if (port->socket->connect(address, WeakRef<STunnelServiceSession>(this))) {
		m_portsTCP.put(portId, port);
	} else {
		_closePort_TCP(portId);
	}
}

void STunnelServiceSession::_closePort_TCP(sl_uint64 portId)
{
	m_portsTCP.remove(portId);
	sendClosePort_TCP(portId);
}

void STunnelServiceSession::_receivePort_TCP(TCP_Port* port)
{
	MutexLocker lock(&(port->lock));
	if (port->flagReading) {
		return;
	}
	if (port->sizeServerPendingData < TCP_STREAM_PENDING_SIZE) {
		port->flagReading = sl_true;
		if (!(port->socket->readToMemory(port->memReceive, WeakRef<STunnelServiceSession>(this)))) {
			_closePort_TCP(port->id);
		}
	}
}

void STunnelServiceSession::onConnect(AsyncTcpSocket* socket, const SocketAddress& address, sl_bool flagError)
{
	PtrLocker<TCP_Port> port(Ptr<TCP_Port>::from(socket->getUserObject(0)));
	if (port.isNotNull()) {
		if (flagError) {
			_closePort_TCP(port->id);
		} else {
			port->flagConnected = sl_true;
			Memory mem;
			while (port->queueSend.pop(&mem)) {
				port->socket->writeFromMemory(mem, WeakRef<STunnelServiceSession>(this));
			}
			_receivePort_TCP(port.get());
		}
	}
}

void STunnelServiceSession::_onReadPort_TCP(TCP_Port* port, void* buf, sl_uint32 size, sl_bool flagError)
{
	MutexLocker lock(&(port->lock));
	port->flagReading = sl_false;
	if (size > 0) {
		sendData_TCP(port->id, buf, size);
		port->sizeServerPendingData += size;
	}
	if (flagError) {
		_closePort_TCP(port->id);
	} else {
		_receivePort_TCP(port);
	}
}

void STunnelServiceSession::_onWritePort_TCP(TCP_Port* port, void* buf, sl_uint32 size, sl_bool flagError)
{
	MutexLocker lock(&(port->lock));
	if (size > 0) {
		if (port->sizeClientPendingData > size) {
			port->sizeClientPendingData -= size;
		} else {
			port->sizeClientPendingData = 0;
		}
		sendDataReply_TCP(port->id, size);
	}
	if (flagError) {
		_closePort_TCP(port->id);
	}
}

void STunnelServiceSession::sendData_UDP(sl_uint64 portId, const SocketAddress& addressFrom, const void* data, sl_uint32 size)
{
	if (size == 0) {
		return;
	}
	Memory mem = Zlib::compressRaw(data, size);
	if (mem.isNotEmpty()) {
		MemoryWriter writer;
		writer.writeUint64(portId);
		writer.writeUint32(addressFrom.ip.getIPv4().toInt());
		writer.writeUint16((sl_uint16)(addressFrom.port));
		writer.write(mem);
		sendSessionMessage(32, writer.getMemoryBuffer(), sl_true);
	}
}

void STunnelServiceSession::receiveData_UDP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size > 14) {
		sl_uint64 portId = MIO::readUint64LE(data);
		SocketAddress address;
		address.ip = IPv4Address(MIO::readUint32LE(data + 8));
		address.port = MIO::readUint16LE(data + 12);
		Ref<UDP_Port> port = _getPort_UDP(portId);
		if (port.isNotNull()) {
			Memory mem = Zlib::decompressRaw(data + 14, size - 14);
			if (mem.isNotEmpty()) {
				port->socket->sendTo(address, mem, WeakRef<STunnelServiceSession>(this));
			}
		}
	}
}

Ref<STunnelServiceSession::UDP_Port> STunnelServiceSession::_getPort_UDP(sl_uint64 portId)
{
	Ref<UDP_Port> port = m_portsUDP.getValue(portId, Ref<UDP_Port>::null());
	if (port.isNotNull()) {
		return Ref<UDP_Port>::null();
	}
	Ref<AsyncLoop> loop = getAsyncLoop();
	if (loop.isNull()) {
		return Ref<UDP_Port>::null();
	}
	port = new UDP_Port;
	if (port.isNull()) {
		return Ref<UDP_Port>::null();
	}
	port->id = portId;
	port->memReceive = Memory::create(MESSAGE_SIZE);
	if (port->memReceive.isNull()) {
		return Ref<UDP_Port>::null();
	}
	port->socket = AsyncUdpSocket::create(loop);
	if (port->socket.isNull()) {
		return Ref<UDP_Port>::null();
	}
	port->socket->setUserObject(0, WeakRef<UDP_Port>(port));
	_receivePort_UDP(port.get());
	return port;
}

void STunnelServiceSession::_receivePort_UDP(UDP_Port* port)
{
	port->socket->receiveFrom(port->memReceive, WeakRef<STunnelServiceSession>(this));
}

void STunnelServiceSession::onReceiveFrom(AsyncUdpSocket* socket, void* data, sl_uint32 sizeReceive, const SocketAddress& address, sl_bool flagError)
{
	PtrLocker<UDP_Port> port(Ptr<UDP_Port>::from(socket->getUserObject(0)));
	if (port.isNotNull()) {
		if (sizeReceive) {
			sendData_UDP(port->id, address, data, sizeReceive);
		}
		_receivePort_UDP(port.get());
	}
}

void STunnelServiceSession::onSendTo(AsyncUdpSocket* socket, void* data, sl_uint32 sizeSent, const SocketAddress& address, sl_bool flagError)
{
}

void STunnelServiceSession::receiveDNSRequest(const void* data, sl_uint32 size)
{
	MemoryReader reader(data, size);
	String dns;
	if (reader.readString(&dns)) {
		Ref<AsyncLoop> loop = getAsyncLoop();
		if (loop.isNotNull()) {
			loop->addTask(SLIB_CALLBACK_WEAKREF(STunnelServiceSession, _resolveDNS, this, dns));
		}
	}
}

void STunnelServiceSession::sendDNSResponse(const String& dns, const IPv4Address& ip)
{
	MemoryWriter writer;
	writer.writeString(dns);
	writer.writeUint32(ip.toInt());
	sendSessionMessage(41, writer.getMemoryBuffer(), sl_true);
}

void STunnelServiceSession::_resolveDNS(String dns)
{
	IPv4Address ip = NetworkAddress::getIPv4AddressFromHostName(dns);
	sendDNSResponse(dns, ip);
}

Ref<AsyncLoop> STunnelServiceSession::getAsyncLoop()
{
	Ref<STunnelServer> server = m_server.lock();
	if (server.isNotNull()) {
		return server->m_asyncLoop.lock();
	}
	return Ref<AsyncLoop>::null();
}
SLIB_SNET_NAMESPACE_END

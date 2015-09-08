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
				m_captureNat = NetCapture::createRawPacket(ncp);
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
		m_server = AsyncTcpSecureStreamServer::create(ssp, param.addressBind);
		if (m_server.isNotNull()) {
			SLIB_LOG(TAG, "Server started");
		} else {
			SLIB_LOG_ERROR(TAG, "Server failed");
			return sl_false;
		}
	}

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


#define PACKET_SIZE 2000

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
	if (ip && lenIP < PACKET_SIZE) {
		char data[PACKET_SIZE];
		Base::copyMemory(data, ip, lenIP);
		
		Ptr<Referable> userObject;
		if (m_tableNat.translateIncomingPacket(data, lenIP, &userObject)) {
			PtrLocker<STunnelServiceSession> client(Ptr<STunnelServiceSession>::from(userObject));
			if (client.isNotNull()) {
				client->sendRawIP(data, lenIP, ip->isTCP());
			}
		}
	}
}

void STunnelServer::writeIPToNAT(STunnelServiceSession* session, const void* ip, sl_uint32 size)
{
	if (size > PACKET_SIZE) {
		return;
	}
	Ref<NetCapture> dev = m_captureNat;
	if (dev.isNull()) {
		return;
	}
	char bufFrame[PACKET_SIZE + 100];
	sl_uint32 lenFrame;
	sl_uint8* bufIP;
	sl_uint32 lenIP = size;
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
		bufIP = frameOut->getContent();
		Base::copyMemory(bufIP, ip, lenIP);
		lenFrame = lenIP + EthernetFrameFormat::getHeaderSize();
	} else {
		bufIP = (sl_uint8*)bufFrame;
		Base::copyMemory(bufIP, ip, lenIP);
		lenFrame = lenIP;
	}
	Ptr<Referable> userObject = WeakRef<STunnelServiceSession>(session);
	if (m_tableNat.translateOutgoingPacket(bufIP, lenIP, &userObject)) {
		dev->sendPacket(bufFrame, lenFrame);
	}
}

void STunnelServer::onConnectedSecureStream(AsyncSecureStream* securedStream, sl_bool flagError)
{
	Ref<STunnelServiceSession> session = new STunnelServiceSession;
	if (session.isNotNull()) {
		session->m_stream = securedStream;
		session->m_server = this;
		session->m_limitQueueOutputStream = m_param.session_stream_output_queue_limit;
		session->m_limitQueueOutputDatagram = m_param.session_datagram_output_queue_limit;
		session->m_bufRead = Memory::create(102400);
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
	m_limitQueueOutputStream = sl_false;
	m_limitQueueOutputDatagram = sl_false;
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
	Memory output;
	while (m_queueOutputChannels.popFront(&output)) {
		if (output.isNotEmpty()) {
			break;
		}
	}
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
	MutexLocker lock(getLocker());
	m_flagWriting = sl_false;
	if (flagError) {
		close();
	} else {
		write();
	}
}

void STunnelServiceSession::sendSessionMessage(sl_uint8 method, const void* data, sl_uint32 n, sl_bool flagStream)
{
	Ref<AsyncSecureStream> stream = m_stream;
	if (stream.isNull()) {
		return;
	}
	SLIB_SCOPED_ARRAY(sl_uint8, buf, 1 + n);
	if (!buf) {
		return;
	}
	buf[0] = method;
	Base::copyMemory(buf + 1, data, n);
	Memory mem = m_datagram.build(buf, 1 + n);
	if (mem.isNotEmpty()) {
		if (flagStream) {
			m_queueOutputChannels.pushBack(0, mem, m_limitQueueOutputStream);
		} else {
			m_queueOutputChannels.pushBack(1, mem, m_limitQueueOutputDatagram);
		}
		write();
	}
}

void STunnelServiceSession::receiveSessionMessage(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	sl_uint8 method = data[0];
	switch (method) {
	case 10:
		receiveRawIP(data + 1, size - 1);
		break;
	}
}

void STunnelServiceSession::sendRawIP(const void* ip, sl_uint32 size, sl_bool flagStream)
{
	Memory mem = Zlib::compressRaw(ip, size);
	if (mem.isNotEmpty()) {
		sendSessionMessage(10, mem.getBuf(), (sl_uint32)(mem.getSize()), flagStream);
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

SLIB_SNET_NAMESPACE_END

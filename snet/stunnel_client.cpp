#include "../../../inc/slibx/snet/stunnel_client.h"

#include "../../../inc/slib/crypto/zlib.h"
#include "../../../inc/slib/core/log.h"

SLIB_SNET_NAMESPACE_BEGIN
class _STunnelClient_TcpStream : public AsyncStream
{
public:
	sl_bool m_flagClosed;
	sl_uint64 m_port;
	WeakRef<STunnelClient> m_client;
	MemoryQueue m_bufReceive;
	sl_uint32 m_sizeClientPendingData;
	sl_uint32 m_sizeServerPendingData;
	Queue< Ref<AsyncStreamRequest> > m_requestsRead;
	Queue< Ref<AsyncStreamRequest> > m_requestsWrite;

public:
	_STunnelClient_TcpStream();

	void close();
	sl_bool isOpened();
	sl_bool read(void* data, sl_uint32 size, const Ptr<IAsyncStreamListener>& listener, Referable* ref);
	sl_bool write(void* data, sl_uint32 size, const Ptr<IAsyncStreamListener>& listener, Referable* ref);

	sl_bool read(STunnelClient* client, AsyncStreamRequest* req);
	void processReadDone(Ref<AsyncStreamRequest> req);
	void processReadError(Ref<AsyncStreamRequest> req);

	sl_bool write(STunnelClient* client, AsyncStreamRequest* req);
	void processWriteDone(Ref<AsyncStreamRequest> req);
	void processWriteError(Ref<AsyncStreamRequest> req);


	friend class STunnelClient;
};

STunnelClient::STunnelClient()
{
	m_flagClosed = sl_false;
	m_flagConnecting = sl_false;
	m_flagReading = sl_false;
	m_flagWriting = sl_false;
	m_limitQueueDatagram = 0;
	m_timeLastReceive = Time::now();
	m_timeLastSend = m_timeLastReceive;
	m_timeLastConnecting = m_timeLastReceive;
}

STunnelClient::~STunnelClient()
{
	close();
}

Ref<STunnelClient> STunnelClient::create(const STunnelClientParam& param, const Ref<AsyncLoop>& _loop)
{
	Ref<AsyncLoop> loop = _loop;
	if (loop.isNotNull()) {
		loop = AsyncLoop::getDefault();
		if (loop.isNull()) {
			return Ref<STunnelClient>::null();
		}
	}
	Ref<STunnelClient> ret = new STunnelClient;
	if (ret.isNotNull()) {
		if (!(ret->_initialize(param))) {
			ret.setNull();
		}
		ret->m_asyncLoop = loop;
	}
	return ret;
}

Ref<STunnelClient> STunnelClient::create(const STunnelClientParam& param)
{
	return create(param, Ref<AsyncLoop>::null());
}

sl_bool STunnelClient::_initialize(const STunnelClientParam& param)
{
	m_bufRead = Memory::create(102400);
	if (m_bufRead.isEmpty()) {
		return sl_false;
	}
	if (!(m_queueOutputChannels.setChannelsCount(2))) {
		return sl_false;
	}
	
	m_serviceAddress = param.serviceAddress;
	if (m_serviceAddress.isInvalid()) {
		return sl_false;
	}
	m_masterKey = param.masterKey;
	m_limitQueueDatagram = param.datagram_queue_limit;
	setListener(param.listener);
	return sl_true;
}

void STunnelClient::close()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	m_flagClosed = sl_true;
	if (m_stream.isNotNull()) {
		m_stream->close();
		m_stream.setNull();
	}
}

void STunnelClient::sendRawIPPacket(const void* ip, sl_uint32 size)
{
	sendRawIP(ip, size);
}

void STunnelClient::sendUDP(sl_uint64 portId, const SocketAddress& addressTo, const void* data, sl_uint32 n)
{
	sendData_UDP(portId, addressTo, data, n);
}

sl_bool STunnelClient::isConnecting()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return sl_false;
	}
	return m_flagConnecting;
}

sl_bool STunnelClient::isConnected()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return sl_false;
	}
	if (m_flagConnecting) {
		return sl_false;
	}
	return m_stream.isNotNull();
}

void STunnelClient::connect()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	if (m_flagConnecting) {
		return;
	}
	if (m_stream.isNotNull()) {
		return;
	}

	m_timeLastConnecting = Time::now();

	AsyncSecureStreamClientParam ssp;
	ssp.masterKey = m_masterKey;
	ssp.listener = WeakRef<STunnelClient>(this);
	
	m_flagReading = sl_false;
	m_flagWriting = sl_false;
	m_datagram.clear();
	
	m_stream = AsyncTcpSecureStreamClient::create(ssp, m_serviceAddress);
	if (m_stream.isNull()) {
		return;
	}
	m_flagConnecting = sl_true;
}

void STunnelClient::reconnect()
{
	MutexLocker lock(getLocker());
	if (m_stream.isNotNull()) {
		m_stream->close();
		m_stream.setNull();
	}
	m_flagConnecting = sl_false;
	connect();
}

void STunnelClient::onConnectedSecureStream(AsyncSecureStream* securedStream, sl_bool flagError)
{
	MutexLocker lock(getLocker());
	if (m_stream.get() != securedStream) {
		return;
	}
	m_flagConnecting = sl_false;
	if (m_flagClosed) {
		return;
	}
	if (flagError) {
		m_stream.setNull();
	} else {
		m_timeLastReceive = Time::now();
		m_timeLastSend = m_timeLastReceive;
		_read();
	}
}

void STunnelClient::_read()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	if (m_flagConnecting) {
		return;
	}
	if (m_flagReading) {
		return;
	}
	Ref<AsyncSecureStream> stream = m_stream;
	if (stream.isNull()) {
		return;
	}
	m_flagReading = sl_true;
	if (!(stream->readToMemory(m_bufRead, WeakRef<STunnelClient>(this)))) {
		m_flagReading = sl_false;
		reconnect();
	}
}

void STunnelClient::onRead(AsyncStream* stream, void* data, sl_uint32 sizeRead, Referable* ref, sl_bool flagError)
{
	MutexLocker lock(getLocker());
	if (m_stream.get() != stream) {
		return;
	}
	m_flagReading = sl_false;
	if (m_flagClosed) {
		return;
	}
	if (sizeRead > 0) {
		Queue<Memory> datagrams;
		if (m_datagram.parse(data, sizeRead, datagrams)) {
			lock.unlock();
			Memory datagram;
			while (datagrams.pop(&datagram)) {
				if (datagram.isNotEmpty()) {
					receiveSessionMessage(datagram.getBuf(), (sl_uint32)(datagram.getSize()));
				}
			}
		} else {
			reconnect();
			return;
		}
	}
	if (flagError) {
		reconnect();
	} else {
		m_timeLastReceive = Time::now();
		_read();
	}
}

void STunnelClient::_write()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	if (m_flagConnecting) {
		return;
	}
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
	if (!(stream->writeFromMemory(output, WeakRef<STunnelClient>(this)))) {
		m_flagWriting = sl_false;
	}
}

void STunnelClient::onWrite(AsyncStream* stream, void* data, sl_uint32 sizeWritten, Referable* ref, sl_bool flagError)
{
	MutexLocker lock(getLocker());
	if (m_stream.get() != stream) {
		return;
	}
	m_flagWriting = sl_false;
	if (m_flagClosed) {
		return;
	}
	if (flagError) {
		reconnect();
	} else {
		m_timeLastSend = Time::now();
		_write();
	}
}

void STunnelClient::sendSessionMessage(sl_uint8 method, MemoryBuffer& buf, sl_bool flagDatagram)
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
		_write();
	}
}

void STunnelClient::sendSessionMessage(sl_uint8 method, const void* buf, sl_uint32 n, sl_bool flagDatagram)
{
	MemoryBuffer buffer;
	buffer.add(buf, n);
	sendSessionMessage(method, buffer, flagDatagram);
}

void STunnelClient::receiveSessionMessage(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	sl_uint8 method = data[0];
	switch (method) {
	case 10: // Raw IP Packet
		receiveRawIP(data + 1, size - 1);
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
	}
}

void STunnelClient::sendRawIP(const void* ip, sl_uint32 size)
{
	Memory mem = Zlib::compressRaw(ip, size);
	if (mem.isNotEmpty()) {
		sendSessionMessage(10, mem.getBuf(), (sl_uint32)(mem.getSize()), sl_true);
	}
}

void STunnelClient::receiveRawIP(const void* data, sl_uint32 size)
{
	PtrLocker<ISTunnelClientListener> listener(getListener());
	if (listener.isNotNull()) {
		Memory mem = Zlib::decompressRaw(data, size);
		if (mem.isNotEmpty()) {
			listener->onSTunnelRawIP(this, mem.getBuf(), (sl_uint32)(mem.getSize()));
		}
	}
}

#define TCP_STREAM_PENDING_SIZE 655360
#define TCP_STREAM_PENDING_SIZE_ERROR 6553600

void STunnelClient::sendOpenPort_TCP(sl_uint64 portId, const String& hostAddress)
{
	MemoryWriter writer;
	writer.writeUint64(portId);
	writer.writeString(hostAddress);
	sendSessionMessage(20, writer.getMemoryBuffer(), sl_false);
}

void STunnelClient::sendClosePort_TCP(sl_uint64 portId)
{
	Ref<_STunnelClient_TcpStream> port = m_portsTCP.getValue(portId, Ref<_STunnelClient_TcpStream>::null());
	if (port.isNotNull()) {
		sl_uint8 buf[8];
		MIO::writeUint64LE(buf, portId);
		sendSessionMessage(21, buf, 8, sl_false);
	}
}

void STunnelClient::receiveClosePort_TCP(const void* data, sl_uint32 size)
{
	if (size == 8) {
		sl_uint64 portId = MIO::readUint64LE(data);
		Ref<_STunnelClient_TcpStream> port = m_portsTCP.getValue(portId, Ref<_STunnelClient_TcpStream>::null());
		closePort_TCP(portId);
		if (port.isNotNull()) {
			port->close();
		}
	}
}

void STunnelClient::sendData_TCP(sl_uint64 portId, const void* data, sl_uint32 size)
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

void STunnelClient::receiveData_TCP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size > 8) {
		sl_uint64 portId = MIO::readUint64LE(data);
		Ref<_STunnelClient_TcpStream> port = m_portsTCP.getValue(portId, Ref<_STunnelClient_TcpStream>::null());
		if (port.isNotNull()) {
			Memory mem = Zlib::decompressRaw(data + 8, size - 8);
			if (mem.isNotEmpty()) {
				MutexLocker lock(port->getLocker());
				port->m_sizeServerPendingData += (sl_uint32)(mem.getSize());
				if (port->m_sizeServerPendingData > TCP_STREAM_PENDING_SIZE_ERROR) {
					port->close();
				} else {
					if (port->m_bufReceive.push(mem)) {
						while (port->m_bufReceive.getSize() > 0) {
							Ref<AsyncStreamRequest> req;
							if (port->m_requestsRead.pop(&req)) {
								if (req.isNotNull()) {
									if (!(port->read(this, req.get()))) {
										break;
									}
								}
							} else {
								break;
							}
						}
					} else {
						port->close();
					}
				}
			} else {
				port->close();
			}
		}
	}
}

void STunnelClient::sendDataReply_TCP(sl_uint64 portId, sl_uint32 sizeReceived)
{
	sl_uint8 buf[12];
	MIO::writeUint64LE(buf, portId);
	MIO::writeUint64LE(buf + 8, sizeReceived);
	sendSessionMessage(23, buf, 12, sl_false);
}

void STunnelClient::receiveDataReply_TCP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size == 12) {
		sl_uint64 portId = MIO::readUint64LE(data);
		sl_uint32 sizeReceived = MIO::readUint32LE(data + 8);
		Ref<_STunnelClient_TcpStream> port = m_portsTCP.getValue(portId, Ref<_STunnelClient_TcpStream>::null());
		if (port.isNotNull()) {
			MutexLocker lock(port->getLocker());
			if (port->m_sizeClientPendingData > sizeReceived) {
				port->m_sizeClientPendingData -= sizeReceived;
			} else {
				port->m_sizeClientPendingData = 0;
			}
			while (port->m_sizeClientPendingData < TCP_STREAM_PENDING_SIZE) {
				Ref<AsyncStreamRequest> req;
				if (port->m_requestsWrite.pop(&req)) {
					if (req.isNotNull()) {
						if (!(port->write(this, req.get()))) {
							break;
						}
					}
				} else {
					break;
				}
			}
		}
	}
}

void STunnelClient::closePort_TCP(sl_uint64 port)
{
	m_portsTCP.remove(port, sl_true);
}


Ref<AsyncStream> STunnelClient::createTCPStream(sl_uint64 portId, const String& hostAddress)
{
	Ref<_STunnelClient_TcpStream> ret;
	ret = new _STunnelClient_TcpStream;
	if (ret.isNotNull()) {
		ret->setLoop(getAsyncLoop());
		ret->m_client = this;
		ret->m_port = portId;
		if (m_portsTCP.put(portId, ret)) {
			sendOpenPort_TCP(portId, hostAddress);
		} else {
			ret.setNull();
		}
	}
	return ret;
}

_STunnelClient_TcpStream::_STunnelClient_TcpStream()
{
	m_flagClosed = sl_false;
	m_sizeClientPendingData = 0;
	m_sizeServerPendingData = 0;
}

void _STunnelClient_TcpStream::close()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	m_flagClosed = sl_true;
	Ref<STunnelClient> client = m_client.lock();
	m_client.setNull();
	if (client.isNotNull()) {
		client->sendClosePort_TCP(m_port);
		client->closePort_TCP(m_port);
		Ref<AsyncLoop> loop = getLoop();
		if (loop.isNotNull()) {
			Ref<AsyncStreamRequest> request;
			while (m_requestsRead.pop(&request)) {
				if (request.isNotNull()) {
					loop->addTask(SLIB_CALLBACK_WEAKREF(_STunnelClient_TcpStream, processReadError, this, request), sl_false);
				}
			}
			while (m_requestsWrite.pop(&request)) {
				if (request.isNotNull()) {
					loop->addTask(SLIB_CALLBACK_WEAKREF(_STunnelClient_TcpStream, processWriteError, this, request), sl_false);
				}
			}
		}
	}
}

sl_bool _STunnelClient_TcpStream::isOpened()
{
	return !m_flagClosed;
}

sl_bool _STunnelClient_TcpStream::read(void* data, sl_uint32 size, const Ptr<IAsyncStreamListener>& listener, Referable* ref)
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return sl_false;
	}
	Ref<STunnelClient> client = m_client.lock();
	if (client.isNull()) {
		return sl_false;
	}
	Ref<AsyncStreamRequest> req = AsyncStreamRequest::createRead(data, size, ref, listener);
	if (req.isNull()) {
		return sl_false;
	}
	if (read(client.get(), req.get())) {
		return sl_true;
	} else {
		return m_requestsRead.push(req) != sl_null;
	}
}

sl_bool _STunnelClient_TcpStream::read(STunnelClient* client, AsyncStreamRequest* req)
{
	sl_size sizeRead = m_bufReceive.pop(req->data, req->size);
	if (sizeRead > 0) {
		if (m_sizeServerPendingData > req->size) {
			m_sizeServerPendingData -= req->size;
		} else {
			m_sizeServerPendingData = 0;
		}
		req->size = (sl_uint32)sizeRead;
		client->sendDataReply_TCP(m_port, req->size);
		if (req->listener.isNotNull()) {
			Ref<AsyncLoop> loop = getLoop();
			if (loop.isNotNull()) {
				loop->addTask(SLIB_CALLBACK_WEAKREF(_STunnelClient_TcpStream, processReadDone, this, Ref<AsyncStreamRequest>(req)), sl_false);
			}
		}
		return sl_true;
	}
	return sl_false;
}

void _STunnelClient_TcpStream::processReadDone(Ref<AsyncStreamRequest> req)
{
	if (req.isNotNull()) {
		PtrLocker<IAsyncStreamListener> listener(req->listener);
		if (listener.isNotNull()) {
			listener->onRead(this, req->data, req->size, req->refData.getObject(), sl_false);
		}
	}
}

void _STunnelClient_TcpStream::processReadError(Ref<AsyncStreamRequest> req)
{
	if (req.isNotNull()) {
		PtrLocker<IAsyncStreamListener> listener(req->listener);
		if (listener.isNotNull()) {
			listener->onRead(this, req->data, 0, req->refData.getObject(), sl_true);
		}
	}
}

sl_bool _STunnelClient_TcpStream::write(void* data, sl_uint32 size, const Ptr<IAsyncStreamListener>& listener, Referable* ref)
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return sl_false;
	}
	Ref<STunnelClient> client = m_client.lock();
	if (client.isNull()) {
		return sl_false;
	}
	Ref<AsyncStreamRequest> req = AsyncStreamRequest::createWrite(data, size, ref, listener);
	if (req.isNull()) {
		return sl_false;
	}
	if (write(client.get(), req.get())) {
		return sl_true;
	} else {
		return m_requestsWrite.push(req) != sl_null;
	}
}

sl_bool _STunnelClient_TcpStream::write(STunnelClient* client, AsyncStreamRequest* req)
{
	if (m_sizeClientPendingData < TCP_STREAM_PENDING_SIZE) {
		client->sendData_TCP(m_port, req->data, req->size);
		m_sizeClientPendingData += req->size;
		if (req->listener.isNotNull()) {
			Ref<AsyncLoop> loop = getLoop();
			if (loop.isNotNull()) {
				loop->addTask(SLIB_CALLBACK_WEAKREF(_STunnelClient_TcpStream, processWriteDone, this, Ref<AsyncStreamRequest>(req)), sl_false);
			}
		}
		return sl_true;
	}
	return sl_false;
}

void _STunnelClient_TcpStream::processWriteDone(Ref<AsyncStreamRequest> req)
{
	if (req.isNotNull()) {
		PtrLocker<IAsyncStreamListener> listener(req->listener);
		if (listener.isNotNull()) {
			listener->onWrite(this, req->data, req->size, req->refData.getObject(), sl_false);
		}
	}
}

void _STunnelClient_TcpStream::processWriteError(Ref<AsyncStreamRequest> req)
{
	if (req.isNotNull()) {
		PtrLocker<IAsyncStreamListener> listener(req->listener);
		if (listener.isNotNull()) {
			listener->onWrite(this, req->data, 0, req->refData.getObject(), sl_true);
		}
	}
}

void STunnelClient::sendData_UDP(sl_uint64 portId, const SocketAddress& addressTo, const void* data, sl_uint32 size)
{
	if (size == 0) {
		return;
	}
	Memory mem = Zlib::compressRaw(data, size);
	if (mem.isNotEmpty()) {
		MemoryWriter writer;
		writer.writeUint64(portId);
		writer.writeUint32(addressTo.ip.getIPv4().toInt());
		writer.writeUint16((sl_uint16)(addressTo.port));
		writer.write(mem);
		sendSessionMessage(32, writer.getMemoryBuffer(), sl_true);
	}
}

void STunnelClient::receiveData_UDP(const void* _data, sl_uint32 size)
{
	const sl_uint8* data = (const sl_uint8*)_data;
	if (size > 14) {
		sl_uint64 portId = MIO::readUint64LE(data);
		SocketAddress address;
		address.ip = IPv4Address(MIO::readUint32LE(data + 8));
		address.port = MIO::readUint16LE(data + 12);
		PtrLocker<ISTunnelClientListener> listener(getListener());
		if (listener.isNotNull()) {
			listener->onSTunnelDataUDP(this, portId, address, data + 14, size - 14);
		}
	}
}

SLIB_SNET_NAMESPACE_END

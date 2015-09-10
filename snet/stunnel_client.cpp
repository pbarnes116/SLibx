#include "../../../inc/slibx/snet/stunnel_client.h"

#include "../../../inc/slib/crypto/zlib.h"
#include "../../../inc/slib/core/log.h"

SLIB_SNET_NAMESPACE_BEGIN
STunnelClient::STunnelClient()
{
	m_flagClosed = sl_false;
	m_flagConnecting = sl_false;
	m_flagReading = sl_false;
	m_flagWriting = sl_false;
	m_limitQueueOutputStream = 0;
	m_limitQueueOutputDatagram = 0;
	m_timeLastReceive = Time::now();
	m_timeLastSend = m_timeLastReceive;
	m_timeLastConnecting = m_timeLastReceive;
}

STunnelClient::~STunnelClient()
{
	close();
}

Ref<STunnelClient> STunnelClient::create(const STunnelClientParam& param)
{
	Ref<STunnelClient> ret = new STunnelClient;
	if (ret.isNotNull()) {
		if (!(ret->_initialize(param))) {
			ret.setNull();
		}
	}
	return ret;
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
	m_limitQueueOutputStream = param.session_stream_output_queue_limit;
	m_limitQueueOutputDatagram = param.session_datagram_output_queue_limit;
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

void STunnelClient::sendRawIP(const void* ip, sl_uint32 size, sl_bool flagStream)
{
	_sendRawIP(ip, size, flagStream);
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
					_receiveSessionMessage(datagram.getBuf(), (sl_uint32)(datagram.getSize()));
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

#define PACKET_SIZE 2000

void STunnelClient::_sendSessionMessage(sl_uint8 method, const void* data, sl_uint32 n, sl_bool flagStream)
{
	if (m_flagClosed) {
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
		_write();
	}
}

void STunnelClient::_receiveSessionMessage(const void* _data, sl_uint32 size)
{
	if (m_flagClosed) {
		return;
	}
	const sl_uint8* data = (const sl_uint8*)_data;
	sl_uint8 method = data[0];
	switch (method) {
	case 10:
		_receiveRawIP(data + 1, size - 1);
		break;
	}
}

void STunnelClient::_sendRawIP(const void* ip, sl_uint32 size, sl_bool flagStream)
{
	Memory mem = Zlib::compressRaw(ip, size);
	if (mem.isNotEmpty()) {
		_sendSessionMessage(10, mem.getBuf(), (sl_uint32)(mem.getSize()), flagStream);
	}
}

void STunnelClient::_receiveRawIP(const void* data, sl_uint32 size)
{
	PtrLocker<ISTunnelClientListener> listener(getListener());
	if (listener.isNotNull()) {
		Memory mem = Zlib::decompressRaw(data, size);
		if (mem.isNotEmpty()) {
			listener->onSTunnelRawIP(this, mem.getBuf(), (sl_uint32)(mem.getSize()));
		}
	}
}

SLIB_SNET_NAMESPACE_END

#ifndef CHECKHEADER_SLIB_SNET_STUNNEL_CLIENT
#define CHECKHEADER_SLIB_SNET_STUNNEL_CLIENT

#include "definition.h"

#include <slib/network/socket_address.h>
#include <slib/network/secure_stream.h>

#include <slib/core/object.h>
#include <slib/core/string.h>
#include <slib/core/queue.h>
#include <slib/core/queue_channel.h>

SLIB_SNET_NAMESPACE_BEGIN

class STunnelClient;
class SLIB_EXPORT ISTunnelClientListener
{
public:
	virtual void onSTunnelRawIP(STunnelClient* client, const void* ip, sl_uint32 len) {}

	virtual void onSTunnelDataUDP(STunnelClient* client, sl_uint64 portId, const SocketAddress& addressFrom, const void* data, sl_uint32 n) {}

	virtual void onSTunnelDNSResponse(STunnelClient* client, const String& dns, const IPv4Address& ip) {}
};

class SLIB_EXPORT STunnelClientParam
{
public:
	SocketAddress serviceAddress;
	String masterKey;

	sl_size datagram_queue_limit;

	Ptr<ISTunnelClientListener> listener;

	STunnelClientParam()
	{
		datagram_queue_limit = 500;
	}
};

class _STunnelClient_TcpStream;
class SLIB_EXPORT STunnelClient : public Object, public IAsyncSecureStreamListener, public IAsyncStreamListener
{
protected:
	STunnelClient();
public:
	~STunnelClient();

public:
	static Ref<STunnelClient> create(const STunnelClientParam& param, const Ref<AsyncLoop>& loop);
	static Ref<STunnelClient> create(const STunnelClientParam& param);

public:
	void close();
	sl_bool isConnecting();
	sl_bool isConnected();
	void connect();
	void reconnect();

	SLIB_INLINE Ref<AsyncLoop> getAsyncLoop()
	{
		return m_asyncLoop;
	}

	SLIB_INLINE const SocketAddress& getServiceAddress()
	{
		return m_serviceAddress;
	}

	SLIB_INLINE Time getLastSentTime()
	{
		return m_timeLastSend;
	}

	SLIB_INLINE Time getLastReceivedTime()
	{
		return m_timeLastReceive;
	}

	SLIB_INLINE Time getLastConnectingTime()
	{
		return m_timeLastConnecting;
	}

	void sendRawIPPacket(const void* ip, sl_uint32 n);

	Ref<AsyncStream> createTCPStream(sl_uint64 portId, const String& hostAddress);

	void sendUDP(sl_uint64 portId, const SocketAddress& adressTo, const void* data, sl_uint32 n);

	void sendDNS(const String& dns);

public:
	SLIB_PROPERTY_INLINE(Ptr<ISTunnelClientListener>, Listener);

protected:
	sl_bool _initialize(const STunnelClientParam& param);

	void _read();
	void _write();

protected:
	void sendSessionMessage(sl_uint8 method, MemoryBuffer& buf, sl_bool flagDatagram);
	void sendSessionMessage(sl_uint8 method, const void* data, sl_uint32 n, sl_bool flagDatagram);
	void receiveSessionMessage(const void* data, sl_uint32 size);

	void sendRawIP(const void* ip, sl_uint32 n);
	void receiveRawIP(const void* data, sl_uint32 size);

	void sendOpenPort_TCP(sl_uint64 portId, const String& hostAddress);
	void sendClosePort_TCP(sl_uint64 portId);
	void receiveClosePort_TCP(const void* data, sl_uint32 size);
	void sendData_TCP(sl_uint64 portId, const void* data, sl_uint32 size);
	void receiveData_TCP(const void* data, sl_uint32 size);
	void sendDataReply_TCP(sl_uint64 portId, sl_uint32 sizeReceived);
	void receiveDataReply_TCP(const void* data, sl_uint32 size);

	void sendData_UDP(sl_uint64 portId, const SocketAddress& addressTo, const void* data, sl_uint32 size);
	void receiveData_UDP(const void* data, sl_uint32 size);

	void sendDNSRequest(const String& dns);
	void receiveDNSResponse(const void* data, sl_uint32 size);

	virtual void onConnectedSecureStream(AsyncSecureStream* securedStream, sl_bool flagError);
	virtual void onRead(AsyncStream* stream, void* data, sl_uint32 sizeRead, Referable* ref, sl_bool flagError);
	virtual void onWrite(AsyncStream* stream, void* data, sl_uint32 sizeWritten, Referable* ref, sl_bool flagError);

protected:
	void closePort_TCP(sl_uint64 port);

protected:
	SocketAddress m_serviceAddress;
	String m_masterKey;

	Ref<AsyncSecureStream> m_stream;

	DatagramStream m_datagram;
	Memory m_bufRead;

	sl_bool m_flagClosed;
	sl_bool m_flagConnecting;
	sl_bool m_flagReading;
	sl_bool m_flagWriting;
	Time m_timeLastConnecting;
	Time m_timeLastSend;
	Time m_timeLastReceive;

	sl_size m_limitQueueDatagram;
	QueueChannelArray<Memory> m_queueOutputChannels;

	Map< sl_uint64, Ref<_STunnelClient_TcpStream> > m_portsTCP;

	WeakRef<AsyncLoop> m_asyncLoop;

	friend class _STunnelClient_TcpStream;
};

SLIB_SNET_NAMESPACE_END

#endif

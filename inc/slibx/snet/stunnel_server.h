#ifndef CHECKHEADER_SLIB_SNET_STUNNEL_SERVER
#define CHECKHEADER_SLIB_SNET_STUNNEL_SERVER

#include "definition.h"

#include <slib/network/socket_address.h>
#include <slib/network/mac_address.h>
#include <slib/network/capture.h>
#include <slib/network/secure_stream.h>
#include <slib/network/nat.h>

#include <slib/core/object.h>
#include <slib/core/string.h>
#include <slib/core/map.h>
#include <slib/core/queue.h>
#include <slib/core/queue_channel.h>
#include <slib/core/expire.h>
#include <slib/core/thread.h>

SLIB_SNET_NAMESPACE_BEGIN
class SLIB_EXPORT STunnelServerParam
{
public:
	SocketAddress addressBind;
	String masterKey;

	sl_size session_datagram_queue_limit;

	sl_bool nat_enabled;
	String nat_device;
	sl_bool nat_raw_socket;
	sl_uint32 nat_port_begin;
	sl_uint32 nat_port_end;
	sl_uint32 nat_mtu_incoming;
	sl_uint32 nat_mtu_outgoing;
	sl_uint32 nat_expiring_duration;
	MacAddress nat_gateway;

	Ref<AsyncLoop> asyncLoop;

	STunnelServerParam()
	{
		addressBind.port = 28000;

		session_datagram_queue_limit = 5000;
		
		nat_enabled = sl_false;
		nat_raw_socket = sl_false;
		nat_port_begin = 30000;
		nat_port_end = 60000;
		nat_mtu_incoming = 1500;
		nat_mtu_outgoing = 1480;
		nat_expiring_duration = 500000;
	}
};

class STunnelServiceSession;
class SLIB_EXPORT STunnelServer : public Object, public IAsyncSecureStreamListener, public INetCaptureListener
{
protected:
	STunnelServer();
public:
	~STunnelServer();

public:
	static Ref<STunnelServer> create(const STunnelServerParam& param);

public:
	void release();

protected:
	sl_bool initialize(const STunnelServerParam& param);

	void onConnectedSecureStream(AsyncSecureStream* securedStream, sl_bool flagError);
	void onCapturePacket(NetCapture* capture, NetCapturePacket* packet);

	void runNAT();
	void writeIPToNAT(STunnelServiceSession* session, const void* ip, sl_uint32 size);

protected:
	STunnelServerParam m_param;

	WeakRef<AsyncLoop> m_asyncLoop;

	Ref<AsyncTcpSecureStreamServer> m_server;
	Map< STunnelServiceSession*, Ref<STunnelServiceSession> > m_sessions;

	Ref<NetCapture> m_captureNat;
	NAT_Table m_tableNat;
	Map< IPv4Address, WeakRef<STunnelServiceSession> > m_mapSessionLocalIP;
	IPv4Fragmentation m_fragmentationIncoming;
	IPv4Fragmentation m_fragmentationOutgoing;
	Ref<Thread> m_threadNat;
	MacAddress m_addressNatDev;
	MacAddress m_addressNatGateway;

	friend class STunnelServiceSession;
};

class SLIB_EXPORT STunnelServiceSession : public Object, public IAsyncTcpSocketListener, public IAsyncUdpSocketListener
{
public:
	STunnelServiceSession();

public:
	void close();
	void read();
	void write();

	void sendSessionMessage(sl_uint8 method, MemoryBuffer& buf, sl_bool flagDatagram);
	void sendSessionMessage(sl_uint8 method, const void* buf, sl_uint32 n, sl_bool flagDatagram);
	void receiveSessionMessage(const void* data, sl_uint32 size);

	void sendRawIP(const void* ip, sl_uint32 size);
	void receiveRawIP(const void* data, sl_uint32 size);

	void receiveOpenPort_TCP(const void* data, sl_uint32 size);
	void sendClosePort_TCP(sl_uint64 portId);
	void receiveClosePort_TCP(const void* data, sl_uint32 size);
	void sendData_TCP(sl_uint64 portId, const void* data, sl_uint32 size);
	void receiveData_TCP(const void* data, sl_uint32 size);
	void sendDataReply_TCP(sl_uint64 portId, sl_uint32 sizeReceived);
	void receiveDataReply_TCP(const void* data, sl_uint32 size);

	void sendData_UDP(sl_uint64 portId, const SocketAddress& addressFrom, const void* data, sl_uint32 size);
	void receiveData_UDP(const void* data, sl_uint32 size);

	void receiveDNSRequest(const void* data, sl_uint32 size);
	void sendDNSResponse(const String& dns, const IPv4Address& ip);

	Ref<NetCapture> getNatDevice();

protected:
	virtual void onConnect(AsyncTcpSocket* socket, const SocketAddress& address, sl_bool flagError);
	virtual void onRead(AsyncStream* stream, void* data, sl_uint32 sizeRead, Referable* ref, sl_bool flagError);
	virtual void onWrite(AsyncStream* stream, void* data, sl_uint32 sizeWritten, Referable* ref, sl_bool flagError);
	virtual void onReceiveFrom(AsyncUdpSocket* socket, void* data, sl_uint32 sizeReceive, const SocketAddress& address, sl_bool flagError);
	virtual void onSendTo(AsyncUdpSocket* socket, void* data, sl_uint32 sizeSent, const SocketAddress& address, sl_bool flagError);

public:
	Ref<AsyncSecureStream> m_stream;
	WeakRef<STunnelServer> m_server;

	DatagramStream m_datagram;
	Memory m_bufRead;

	sl_bool m_flagReading;
	sl_bool m_flagWriting;

	sl_size m_limitQueueDatagram;
	QueueChannelArray<Memory> m_queueOutputChannels;

	class TCP_Port : public Referable
	{
	public:
		sl_uint64 id;
		Ref<AsyncTcpSocket> socket;
		sl_bool flagConnected;
		Queue<Memory> queueSend;
		Memory memReceive;
		sl_uint32 sizeClientPendingData;
		sl_uint32 sizeServerPendingData;
		sl_bool flagReading;
		Mutex lock;
	
		TCP_Port()
		{
			flagConnected = sl_false;
			sizeClientPendingData = 0;
			sizeServerPendingData = 0;
			flagReading = sl_false;
		}
	};
	Map< sl_uint64, Ref<TCP_Port> > m_portsTCP;

	class UDP_Port : public Referable
	{
	public:
		sl_uint64 id;
		Ref<AsyncUdpSocket> socket;
		Memory memReceive;
	};
	ExpiringMap< sl_uint64, Ref<UDP_Port> > m_portsUDP;

protected:
	Ref<AsyncLoop> getAsyncLoop();

	void _openPort_TCP(sl_uint64 portId, String hostAddress);
	void _closePort_TCP(sl_uint64 portId);
	void _receivePort_TCP(TCP_Port* port);
	void _onReadPort_TCP(TCP_Port* port, void* buf, sl_uint32 size, sl_bool flagError);
	void _onWritePort_TCP(TCP_Port* port, void* buf, sl_uint32 size, sl_bool flagError);

	Ref<UDP_Port> _getPort_UDP(sl_uint64 portId);
	void _receivePort_UDP(UDP_Port* port);

	void _resolveDNS(String dns);
};
SLIB_SNET_NAMESPACE_END

#endif

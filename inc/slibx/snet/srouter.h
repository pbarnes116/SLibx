#ifndef CHECKHEADER_SLIB_SNET_SROUTER
#define CHECKHEADER_SLIB_SNET_SROUTER

#include "definition.h"

#include <slib/network/socket_address.h>
#include <slib/network/capture.h>
#include <slib/network/nat.h>
#include <slib/network/async.h>
#include <slib/network/datagram.h>
#include <slib/network/ethernet.h>

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/queue.h>
#include <slib/core/thread.h>

#include "slib/crypto/aes.h"

SLIB_SNET_NAMESPACE_BEGIN

class SRouter;

class SRouterInterface;

class SLIB_EXPORT SRouterInterfaceParam
{
public:
	sl_uint32 fragment_expiring_seconds;
	sl_uint32 mtu_outgoing;

	sl_bool use_nat;
	IPv4Address nat_ip;
	sl_uint16 nat_port_begin;
	sl_uint16 nat_port_end;

	Ref<Dispatcher> dispatcher;

public:
	SRouterInterfaceParam();
	
public:
	void parseConfig(const Variant& varConfig);
	
};

class SLIB_EXPORT SRouterInterface : public Object
{
	SLIB_DECLARE_OBJECT
	
public:
	SRouterInterface();

public:
	Ref<SRouter> getRouter();
	
	void setRouter(const Ref<SRouter>& router);
	
	
	void initWithParam(const SRouterInterfaceParam& param);
	
	void setNatIp(const IPv4Address& ip);
	
	void writeIPv4Packet(const void* packet, sl_uint32 size);

protected:
	virtual void _writeIPv4Packet(const void* packet, sl_uint32 size) = 0;

protected:
	AtomicWeakRef<SRouter> m_router;

	sl_uint32 m_mtuOutgoing;

	sl_bool m_flagUseNat;
	NatTable m_nat;
	NatTableParam m_natParam;
	sl_bool m_flagNatDynamicTarget;

	IPv4Fragmentation m_fragmentation;

	friend class SRouter;
};

class SLIB_EXPORT SRouterDeviceParam : public SRouterInterfaceParam
{
public:
	Ref<NetCapture> device;

	String iface_name;
	sl_bool use_pcap;
	sl_bool use_raw_socket;
	sl_bool is_ethernet;
	IPv4Address subnet_broadcast;
	MacAddress gateway_mac;

public:
	SRouterDeviceParam();
	
public:
	void parseConfig(const Variant& varConfig);
};

class SLIB_EXPORT SRouterDevice : public SRouterInterface, public INetCaptureListener
{
	SLIB_DECLARE_OBJECT
	
protected:
	SRouterDevice();
	
	~SRouterDevice();

public:
	static Ref<SRouterDevice> create(const SRouterDeviceParam& param);

	void release();

	void start();

	void processReadL2Frame(void* frame, sl_uint32 size);

	void addMacAddress(const IPv4Address& ipAddress, const MacAddress& macAddress);

	void writeL2Frame(const void* packet, sl_uint32 size);

	String getStatus();

protected:
	// override
	void _writeIPv4Packet(const void* packet, sl_uint32 size);
	
	// override
	void onCapturePacket(NetCapture* capture, NetCapturePacket* packet);

	void _idle();

protected:
	String m_deviceName;
	IPv4Address m_ipAddressDevice;
	IPv4Address m_subnetBroadcast;
	Ref<NetCapture> m_device;
	
	MacAddress m_macAddressDevice;
	MacAddress m_macAddressGateway;
	
	EthernetMacTable m_tableMac;

	friend class SRouter;
};

class SLIB_EXPORT SRouterRemoteParam : public SRouterInterfaceParam
{
public:
	sl_bool flagTcp;
	SocketAddress host_address;
	String key;
	sl_bool flagCompressPacket;
	sl_uint32 tcp_send_buffer_size;

public:
	SRouterRemoteParam();

public:
	void parseConfig(const Variant& varConfig);
};

class SLIB_EXPORT SRouterRemote : public SRouterInterface
{
	SLIB_DECLARE_OBJECT
	
protected:
	SRouterRemote();

public:
	static Ref<SRouterRemote> create(const SRouterRemoteParam& param);

public:
	String getStatus();

	void reconnect();

protected:
	// override
	void _writeIPv4Packet(const void* packet, sl_uint32 size);

	void _idle();

protected:
	sl_bool m_flagTcp;
	AtomicRef<TcpDatagramClient> m_tcp;
	SocketAddress m_address;
	String m_key;
	
	Time m_timeLastKeepAliveReceive;
	Time m_timeLastKeepAliveSend;
	sl_bool m_flagDynamicConnection;
	sl_bool m_flagCompressPacket;
	sl_uint32 m_tcpSendBufferSize;

	AES m_aes;

	friend class SRouter;
};

class SLIB_EXPORT SRouterRoute
{
public:
	sl_bool flagCheckProtocol;
	sl_bool flagTcp;
	sl_bool flagUdp;
	sl_bool flagIcmp;

	sl_bool flagCheckDstIp;
	IPv4Address dst_ip_begin;
	sl_uint32 dst_port_begin;

	sl_bool flagCheckDstPort;
	IPv4Address dst_ip_end;
	sl_uint32 dst_port_end;

	sl_bool flagCheckSrcIp;
	IPv4Address src_ip_begin;
	sl_uint32 src_port_begin;

	sl_bool flagCheckSrcPort;
	IPv4Address src_ip_end;
	sl_uint32 src_port_end;
	
	Ref<SRouterInterface>* targets;
	sl_uint32 countTargets;
	Array< Ref<SRouterInterface> > arrTargets;

	sl_bool flagBreak;

public:
	SRouterRoute();
	
public:
	sl_bool parseConfig(SRouter* router, const Variant& conf);

};


class SLIB_EXPORT SRouterArpProxy
{
public:
	IPv4Address ip_begin;
	IPv4Address ip_end;
	Ref<SRouterDevice> device;

public:
	SRouterArpProxy();

public:
	sl_bool parseConfig(SRouter* router, const Variant& conf);

};


class SLIB_EXPORT SRouterListener
{
public:
	// returns true when the packet is consumed and doesn't need more processing
	virtual sl_bool onForwardIPv4Packet(SRouter* router, SRouterInterface* device, void* packet, sl_uint32 size) = 0;

	// returns true when the packet is consumed and doesn't need more processing
	virtual sl_bool onForwardEthernetFrame(SRouter* router, SRouterDevice* device, void* frame, sl_uint32 size) = 0;

};

class SLIB_EXPORT SRouterParam
{
public:
	String name;

	sl_uint32 udp_server_port;
	sl_uint32 tcp_server_port;
	String server_key;

	sl_uint32 tcp_send_buffer_size;
	
	Ptr<SRouterListener> listener;

public:
	SRouterParam();

};

class SLIB_EXPORT SRouter : public Object, public IAsyncUdpSocketListener, public ITcpDatagramListener
{
protected:
	SRouter();
	
	~SRouter();

public:
	static Ref<SRouter> create(const SRouterParam& param);
	
	static Ref<SRouter> createFromConfiguration(const Variant& varConfig);

public:
	void release();

	void start();

	
	Ref<SRouterInterface> getInterface(const String& name);
	
	void registerInterface(const String& name, const Ref<SRouterInterface>& iface);
	
	
	Ref<SRouterDevice> getDevice(const String& name);
	
	void registerDevice(const String& name, const Ref<SRouterDevice>& dev);

	
	Ref<SRouterRemote> getRemote(const String& name);
	
	void registerRemote(const String& name, const Ref<SRouterRemote>& remote);

	
	void addRoute(const SRouterRoute& route);


	void forwardIPv4Packet(SRouterInterface* deviceSource, void* packet, sl_uint32 size, sl_bool flagCheckedHeader = sl_false);

	sl_bool forwardEthernetFrame(SRouterDevice* deviceSource, void* frame, sl_uint32 size);


	String getStatusReport();
	
protected:
	void _sendRemoteMessage(SRouterRemote* remote, sl_uint8 method, const void* data, sl_uint32 n);
	
	void _receiveRemoteMessage(const SocketAddress& address, TcpDatagramClient* client, void* data, sl_uint32 size);

	
	void _sendRawIPv4PacketToRemote(SRouterRemote* remote, const void* packet, sl_uint32 size);
	
	void _receiveRawIPv4PacketFromRemote(SRouterRemote* remote, void* data, sl_uint32 size);


	void _sendCompressedRawIPv4PacketToRemote(SRouterRemote* remote, const void* packet, sl_uint32 size);

	void _receiveCompressedRawIPv4PacketFromRemote(SRouterRemote* remote, void* data, sl_uint32 size);


	void _sendRouterKeepAlive(SRouterRemote* remote);
	
	void _receiveRouterKeepAlive(const SocketAddress& address, TcpDatagramClient* client, void* data, sl_uint32 size);

	void _onIdle();

protected:
	// override
	void onReceiveFrom(AsyncUdpSocket* socket, const SocketAddress& address, void* data, sl_uint32 sizeReceived);

	// override
	void onReceiveFrom(TcpDatagramClient* client, void* data, sl_uint32 sizeReceived);

protected:
	String m_name;
	sl_bool m_flagInit;
	sl_bool m_flagRunning;

	Ref<Dispatcher> m_dispatcher;
	Ref<AsyncIoLoop> m_ioLoop;

	Ref<AsyncUdpSocket> m_udpServer;
	Ref<TcpDatagramServer> m_tcpServer;
	AES m_aesPacket;

	Ref<Timer> m_timerIdle;

	HashMap< String, Ref<SRouterInterface> > m_mapInterfaces;
	HashMap< String, Ref<SRouterDevice> > m_mapDevices;
	HashMap< String, Ref<SRouterRemote> > m_mapRemotes;
	HashMap< SocketAddress, Ref<SRouterRemote> > m_mapRemotesBySocketAddress;
	HashMap< TcpDatagramClient*, Ref<SRouterRemote> > m_mapRemotesByTcpClient;
	CList<SRouterRoute> m_listRoutes;
	CList<SRouterArpProxy> m_listArpProxies;

	Ptr<SRouterListener> m_listener;

	friend class SRouterRemote;
};

SLIB_SNET_NAMESPACE_END

#endif

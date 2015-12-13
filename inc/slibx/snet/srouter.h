#ifndef CHECKHEADER_SLIB_SNET_SROUTER
#define CHECKHEADER_SLIB_SNET_SROUTER

#include "definition.h"

#include <slib/network/socket_address.h>
#include <slib/network/capture.h>
#include <slib/network/nat.h>
#include <slib/network/socket.h>
#include <slib/network/ethernet.h>

#include <slib/core/object.h>
#include <slib/core/map.h>
#include <slib/core/queue.h>
#include <slib/core/thread.h>

#include "slib/crypto/aes.h"

SLIB_SNET_NAMESPACE_BEGIN

class SRouter;

class SRouterInterface;
class SLIB_EXPORT ISRouterInterfaceListener
{
public:
	// return sl_false when the packet is not allowed
	virtual sl_bool onWritePacket(SRouterInterface* iface, void* ip, sl_uint32 size)
	{
		return sl_true;
	}
};

class SLIB_EXPORT SRouterInterfaceParam
{
public:
	sl_uint32 fragment_expiring_seconds;
	sl_uint32 mtu_outgoing;

	sl_bool use_nat;
	IPv4Address nat_ip;
	sl_uint32 nat_port_begin;
	sl_uint32 nat_port_end;

	Ptr<ISRouterInterfaceListener> listener;

public:
	SRouterInterfaceParam()
	{
		fragment_expiring_seconds = 3600;
		mtu_outgoing = 1450;
		
		use_nat = sl_false;
		nat_ip.setZero();
		nat_port_begin = 30000;
		nat_port_end = 60000;
	}

	void parseConfig(const Variant& varConfig);
};

class SLIB_EXPORT SRouterInterface : public Object
{
public:
	SRouterInterface();
	~SRouterInterface();

public:
	Ref<SRouter> getRouter();
	void setRouter(const Ref<SRouter>& router);
	
	void initWithParam(const SRouterInterfaceParam& param);

	void setNAT_IP(const IPv4Address& ip);

	void writePacket(const void* packet, sl_uint32 size);
	void forwardPacket(const void* packet, sl_uint32 size);

	SLIB_PROPERTY_INLINE(Ptr<ISRouterInterfaceListener>, Listener);

protected:
	virtual void _writePacket(const void* packet, sl_uint32 size) = 0;

protected:
	WeakRef<SRouter> m_router;

	sl_uint32 m_mtuOutgoing;

	sl_bool m_flagUseNat;
	NAT_Table m_tableNat;
	
	IPv4Fragmentation m_fragmentation;

};

class SLIB_EXPORT SRouterDeviceParam : public SRouterInterfaceParam
{
public:
	Ref<NetCapture> device;

	String iface_name;
	sl_bool use_raw_socket;
	sl_bool is_ethernet;
	MacAddress gateway_mac;

public:
	SRouterDeviceParam()
	{
		use_raw_socket = sl_false;
		is_ethernet = sl_true;
		gateway_mac.setZero();
	}

	void parseConfig(const Variant& varConfig);
};

class SLIB_EXPORT SRouterDevice : public SRouterInterface, public INetCaptureListener
{
protected:
	SRouterDevice();
public:
	~SRouterDevice();

public:
	static Ref<SRouterDevice> create(const SRouterDeviceParam& param);

	void processReadFrame(const void* frame, sl_uint32 size);

	void addMacAddress(const IPv4Address& ipAddress, const MacAddress& macAddress);

	String getStatus();



protected:
	virtual void _writePacket(const void* packet, sl_uint32 size);
	virtual void onCapturePacket(NetCapture* capture, NetCapturePacket* packet);

	void _idle();

protected:
	String m_deviceName;
	IPv4Address m_ipAddressDevice;
	Ref<NetCapture> m_device;
	MacAddress m_macAddressDevice;
	MacAddress m_macAddressGateway;
	EthernetMacTable m_tableMac;

	friend class SRouter;
};

class SLIB_EXPORT SRouterRemoteParam : public SRouterInterfaceParam
{
public:
	SocketAddress host_address;
	String key;

public:
	SRouterRemoteParam()
	{
	}

	void parseConfig(const Variant& varConfig);
};

class SLIB_EXPORT SRouterRemote : public SRouterInterface
{
protected:
	SRouterRemote();
public:
	~SRouterRemote();

public:
	static Ref<SRouterRemote> create(const SRouterRemoteParam& param);

public:
	String getStatus();

protected:
	virtual void _writePacket(const void* packet, sl_uint32 size);

	void _idle();

protected:
	SocketAddress m_address;
	String m_key;
	Time m_timeLastKeepAliveReceive;
	Time m_timeLastKeepAliveSend;
	sl_bool m_flagDynamicAddress;

	AES m_aes;

	friend class SRouter;
};

class SLIB_EXPORT SRouterRoute
{
public:
	IPv4Address dst_ip_begin;
	IPv4Address dst_ip_end;
	IPv4Address src_ip_begin;
	IPv4Address src_ip_end;
	
	String interfaceName;
	Ref<SRouterInterface> interfaceDevice;

	sl_bool flagBreak;

public:
	SRouterRoute();
	SRouterRoute(const SRouterRoute& other);

	SRouterRoute& operator=(const SRouterRoute& other);

	sl_bool operator==(const SRouterRoute& other) const;
};

class SLIB_EXPORT SRouterParam
{
public:
	String name;
	sl_uint32 port;
	String key;

public:
	SRouterParam()
	{
		port = 0;
	}
};

class SLIB_EXPORT SRouter : public Object
{
protected:
	SRouter();
public:
	~SRouter();

public:
	static Ref<SRouter> create(const SRouterParam& param);
	static Ref<SRouter> createFromConfiguration(const Variant& varConfig);

	void release();

	Ref<SRouterInterface> getInterface(const String& name);
	void registerInterface(const String& name, const Ref<SRouterInterface>& iface);
	
	Ref<SRouterDevice> getDevice(const String& name);
	void registerDevice(const String& name, const Ref<SRouterDevice>& dev);

	Ref<SRouterRemote> getRemote(const String& name);
	void registerRemote(const String& name, const Ref<SRouterRemote>& remote);

	void addRoute(const SRouterRoute& route, sl_bool flagReplace = sl_true);

	void forwardPacket(const void* ip, sl_uint32 size);

	String getStatusReport();
	
protected:
	void _sendRemoteMessage(SRouterRemote* remote, sl_uint8 method, const void* data, sl_uint32 n);
	void _receiveRemoteMessage(SocketAddress& address, const void* data, sl_uint32 size);

	void _sendRawPacketToRemote(SRouterRemote* remote, const void* ip, sl_uint32 size);
	void _receiveRawPacketFromRemote(SocketAddress& address, const void* data, sl_uint32 size);

	void _sendRouterKeepAlive(SRouterRemote* remote);
	void _receiveRouterKeepAlive(SocketAddress& address, const void* data, sl_uint32 size);

	void _runUdp();

protected:
	String m_name;

	Ref<Socket> m_udp;
	Ref<Thread> m_threadUdp;
	sl_bool m_flagClosed;

	AES m_aes;

	Map< String, Ref<SRouterInterface> > m_mapInterfaces;
	Map< String, Ref<SRouterDevice> > m_mapDevices;
	Map< String, Ref<SRouterRemote> > m_mapRemotes;
	List<SRouterRoute> m_listRoutes;

	friend class SRouterRemote;
};
SLIB_SNET_NAMESPACE_END

#endif

#ifndef CHECKHEADER_SLIB_P2P_SWITCH
#define CHECKHEADER_SLIB_P2P_SWITCH

#include "definition.h"

/*
	P2P Switch

Packet Format:

All packets are leaded by SHA-256 Hash

	Hash (hash of Content-Region) = 4 bytes
	Nonce, MethodId = 4 bytes (28 bits, 4 bits)
	Content-Region (xor with nonce-32bits)

* Using Little Endian

- Ping
Request: (MethodId = 8)
	SourceHostId = String-64
	TargetHostId = String-64
	SourceTime = 8 bytes

Response: (MethodId = 9)
	SourceHostId = String-64
	TargetHostId = String-64
	SourceTime = 8 bytes
	TargetTime = 8 bytes
	SessionId = 8 bytes

- Message (MethodId = 10)
	SessionId = 8 bytes
	SourceHostId = String-64
	TargetHostId = String-64
	MessageId = 8 bytes
	Content


Hns Relaying Message Format:

MessageId = 0 => Special Command Messages
MessageId != 0 => Relaying Message

[Header]
MethodId = 1 byte

- Query Addresses Request (MethodId = 1)

- Query Addresses Response (MethodId = 2)
	Count = Number (Max: 16)
	String1
	String2
	...
	
*/

#include "hns_client.h"

namespace slib
{

	class P2PSwitch;
	class P2PSwitchHost;

	class P2PSwitchServiceInfo
	{
	public:
		sl_bool flagUdp;
		String serviceId;
		String serviceHost;
		sl_uint32 servicePort;
		RSAPublicKey servicePublicKey;
		String serviceSecret;

	public:
		SLIB_INLINE P2PSwitchServiceInfo()
		{
			flagUdp = sl_true;
			servicePort = 0;
			serviceSecret = "HNS_SERVICE";
		}
	};
		
	class P2PSwitchSession : public Referable
	{
	public:
		SLIB_INLINE P2PSwitchSession()
		{
			m_timeLastCommunicated.setZero();
		}

		enum SessionType
		{
			typeDirectUdp = 1
			, typeDirectTcp = 2
			, typeTurningUdp = 3
			, typeTurningTcp = 4
		};
		SLIB_INLINE SessionType getType() const
		{
			return m_type;
		}
		SLIB_INLINE sl_bool isDirectSession() const
		{
			return m_type == typeDirectUdp || m_type == typeDirectTcp;
		}
		SLIB_INLINE sl_bool isTurningSession() const
		{
			return m_type == typeTurningUdp || m_type == typeTurningTcp;
		}
		SLIB_INLINE sl_bool isUdpSession() const
		{
			return m_type == typeDirectUdp || m_type == typeTurningUdp;
		}
		SLIB_INLINE sl_bool isTcpSession() const
		{
			return m_type == typeDirectTcp || m_type == typeTurningTcp;
		}

		SLIB_INLINE Ref<P2PSwitchHost> getHost() const
		{
			return m_host.lock();
		}
		String getHostId();

		SLIB_INLINE Time getLastCommunicatedTime() const
		{
			return m_timeLastCommunicated;
		}

	protected:
		WeakRef<P2PSwitchHost> m_host;
		SessionType m_type;
		Time m_timeLastCommunicated;
	};

	class P2PSwitchDirectUdpSession : public P2PSwitchSession
	{
	public:
		SLIB_INLINE P2PSwitchDirectUdpSession()
		{
			m_type = typeDirectUdp;
		}
	};

	class P2PSwitchTurningUdpSession : public P2PSwitchSession
	{
	public:
		SLIB_INLINE P2PSwitchTurningUdpSession()
		{
			m_type = typeTurningUdp;
		}
	};

	class P2PSwitchHost : public Referable
	{
	public:
		SLIB_INLINE String getHostId() const
		{
			return m_hostId;
		}

		SLIB_INLINE String getHostCertificate() const
		{
			return m_hostCertificate;
		}

		virtual List< Ref<P2PSwitchDirectUdpSession> > getDirectUdpSessions() const = 0;
		virtual List< Ref<P2PSwitchTurningUdpSession> > getTurningUdpSessions() const = 0;

		SLIB_INLINE Time getLastCommunicatedTime() const
		{
			return m_timeLastCommunicated;
		}

		SLIB_INLINE Time getLastCommunicatedTimeForDirectUdp() const
		{
			return m_timeLastCommunicatedDirectUdp;
		}

		SLIB_INLINE Time getLastCommunicatedTimeForTurningUdp() const
		{
			return m_timeLastCommunicatedTurningUdp;
		}

	protected:
		String m_hostId;
		String m_hostCertificate;
		Time m_timeLastCommunicated;
		Time m_timeLastCommunicatedDirectUdp;
		Time m_timeLastCommunicatedTurningUdp;

	public:
		SLIB_INLINE P2PSwitchHost()
		{
		}
	};

	class IP2PSwitchListener
	{
	public:
		virtual void onServiceConnected(P2PSwitch* p2p, HnsClient* connection) {}

		virtual void onReceiveBroadcast(P2PSwitch* p2p, String senderId, sl_uint64 messageId, const void* data, sl_uint32 size) {}

		virtual void onReceiveMessage(P2PSwitch* p2p, P2PSwitchSession* session, sl_uint64 messageId, const void* data, sl_uint32 size) {}

	};

	class P2PSwitchParam
	{
	public:
		String hostId;
		String hostCertificate;
		String hostCertificateForServices;
		sl_uint32 hostPortUdp;
		sl_uint32 hostPortTcp;

		Ptr<IP2PSwitchListener> listener;

		sl_uint32 connectionTimeoutMilliseconds;
		sl_uint32 keepAliveIntervalMilliseconds;

		SLIB_INLINE P2PSwitchParam()
		{
			hostPortUdp = 0;
			hostPortTcp = 0;

			connectionTimeoutMilliseconds = 20000;
			keepAliveIntervalMilliseconds = 5000;
		}
	};

	class P2PSwitch : public Object
	{
	protected:
		SLIB_INLINE P2PSwitch()
		{
		}

	public:
		virtual void release() = 0;

		virtual sl_bool isRunning() = 0;
		
		virtual sl_bool addService(const P2PSwitchServiceInfo& service) = 0;
		virtual sl_bool removeService(String serviceId) = 0;
		virtual List< Ref<HnsClient> > getServiceConnections(String serviceId) = 0;
		SLIB_INLINE sl_bool checkServiceConnection(String serviceId)
		{
			ListLocker< Ref<HnsClient> > services(getServiceConnections(serviceId));
			for (sl_size i = 0; i < services.count; i++) {
				Ref<HnsClient>& service = services[i];
				if (service.isNotNull()) {
					if (service->isConnected()) {
						return sl_true;
					}
				}
			}
			return sl_false;
		}

		virtual Ref<P2PSwitchHost> addHost(String hostId) = 0;
		virtual sl_bool removeHost(String hostId) = 0;
		virtual Ref<P2PSwitchHost> getHost(String hostId) = 0;

		virtual void sendBroadcastMessage(sl_uint64 messageId, const void* data, sl_uint32 n) = 0;
		enum MessageMode
		{
			messageModeDirectAndTurn = 0
			, messageModeDirectOnly = 1
			, messageModeTurnOnly = 2
			, messageModeAlwaysSend = 3
		};
		virtual sl_bool sendMessage(String targetHostId, sl_uint64 messageId, const void* data, sl_uint32 n, MessageMode mode = messageModeAlwaysSend) = 0;
		virtual void sendMessage(Ref<P2PSwitchSession> session, sl_uint64 messageId, const void* data, sl_uint32 n) = 0;

	public:
		SLIB_PROPERTY(Ptr<IP2PSwitchListener>, Listener);

		virtual sl_uint32 getConnectionTimeoutMilliseconds() = 0;
		virtual void setConnectionTimeoutMilliseconds(sl_uint32 millis) = 0;
		virtual sl_uint32 getKeepAliveIntervalMilliseconds() = 0;
		virtual void setKeepAliveIntervalMilliseconds(sl_uint32 millis) = 0;

	public:
		static Ref<P2PSwitch> start(const P2PSwitchParam& param);
		
	};

}

#endif


#ifndef CHECKHEADER_SLIB_SNET_DATAGRAM
#define CHECKHEADER_SLIB_SNET_DATAGRAM

#include "definition.h"

#include <slib/core/io.h>
#include <slib/network/async.h>

namespace slib
{

	
	/*
			DatagramSerializer
	 
	 Datagram1-Length (4 bytes, LE)
	 Datagram1-Content
	 Datagram2-Length (4 bytes, LE)
	 Datagram2-Content
	 ...
	 */
	class SLIB_EXPORT DatagramSerializer : public Object
	{
	public:
		DatagramSerializer();
		
		~DatagramSerializer();
		
	public:
		void clear();
		
		sl_bool parse(const void* data, sl_size size, LinkedQueue<Memory>& datagrams);
		
		Memory build(const void* datagram, sl_uint32 size);
		
		Memory build(MemoryBuffer& buf);
		
	public:
		SLIB_PROPERTY(sl_uint32, MaxDatagramSize)
		
	protected:
		MemoryBuffer m_buf;
		sl_uint32 m_lenCurrentDatagram;
		sl_uint32 m_posCurrentDatagram;
		sl_uint8 m_bufSize[4];
		sl_uint32 m_lenBufSize;
		
	};
	
	
	class TcpDatagramClient;
	class TcpDatagramServer;
	
	class SLIB_EXPORT ITcpDatagramListener
	{
	public:
		ITcpDatagramListener();
		
		virtual ~ITcpDatagramListener();
		
	public:
		virtual void onReceiveFrom(TcpDatagramClient* client, void* data, sl_uint32 sizeReceived) = 0;
		
		virtual void onConnect(TcpDatagramClient* client);
		
		virtual void onError(TcpDatagramClient* client);
		
	};
	
	class SLIB_EXPORT TcpDatagramParam
	{
	public:
		SocketAddress bindAddress;
		sl_uint32 maxWaitingBytesForSending; // default: 1024000
		Ref<AsyncIoLoop> ioLoop;
		
		Ptr<ITcpDatagramListener> listener;
		Function<void(TcpDatagramClient*, void*, sl_uint32)> onReceiveFrom;
		Function<void(TcpDatagramClient*)> onConnect;
		Function<void(TcpDatagramClient*)> onError;
		
	public:
		TcpDatagramParam();
		
		~TcpDatagramParam();
		
	};
	
	class SLIB_EXPORT TcpDatagramClientParam : public TcpDatagramParam
	{
	public:
		SocketAddress serverAddress;
		
		sl_bool flagAutoConnect; // default: true
		sl_bool flagAutoReconnect; // default: false
		sl_uint32 autoReconnectIntervalSeconds; // default: 5
		
	public:
		TcpDatagramClientParam();
		
		~TcpDatagramClientParam();
		
	};
	
	class SLIB_EXPORT TcpDatagramClient : public Object, public IAsyncTcpSocketListener
	{
		SLIB_DECLARE_OBJECT
		
	public:
		TcpDatagramClient();
		
		~TcpDatagramClient();
		
	public:
		static Ref<TcpDatagramClient> create(const TcpDatagramClientParam& param);
		
		void close();
		
		void connect();
		
		sl_bool isOpened();
		
		sl_bool isConnecting();
		
		sl_bool isConnected();
		
		Ref<TcpDatagramServer> getServer();
		
		sl_bool send(const void* data, sl_uint32 size);
		
		sl_bool send(const Memory& mem);
		
	protected:
		// override
		void onConnect(AsyncTcpSocket* socket, const SocketAddress& address, sl_bool flagError);
		
		void onReceiveStream(AsyncStreamResult* result);
		
		void onSendStream(AsyncStreamResult* result);
		
		void onMessageError(AsyncTcpSocket* socket);
		
	protected:
		static Ref<TcpDatagramClient> _createForServer(TcpDatagramServer* server, const Ref<Socket>& socket);
		
		void _reconnect();
		
		void _close();
		
	protected:
		sl_bool m_flagOpened;
		Ref<AsyncTcpSocket> m_socketConnect;
		Ref<AsyncTcpSocket> m_socketMessage;
		
		Memory m_bufReceive;
		DatagramSerializer m_datagram;
		sl_uint32 m_maxWaitingBytesForSending;
		
		WeakRef<TcpDatagramServer> m_server;
		Ref<AsyncIoLoop> m_ioLoop;
		SocketAddress m_addressBind;
		SocketAddress m_addressServer;
		sl_bool m_flagAutoReconnect;
		sl_uint32 m_autoReconnectIntervalSeconds;
		
		Ptr<ITcpDatagramListener> m_listener;
		Function<void(TcpDatagramClient*, void*, sl_uint32)> m_onReceiveFrom;
		Function<void(TcpDatagramClient*)> m_onConnect;
		Function<void(TcpDatagramClient*)> m_onError;
		
		friend class TcpDatagramServer;
		
	};
	
	class SLIB_EXPORT TcpDatagramServerParam : public TcpDatagramParam
	{
	public:
		sl_bool flagAutoStart; // default: true
		
	public:
		TcpDatagramServerParam();
		
		~TcpDatagramServerParam();
		
	};
	
	class SLIB_EXPORT TcpDatagramServer : public Object, public IAsyncTcpServerListener
	{
		SLIB_DECLARE_OBJECT
		
	protected:
		TcpDatagramServer();
		
		~TcpDatagramServer();
		
	public:
		static Ref<TcpDatagramServer> create(const TcpDatagramServerParam& param);
		
		void close();
		
		void start();
		
		sl_bool isRunning();
		
		Ptr<ITcpDatagramListener> getListener();
		
		Ref<AsyncIoLoop> getIoLoop();
		
	protected:
		// override
		void onAccept(AsyncTcpServer* socketListen, const Ref<Socket>& socketAccept, const SocketAddress& address);
		
	protected:
		sl_bool _initialize(const TcpDatagramServerParam& param);
		
		void _removeClient(TcpDatagramClient* client);
		
	protected:
		Ref<AsyncTcpServer> m_server;
		HashMap< TcpDatagramClient*, Ref<TcpDatagramClient> > m_clients;
		Ref<AsyncIoLoop> m_ioLoop;
		sl_uint32 m_maxWaitingBytesForSending;
		
		Ptr<ITcpDatagramListener> m_listener;
		Function<void(TcpDatagramClient*, void*, sl_uint32)> m_onReceiveFrom;
		Function<void(TcpDatagramClient*)> m_onConnect;
		Function<void(TcpDatagramClient*)> m_onError;
		
		friend class TcpDatagramClient;
		
	};
	
	
}

#endif

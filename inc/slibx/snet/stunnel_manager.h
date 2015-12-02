#ifndef CHECKHEADER_SLIB_SNET_STUNNEL_MANAGER
#define CHECKHEADER_SLIB_SNET_STUNNEL_MANAGER

#include "definition.h"

#include "stunnel_client.h"

SLIB_SNET_NAMESPACE_BEGIN
class SLIB_EXPORT STunnelManager : public Object, public ISTunnelClientListener
{
public:
	STunnelManager();
	~STunnelManager();

public:
	void release();
	sl_bool init();
	sl_bool initWithConfiguration(const Variant& conf);

	Ref<STunnelClient> getTunnel(const String& name);
	Ref<STunnelClient> addTunnel(const String& name, const STunnelClientParam& param);

	String getStatusReport();

	SLIB_INLINE Ref<AsyncLoop> getAsyncLoop()
	{
		return m_asyncLoop;
	}

public:
	SLIB_PROPERTY_INLINE(sl_uint32, AutoReconnectTimeout);
	SLIB_PROPERTY_INLINE(Ptr<ISTunnelClientListener>, TunnelListener);

private:
	sl_bool _init();
	void _run();

protected:
	virtual void onSTunnelRawIP(STunnelClient* client, const void* ip, sl_uint32 len);
	virtual void onSTunnelDataUDP(STunnelClient* client, sl_uint64 portId, const SocketAddress& addressFrom, const void* data, sl_uint32 n);
	virtual void onSTunnelDNSResponse(STunnelClient* client, const String& dns, const IPv4Address& ip);
private:
	sl_bool m_flagInited;
	Map< String, Ref<STunnelClient> > m_tunnels;
	Ref<Thread> m_thread;
	Ref<AsyncLoop> m_asyncLoop;
};
SLIB_SNET_NAMESPACE_END

#endif

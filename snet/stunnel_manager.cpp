#include "../../../inc/slibx/snet/stunnel_manager.h"
#include "../../../inc/slib/core/log.h"

SLIB_SNET_NAMESPACE_BEGIN
STunnelManager::STunnelManager()
{
	m_flagInited = sl_false;

	setAutoReconnectTimeout(30000);
}

STunnelManager::~STunnelManager()
{
	release();
}

sl_bool STunnelManager::_init()
{
	MutexLocker lock(getLocker());
	release();
	m_asyncLoop = AsyncLoop::create();
	if (m_asyncLoop.isNull()) {
		return sl_false;
	}
	m_thread = Thread::start(SLIB_CALLBACK_CLASS(STunnelManager, _run, this));
	if (m_thread.isNull()) {
		return sl_false;
	}
	m_flagInited = sl_true;
	return sl_true;
}

void STunnelManager::release()
{
	MutexLocker lock(getLocker());
	if (!m_flagInited) {
		return;
	}
	if (m_asyncLoop.isNotNull()) {
		m_asyncLoop->release();
		m_asyncLoop.setNull();
	}
	if (m_thread.isNotNull()) {
		m_thread->finishAndWait();
		m_thread.setNull();
	}
	m_tunnels.clear();
	m_flagInited = sl_false;
}

sl_bool STunnelManager::init()
{
	return _init();
}

sl_bool STunnelManager::initWithConfiguration(const Variant& var)
{
	if (!_init()) {
		return sl_false;
	}
	sl_uint32 datagram_queue_limit = var.getField("datagram_queue_limit").getUint32(5000);
	sl_uint32 auto_reconnect_timeout = var.getField("auto_reconnect_timeout").getUint32(getAutoReconnectTimeout());
	setAutoReconnectTimeout(auto_reconnect_timeout);
	Variant varTunnels = var.getField("tunnels");
	if (varTunnels.isNotEmpty()) {
		Iterator< Pair<String, Variant> > iterator = varTunnels.getVariantMap().iterator();
		Pair<String, Variant> pair;
		while (iterator.next(&pair)) {
			if (pair.key.isNotEmpty()) {
				STunnelClientParam scp;
				String strAddress = pair.value.getField("server").getString();
				if (scp.serviceAddress.setHostAddress(strAddress)) {
					String key = pair.value.getField("master_key").getString();
					scp.masterKey = key;
					scp.datagram_queue_limit = datagram_queue_limit;
					scp.listener = WeakRef<STunnelManager>(this);
					addTunnel(pair.key, scp);
				}
			}
		}
	}
	return sl_true;
}

Ref<STunnelClient> STunnelManager::getTunnel(const String& name)
{
	return m_tunnels.getValue(name, Ref<STunnelClient>::null());
}

Ref<STunnelClient> STunnelManager::addTunnel(const String& name, const STunnelClientParam& param)
{
	Ref<STunnelClient> tunnel = STunnelClient::create(param, m_asyncLoop);
	if (tunnel.isNotNull()) {
		if (m_tunnels.put(name, tunnel)) {
			tunnel->connect();
			return tunnel;
		}
	}
	return Ref<STunnelClient>::null();
}

String STunnelManager::getStatusReport()
{
	StringBuffer ret;
	ListLocker< Pair< String, Ref<STunnelClient> > > tunnels(m_tunnels.pairs());
	for (sl_size i = 0; i < tunnels.count(); i++) {
		Ref<STunnelClient> tunnel = tunnels[i].value;
		if (tunnel.isNotNull()) {
			ret.add(tunnels[i].key + "(" + tunnel->getServiceAddress().toString() + ")");
			ret.add(": ");
			if (tunnel->isConnected()) {
				ret.add("Connected");
			} else if (tunnel->isConnecting()) {
				ret.add("Connecting");
			} else {
				ret.add("Not Connected");
			}
			ret.add("\r\n");
		}
	}
	return ret.merge();
}

void STunnelManager::_run()
{
	while (Thread::isNotStoppingCurrent()) {
		Time now = Time::now();
		ListLocker< Ref<STunnelClient> > tunnels(m_tunnels.values());
		for (sl_size i = 0; i < tunnels.count(); i++) {
			Ref<STunnelClient> tunnel = tunnels[i];
			if (tunnel.isNotNull()) {
				if (tunnel->isConnecting()) {
				} else if (tunnel->isConnected()) {
					Time timeR = tunnel->getLastReceivedTime();
					Time timeS = tunnel->getLastSentTime();
					if (timeS > timeR && (timeS - timeR).getMillisecondsCount() > getAutoReconnectTimeout()) {
						//tunnel->reconnect();
					}
				} else {
					if ((now - tunnel->getLastConnectingTime()).getMillisecondsCount() > getAutoReconnectTimeout()) {
						tunnel->connect();
					}
				}
			}
		}
		Thread::sleep(3000);
	}
}

void STunnelManager::onSTunnelRawIP(STunnelClient* client, const void* ip, sl_uint32 len)
{
	PtrLocker<ISTunnelClientListener> listener(getTunnelListener());
	if (listener.isNotNull()) {
		listener->onSTunnelRawIP(client, ip, len);
	}
}

void STunnelManager::onSTunnelDataUDP(STunnelClient* client, sl_uint64 portId, const SocketAddress& addressFrom, const void* data, sl_uint32 n)
{
	PtrLocker<ISTunnelClientListener> listener(getTunnelListener());
	if (listener.isNotNull()) {
		listener->onSTunnelDataUDP(client, portId, addressFrom, data, n);
	}
}
SLIB_SNET_NAMESPACE_END

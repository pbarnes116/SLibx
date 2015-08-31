#include "../../../inc/slibx/snet/dbip.h"

#include "../../../inc/slib/core/file.h"
#include "../../../inc/slib/core/algorithm.h"

SLIB_SNET_NAMESPACE_BEGIN
DbIp::DbIp()
{
}

DbIp::~DbIp()
{
}

template<>
SLIB_INLINE int Compare<DbIp::IPv4Item, sl_uint32>::compare(const DbIp::IPv4Item& a, const sl_uint32& b)
{
	return Compare<sl_uint32>::compare(a.start, b);
}

const char* DbIp::getCountryCode(const IPv4Address& _ipv4)
{
	sl_size index = 0;
	sl_uint32 ipv4 = _ipv4.toInt();
	if (BinarySearch<IPv4Item, sl_uint32>::search(m_ipv4, m_countIPv4, ipv4, &index)) {
		return m_ipv4[index].code;
	} else if (index > 0 && index < m_countIPv4) {
		if (ipv4 >= m_ipv4[index - 1].start && ipv4 <= m_ipv4[index - 1].end) {
			return m_ipv4[index - 1].code;
		}
	}
	return sl_null;
}

template<>
SLIB_INLINE int Compare<DbIp::IPv6Item, IPv6Address>::compare(const DbIp::IPv6Item& a, const IPv6Address& b)
{
	return Compare<IPv6Address>::compare(a.start, b);
}

const char* DbIp::getCountryCode(const IPv6Address& ipv6)
{
	sl_size index = 0;
	if (BinarySearch<IPv6Item, IPv6Address>::search(m_ipv6, m_countIPv6, ipv6, &index)) {
		return m_ipv6[index].code;
	} else if (index > 0 && index < m_countIPv6) {
		if (ipv6 >= m_ipv6[index - 1].start && ipv6 <= m_ipv6[index - 1].end) {
			return m_ipv6[index - 1].code;
		}
	}
	return sl_null;
}

Ref<DbIp> DbIp::create(const void* _data, sl_size _size)
{
	Ref<DbIp> ret;
	if (_size == 0 || _size >= 0x80000000) {
		return ret;
	}
	char* sz = (char*)_data;
	sl_uint32 len = (sl_uint32)_size;

	List<IPv4Item> list4 = List<IPv4Item>::create(len / 64);
	List<IPv6Item> list6 = List<IPv6Item>::create(len / 128);

	sl_uint32 pos = 0;
	sl_int32 resultParse;
	while (pos < len) {
		IPv4Item item4;
		IPv6Item item6;
		do {
			if (pos >= len || sz[pos] != '"') {
				break;
			}
			pos++;
			IPAddress ip;
			resultParse = IPAddress::parse(&ip, sz, pos, len);
			if (resultParse == SLIB_PARSE_ERROR) {
				break;
			}
			pos = resultParse;
			if (ip.isIPv4()) {
				item4.start = ip.getIPv4().toInt();
			} else {
				item6.start = ip.getIPv6();
			}
			pos = resultParse;
			if (pos >= len || sz[pos] != '"') {
				break;
			}
			pos++;
			if (pos >= len || sz[pos] != ',') {
				break;
			}
			pos++;
			if (pos >= len || sz[pos] != '"') {
				break;
			}
			pos++;
			if (ip.isIPv4()) {
				IPv4Address ip4;
				resultParse = IPv4Address::parse(&ip4, sz, pos, len);
				if (resultParse == SLIB_PARSE_ERROR) {
					break;
				}
				item4.end = ip4.toInt();
			} else {
				IPv6Address ip6;
				resultParse = IPv6Address::parse(&ip6, sz, pos, len);
				if (resultParse == SLIB_PARSE_ERROR) {
					break;
				}
				item6.end = ip6;
			}
			pos = resultParse;
			if (pos >= len || sz[pos] != '"') {
				break;
			}
			pos++;
			if (pos >= len || sz[pos] != ',') {
				break;
			}
			pos++;
			if (pos + 4 > len || sz[pos] != '"' || sz[pos+3] != '"') {
				break;
			}
			if (ip.isIPv4()) {
				item4.code[0] = sz[pos + 1];
				item4.code[1] = sz[pos + 2];
				item4.code[2] = 0;
				item4.code[3] = 0;
				if (!(list4.add(item4))) {
					return ret;
				}
			} else {
				item6.code[0] = sz[pos + 1];
				item6.code[1] = sz[pos + 2];
				item6.code[2] = 0;
				item6.code[3] = 0;
				if (!(list6.add(item6))) {
					return ret;
				}
			}
			pos += 4;
		} while (0);
		while (pos < len && sz[pos] != '\r' && sz[pos] != '\n') {
			pos++;
		}
		while (pos < len && (sz[pos] == '\r' || sz[pos] == '\n')) {
			pos++;
		}
	}
	if (list4.count() == 0 && list6.count() == 0) {
		return ret;
	}	
	ret = new DbIp;
	if (ret.isNotNull()) {
		ret->m_listIPv4 = list4;
		ret->m_ipv4 = list4.getBuffer();
		ret->m_countIPv4 = (sl_uint32)(list4.count());

		ret->m_listIPv6 = list6;
		ret->m_ipv6 = list6.getBuffer();
		ret->m_countIPv6 = (sl_uint32)(list6.count());
	}
	return ret;
}

Ref<DbIp> DbIp::create(const String& pathToCSVFile)
{
	Ref<DbIp> ret;
	Memory mem = File::readAllBytes(pathToCSVFile);
	if (mem.isNotEmpty()) {
		ret = DbIp::create(mem.getBuf(), mem.getSize());
	}
	return ret;
}
SLIB_SNET_NAMESPACE_END

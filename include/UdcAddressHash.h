// udp-connect
// Kyle J Burgess

#ifndef UDC_ADDRESS_HASH
#define UDC_ADDRESS_HASH

#include "UdcAddressMux.h"

#include <unordered_map>
#include <cstring>

template<class T>
class UdcAddressMap
{
protected:
    struct Hasher;
    struct Comparator;
    using Map = std::unordered_map<UdcAddressMux, T, Hasher, Comparator>;
public:

    void insert(const UdcAddressMux& address, T&& val)
    {
        m_map[address] = val;
    }

    typename Map::iterator find(const UdcAddressMux& address)
    {
        return m_map.find(address);
    }

protected:

    struct Hasher
    {
        uint32_t operator()(const UdcAddressMux& x) const
        {
            return *reinterpret_cast<const uint32_t*>(&x.address.ipv4.octets);
        }
    };

    struct Comparator
    {
        bool operator()(const UdcAddressMux& a, UdcAddressMux& b) const
        {
            if (a.family != b.family)
                return false;

            if (a.port != b.port)
                return false;

            if (a.family == UDC_IPV6)
            {
                return memcmp(
                    a.address.ipv6.segments,
                    b.address.ipv6.segments,
                    sizeof(a.address.ipv6.segments)) == 0;
            }

            return memcmp(
                a.address.ipv4.octets,
                b.address.ipv4.octets,
                sizeof(a.address.ipv4.octets)) == 0;
        }
    };

    Map m_map;
};

#endif

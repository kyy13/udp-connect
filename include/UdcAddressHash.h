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
    struct CompareEquals;
    using Map = std::unordered_map<UdcAddressMux, T, Hasher, CompareEquals>;
public:

    template<class U>
    void insert(const UdcAddressMux& address, U&& val)
    {
        static_assert(std::is_same<std::decay_t<U>, std::decay_t<T>>::value, "U must be the same as T");
        m_map[address] = std::forward<U>(val);
    }

    void erase(const UdcAddressMux& address)
    {
        m_map.erase(address);
    }

    typename Map::const_iterator find(const UdcAddressMux& address) const
    {
        return m_map.find(address);
    }

    typename Map::const_iterator cend() const
    {
        return m_map.end();
    }

protected:

    struct Hasher
    {
        uint32_t operator()(const UdcAddressMux& x) const
        {
            return *reinterpret_cast<const uint32_t*>(&x.address.ipv4.octets);
        }
    };

    struct CompareEquals
    {
        bool operator()(const UdcAddressMux& a, const UdcAddressMux& b) const
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

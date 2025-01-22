#pragma once

#include <bit>
#include <cstdint>

#if defined(_MSC_VER)
#define bswap_u16 _byteswap_ushort
#define bswap_u32 _byteswap_ulong
#define bswap_u64 _byteswap_uint64
#else
#define bswap_u16 __builtin_bswap16
#define bswap_u32 __builtin_bswap32
#define bswap_u64 __builtin_bswap64
#endif

namespace Endian {

inline uint64_t be_u64_read(const uint8_t* p) {
    if constexpr (std::endian::native == std::endian::big) {
        return *reinterpret_cast<const uint64_t*>(p);
    } else {
        return bswap_u64(*reinterpret_cast<const uint64_t*>(p));
    }
}

inline void be_u64_write(uint8_t* p, uint64_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        *reinterpret_cast<uint64_t*>(p) = value;
    } else {
        *reinterpret_cast<uint64_t*>(p) = bswap_u64(value);
    }
}

inline uint32_t be_u32_read(const uint8_t* p) {
    if constexpr (std::endian::native == std::endian::big) {
        return *reinterpret_cast<const uint32_t*>(p);
    } else {
        return bswap_u32(*reinterpret_cast<const uint32_t*>(p));
    }
}

inline void be_u32_write(uint8_t* p, uint32_t value) {
    if constexpr (std::endian::native == std::endian::big) {
        *reinterpret_cast<uint32_t*>(p) = value;
    } else {
        *reinterpret_cast<uint32_t*>(p) = bswap_u32(value);
    }
}

}  // namespace Endian

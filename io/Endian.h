#pragma once

#include <cstdint>

namespace mapper::io {

/// Shapefiles mix byte orders: the file and record headers are big-endian,
/// the geometry payloads are little-endian (ESRI Shapefile Technical
/// Description, p.2). These helpers make that explicit at every call site.

[[nodiscard]] std::uint16_t byteSwap16(std::uint16_t value);
[[nodiscard]] std::uint32_t byteSwap32(std::uint32_t value);
[[nodiscard]] std::uint64_t byteSwap64(std::uint64_t value);

[[nodiscard]] constexpr bool hostIsLittleEndian() {
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
    return __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__;
#else
    return true;  // MSVC targets are little-endian.
#endif
}

}  // namespace mapper::io

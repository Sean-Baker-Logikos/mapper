#include "io/Endian.h"

namespace mapper::io {

std::uint16_t byteSwap16(std::uint16_t value) {
    return static_cast<std::uint16_t>((value >> 8) | (value << 8));
}

std::uint32_t byteSwap32(std::uint32_t value) {
    return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8) |
           ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24);
}

std::uint64_t byteSwap64(std::uint64_t value) {
    return ((value & 0x00000000000000FFull) << 56) | ((value & 0x000000000000FF00ull) << 40) |
           ((value & 0x0000000000FF0000ull) << 24) | ((value & 0x00000000FF000000ull) << 8) |
           ((value & 0x000000FF00000000ull) >> 8)  | ((value & 0x0000FF0000000000ull) >> 24) |
           ((value & 0x00FF000000000000ull) >> 40) | ((value & 0xFF00000000000000ull) >> 56);
}

}  // namespace mapper::io

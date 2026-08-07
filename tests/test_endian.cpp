#include "io/Endian.h"

#include <catch2/catch_test_macros.hpp>

using namespace mapper::io;

TEST_CASE("16-bit byte swap reverses byte order", "[endian]") {
    REQUIRE(byteSwap16(0x1234u) == 0x3412u);
    REQUIRE(byteSwap16(byteSwap16(0xABCDu)) == 0xABCDu);
}

TEST_CASE("32-bit byte swap reverses byte order", "[endian]") {
    REQUIRE(byteSwap32(0x12345678u) == 0x78563412u);
    REQUIRE(byteSwap32(byteSwap32(0xDEADBEEFu)) == 0xDEADBEEFu);
}

TEST_CASE("64-bit byte swap reverses byte order", "[endian]") {
    REQUIRE(byteSwap64(0x0123456789ABCDEFull) == 0xEFCDAB8967452301ull);
    REQUIRE(byteSwap64(byteSwap64(0x1122334455667788ull)) == 0x1122334455667788ull);
}

TEST_CASE("The shapefile magic number decodes from big-endian", "[endian]") {
    // Byte 0 of a .shp file header is 9994 stored big-endian.
    constexpr std::uint32_t bigEndianOnDisk = 0x0A270000u;
    REQUIRE(byteSwap32(bigEndianOnDisk) == 9994u);
}

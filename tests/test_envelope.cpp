#include "model/Envelope.h"

#include <catch2/catch_test_macros.hpp>

using mapper::model::Envelope;

TEST_CASE("A default-constructed empty envelope reports empty", "[envelope]") {
    const Envelope e = Envelope::empty();
    REQUIRE(e.isEmpty());
    REQUIRE(e.width() == 0.0);
    REQUIRE(e.height() == 0.0);
}

TEST_CASE("Expanding an empty envelope adopts the point exactly", "[envelope]") {
    Envelope e = Envelope::empty();
    e.expand(3.0, 4.0);

    REQUIRE_FALSE(e.isEmpty());
    REQUIRE(e.xmin == 3.0);
    REQUIRE(e.xmax == 3.0);
    REQUIRE(e.ymin == 4.0);
    REQUIRE(e.ymax == 4.0);
}

TEST_CASE("Envelope grows to cover every expanded point", "[envelope]") {
    Envelope e = Envelope::empty();
    e.expand(-1.0, -2.0);
    e.expand(5.0, 7.0);

    REQUIRE(e.xmin == -1.0);
    REQUIRE(e.ymin == -2.0);
    REQUIRE(e.xmax == 5.0);
    REQUIRE(e.ymax == 7.0);
    REQUIRE(e.width() == 6.0);
    REQUIRE(e.height() == 9.0);
}

TEST_CASE("Intersection is false when either envelope is empty", "[envelope]") {
    Envelope populated = Envelope::empty();
    populated.expand(0.0, 0.0);
    populated.expand(1.0, 1.0);

    REQUIRE_FALSE(populated.intersects(Envelope::empty()));
    REQUIRE_FALSE(Envelope::empty().intersects(populated));
}

TEST_CASE("Envelopes sharing only an edge still intersect", "[envelope]") {
    Envelope a{0.0, 0.0, 1.0, 1.0};
    Envelope b{1.0, 0.0, 2.0, 1.0};

    REQUIRE(a.intersects(b));
    REQUIRE(b.intersects(a));
}

TEST_CASE("Disjoint envelopes do not intersect", "[envelope]") {
    Envelope a{0.0, 0.0, 1.0, 1.0};
    Envelope b{2.0, 2.0, 3.0, 3.0};

    REQUIRE_FALSE(a.intersects(b));
}

TEST_CASE("contains() honours inclusive bounds", "[envelope]") {
    const Envelope e{0.0, 0.0, 10.0, 10.0};

    REQUIRE(e.contains(5.0, 5.0));
    REQUIRE(e.contains(0.0, 0.0));
    REQUIRE(e.contains(10.0, 10.0));
    REQUIRE_FALSE(e.contains(-0.1, 5.0));
    REQUIRE_FALSE(e.contains(5.0, 10.1));
}

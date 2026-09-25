#pragma once

#include <limits>

namespace mapper::model {

/// Axis-aligned bounding box in world coordinates.
/// Default-constructed envelopes are empty (inverted bounds), so the first
/// expand() call adopts the point exactly.
struct Envelope {
    double xmin = std::numeric_limits<double>::infinity();
    double ymin = std::numeric_limits<double>::infinity();
    double xmax = -std::numeric_limits<double>::infinity();
    double ymax = -std::numeric_limits<double>::infinity();

    static Envelope empty();

    [[nodiscard]] bool isEmpty() const;
    [[nodiscard]] double width() const;
    [[nodiscard]] double height() const;

    void expand(double x, double y);
    void expand(const Envelope& other);

    [[nodiscard]] bool intersects(const Envelope& other) const;
    [[nodiscard]] bool contains(double x, double y) const;
};

}  // namespace mapper::model

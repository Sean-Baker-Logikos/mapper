#include "model/Envelope.h"

#include <algorithm>
#include <limits>

namespace mapper::model {

Envelope Envelope::empty() {
    constexpr double inf = std::numeric_limits<double>::infinity();
    return Envelope{inf, inf, -inf, -inf};
}

bool Envelope::isEmpty() const {
    return xmin > xmax || ymin > ymax;
}

double Envelope::width() const {
    return isEmpty() ? 0.0 : xmax - xmin;
}

double Envelope::height() const {
    return isEmpty() ? 0.0 : ymax - ymin;
}

void Envelope::expand(double x, double y) {
    xmin = std::min(xmin, x);
    ymin = std::min(ymin, y);
    xmax = std::max(xmax, x);
    ymax = std::max(ymax, y);
}

void Envelope::expand(const Envelope& other) {
    if (other.isEmpty()) {
        return;
    }
    expand(other.xmin, other.ymin);
    expand(other.xmax, other.ymax);
}

bool Envelope::intersects(const Envelope& other) const {
    if (isEmpty() || other.isEmpty()) {
        return false;
    }
    return !(other.xmin > xmax || other.xmax < xmin || other.ymin > ymax || other.ymax < ymin);
}

bool Envelope::contains(double x, double y) const {
    return !isEmpty() && x >= xmin && x <= xmax && y >= ymin && y <= ymax;
}

}  // namespace mapper::model

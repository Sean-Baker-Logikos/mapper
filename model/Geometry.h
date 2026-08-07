#pragma once

#include <cstdint>

namespace mapper::model {

/// ESRI Shapefile shape type codes (ESRI Shapefile Technical Description, p.4).
enum class ShapeType : std::int32_t {
    Null        = 0,
    Point       = 1,
    PolyLine    = 3,
    Polygon     = 5,
    MultiPoint  = 8,
    PointZ      = 11,
    PolyLineZ   = 13,
    PolygonZ    = 15,
    MultiPointZ = 18,
    PointM      = 21,
    PolyLineM   = 23,
    PolygonM    = 25,
    MultiPointM = 28,
    MultiPatch  = 31,  ///< Out of scope; rejected by the reader.
};

/// A world-coordinate vertex.
///
/// Deliberately NOT QPointF: model/ and io/ must not depend on Qt
/// (SPECIFICATION.md section 5). Conversion happens in the renderer.
struct Vertex {
    double x = 0.0;
    double y = 0.0;
};

/// A contiguous run of vertices within Layer::vertices -- one ring or one line part.
struct Part {
    std::uint32_t firstVertex = 0;
    std::uint32_t vertexCount = 0;
};

}  // namespace mapper::model

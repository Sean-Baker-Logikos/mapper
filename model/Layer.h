#pragma once

#include "model/Envelope.h"
#include "model/Geometry.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mapper::model {

struct Feature {
    ShapeType     type        = ShapeType::Null;
    Envelope      bbox        = Envelope::empty();
    std::uint32_t firstPart   = 0;
    std::uint32_t partCount   = 0;
    std::uint32_t recordNumber = 0;
};

/// One shapefile's worth of geometry.
///
/// Vertices live in ONE flat contiguous buffer; features and parts are index
/// ranges into it. This is the central performance decision in the design --
/// see SPECIFICATION.md section 5.1. Do not replace with nested vectors.
struct Layer {
    std::string          name;
    ShapeType            type = ShapeType::Null;
    Envelope             extent = Envelope::empty();
    std::vector<Feature> features;
    std::vector<Part>    parts;
    std::vector<Vertex>  vertices;
};

}  // namespace mapper::model

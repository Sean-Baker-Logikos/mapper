#pragma once

#include "model/Layer.h"

#include <QWidget>

#include <optional>

namespace mapper::view {

/// Central map widget.
///
/// Phase 0 scaffold: paints a placeholder when no layer is loaded. The
/// Viewport transform, IRenderer indirection and the actual geometry drawing
/// arrive in Phase 2 (KAN-35, KAN-39).
class MapCanvas : public QWidget {
    Q_OBJECT

public:
    explicit MapCanvas(QWidget* parent = nullptr);

    void setLayer(model::Layer layer);
    void clearLayer();
    [[nodiscard]] bool hasLayer() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::optional<model::Layer> m_layer;
};

}  // namespace mapper::view

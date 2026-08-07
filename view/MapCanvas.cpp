#include "view/MapCanvas.h"

#include <QPainter>
#include <QPaintEvent>

namespace mapper::view {

MapCanvas::MapCanvas(QWidget* parent) : QWidget(parent) {
    setAutoFillBackground(true);
    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);
}

void MapCanvas::setLayer(model::Layer layer) {
    m_layer = std::move(layer);
    update();
}

void MapCanvas::clearLayer() {
    m_layer.reset();
    update();
}

bool MapCanvas::hasLayer() const {
    return m_layer.has_value();
}

void MapCanvas::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), palette().base());

    if (!m_layer) {
        painter.setPen(palette().mid().color());
        painter.drawText(rect(), Qt::AlignCenter, tr("No layer loaded"));
        return;
    }

    // Phase 2: transform via Viewport and draw through IRenderer.
    painter.setPen(palette().text().color());
    painter.drawText(rect(), Qt::AlignCenter,
                     tr("%1 features").arg(static_cast<qulonglong>(m_layer->features.size())));
}

}  // namespace mapper::view

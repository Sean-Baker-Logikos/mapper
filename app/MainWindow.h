#pragma once

#include <QMainWindow>

namespace mapper::view {
class MapCanvas;
}

namespace mapper::app {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void buildMenus();

    view::MapCanvas* m_canvas = nullptr;
};

}  // namespace mapper::app

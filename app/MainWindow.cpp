#include "app/MainWindow.h"

#include "view/MapCanvas.h"

#include <QAction>
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>

namespace mapper::app {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("mapper"));
    resize(1024, 768);

    m_canvas = new view::MapCanvas(this);
    setCentralWidget(m_canvas);

    buildMenus();
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::buildMenus() {
    // Phase 2 (KAN-49) fills in Open, drag-and-drop, CLI argument and recent files.
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    QAction* quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->setEnabled(false);  // Populated in Phase 2 (KAN-43).
}

}  // namespace mapper::app

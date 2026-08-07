#include "app/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Logikos"));
    QCoreApplication::setApplicationName(QStringLiteral("mapper"));
    QCoreApplication::setApplicationVersion(QStringLiteral(MAPPER_VERSION));

    mapper::app::MainWindow window;
    window.show();

    return QApplication::exec();
}

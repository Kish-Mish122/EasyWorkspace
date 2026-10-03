#include "MainWindow.h"

#include <Common.h>
#include <ThemeManager.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    easy::setupApp(app, QStringLiteral("EasySheets"), QColor(0x21, 0x73, 0x46));
    easy::setTheme(app, QColor(0x21, 0x73, 0x46), easy::ThemeManager::Theme::Light);

    MainWindow w;
    w.show();
    if (app.arguments().size() > 1)
        w.openFile(app.arguments().at(1));
    return app.exec();
}

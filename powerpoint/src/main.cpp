#include "MainWindow.h"

#include <Common.h>
#include <ThemeManager.h>
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    easy::setupApp(app, QStringLiteral("EasySlides"), QColor(0xc4, 0x3e, 0x1c));
    easy::setTheme(app, QColor(0xc4, 0x3e, 0x1c), easy::ThemeManager::Theme::Light);

    MainWindow w;
    w.show();
    if (app.arguments().size() > 1)
        w.openFile(app.arguments().at(1));
    return app.exec();
}

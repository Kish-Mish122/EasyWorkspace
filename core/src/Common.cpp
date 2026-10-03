#include "Common.h"
#include "ThemeManager.h"

#include <QApplication>
#include <QSettings>
#include <QStyleFactory>

namespace easy {

void setupApp(QApplication& app, const QString& displayName, const QColor& accent)
{
    app.setApplicationName(displayName);
    app.setOrganizationName(QStringLiteral("EasyWorkspace"));
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    ThemeManager::instance().apply(app, accent, ThemeManager::Theme::Light);
}

QMessageBox::StandardButton askSave(QWidget* parent, const QString& docName)
{
    return QMessageBox::question(
        parent, QStringLiteral("Несохранённые изменения"),
        QStringLiteral("Сохранить изменения в «%1»?").arg(docName),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
}

void saveWindowState(QMainWindow* w, const QString& appName, const QString& key)
{
    QSettings settings(appName, key);
    settings.setValue("geometry", w->saveGeometry());
    settings.setValue("windowState", w->saveState());
}

void loadWindowState(QMainWindow* w, const QString& appName, const QString& key)
{
    QSettings settings(appName, key);
    const QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        w->restoreGeometry(geometry);
        w->restoreState(settings.value("windowState").toByteArray());
    }
}

} // namespace easy

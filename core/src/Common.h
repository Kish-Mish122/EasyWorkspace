#pragma once
#include <QColor>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include "ThemeManager.h"

class QApplication;
class QWidget;

namespace easy {

// Общий вид всех приложений: стиль Fusion, светлая палитра, акцентный цвет.
void setupApp(QApplication& app, const QString& displayName, const QColor& accent);

// Переключение темы: Light или Dark.
void setTheme(QApplication& app, const QColor& accent, ThemeManager::Theme theme);

// Диалог «Сохранить изменения?». Возвращает Save, Discard или Cancel.
QMessageBox::StandardButton askSave(QWidget* parent, const QString& docName);

// Сохранение и загрузка геометрии окна.
void saveWindowState(QMainWindow* w, const QString& appName, const QString& key = QStringLiteral("window"));
void loadWindowState(QMainWindow* w, const QString& appName, const QString& key = QStringLiteral("window"));

} // namespace easy

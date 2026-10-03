#pragma once

#include <QColor>
#include <QPalette>
#include <QString>

class QApplication;

namespace easy {

class ThemeManager {
public:
    enum class Theme { Light, Dark };

    static ThemeManager& instance();

    void apply(QApplication& app, const QColor& accent, Theme theme = Theme::Light);
    void toggle(QApplication& app, const QColor& accent);
    Theme current() const { return current_; }

private:
    ThemeManager() = default;

    Theme current_ = Theme::Light;

    QString buildLightQss(const QColor& accent);
    QString buildDarkQss(const QColor& accent);
    QPalette buildLightPalette(const QColor& accent);
    QPalette buildDarkPalette(const QColor& accent);
};

void setTheme(QApplication& app, const QColor& accent, ThemeManager::Theme theme);

} // namespace easy

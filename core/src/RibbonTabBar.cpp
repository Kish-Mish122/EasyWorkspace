#include "RibbonTabBar.h"

namespace easy {

RibbonTabBar::RibbonTabBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(Height);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
}

void RibbonTabBar::addTab(const QString& name)
{
    Tab tab;
    tab.name = name;
    tabs_.append(tab);
    updateTabRects();
    update();
}

void RibbonTabBar::setAccentColor(const QColor& accent)
{
    accent_ = accent;
    update();
}

void RibbonTabBar::setThemeLight(bool light)
{
    themeLight_ = light;
    update();
}

void RibbonTabBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= tabs_.size() || index == current_)
        return;
    current_ = index;
    updateTabRects();
    update();
    emit currentTabChanged(current_);
}

void RibbonTabBar::updateTabRects()
{
    const int totalWidth = width();
    const int tabCount = tabs_.size();
    if (tabCount == 0) return;

    const int tabWidth = std::min(totalWidth / tabCount, 160);
    const int startX = (totalWidth - tabWidth * tabCount) / 2;

    for (int i = 0; i < tabCount; ++i) {
        tabs_[i].rect = QRect(startX + i * tabWidth, 0, tabWidth, Height);
    }
}

void RibbonTabBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor bgColor = themeLight_ ? QColor(0xf8, 0xf9, 0xfa, 200) : QColor(0x1e, 0x1e, 0x1e, 200);
    p.fillRect(rect(), bgColor);

    // Bottom border
    QColor borderColor = themeLight_ ? QColor(0xe5, 0xe5, 0xe5, 180) : QColor(0x3e, 0x3e, 0x42, 180);
    p.fillRect(0, height() - 1, width(), 1, borderColor);

    // Draw tabs
    for (int i = 0; i < tabs_.size(); ++i) {
        const auto& tab = tabs_[i];
        const bool isActive = (i == current_);
        const bool isHover = tab.hovered && !isActive;

        // Tab background
        if (isActive) {
            // Active tab: accent color
            p.setBrush(accent_);
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(tab.rect.adjusted(4, 0, -4, 0), 6, 6);

            // Active tab text: white
            p.setPen(Qt::white);
        } else if (isHover) {
            // Hovered tab: subtle highlight
            p.setBrush(themeLight_ ? QColor(0xe8, 0xe8, 0xe8, 120) : QColor(0x3e, 0x3e, 0x42, 120));
            p.setPen(Qt::NoPen);
            p.drawRoundedRect(tab.rect.adjusted(4, 0, -4, 0), 6, 6);

            p.setPen(themeLight_ ? QColor(0x20, 0x20, 0x20) : QColor(0xf0, 0xf0, 0xf0));
        } else {
            // Inactive tab
            p.setBrush(Qt::NoBrush);
            p.setPen(themeLight_ ? QColor(0x60, 0x60, 0x60) : QColor(0xa0, 0xa0, 0xa0));
        }

        p.setFont(QFont(QStringLiteral("Segoe UI"), 10, isActive ? QFont::DemiBold : QFont::Normal));
        p.drawText(tab.rect, Qt::AlignVCenter | Qt::AlignHCenter, tab.name);
    }
}

void RibbonTabBar::mousePressEvent(QMouseEvent* event)
{
    mousePos_ = event->pos();
    for (int i = 0; i < tabs_.size(); ++i) {
        if (tabs_[i].rect.contains(event->pos())) {
            setCurrentIndex(i);
            return;
        }
    }
}

void RibbonTabBar::mouseMoveEvent(QMouseEvent* event)
{
    mousePos_ = event->pos();
    bool changed = false;
    for (int i = 0; i < tabs_.size(); ++i) {
        bool wasHover = tabs_[i].hovered;
        tabs_[i].hovered = tabs_[i].rect.contains(event->pos()) && (i != current_);
        if (wasHover != tabs_[i].hovered)
            changed = true;
    }
    if (changed)
        update();
}

void RibbonTabBar::leaveEvent(QEvent*)
{
    for (auto& tab : tabs_)
        tab.hovered = false;
    update();
}

} // namespace easy

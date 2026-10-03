#include "RibbonPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyle>

namespace easy {

RibbonPanel::RibbonPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(8, 4, 8, 4);
    mainLayout->setSpacing(12);
}

void RibbonPanel::addGroup(const QString& name)
{
    Group g;
    g.name = name;
    groups_.append(g);
    rebuildLayout();
}

QAction* RibbonPanel::addAction(QAction* action, int group)
{
    if (group < 0 || group >= groups_.size()) return nullptr;
    groups_[group].widgets.append(nullptr); // placeholder for action widget
    rebuildLayout();
    return action;
}

QWidget* RibbonPanel::addWidget(QWidget* widget, int group)
{
    if (group < 0 || group >= groups_.size()) {
        delete widget;
        return nullptr;
    }
    groups_[group].widgets.append(widget);
    rebuildLayout();
    return widget;
}

void RibbonPanel::addSeparator(int group)
{
    if (group >= 0 && group < groups_.size()) {
        groups_[group].hasSeparator = true;
        rebuildLayout();
    }
}

void RibbonPanel::setAccentColor(const QColor& accent)
{
    accent_ = accent;
    update();
}

void RibbonPanel::setThemeLight(bool light)
{
    themeLight_ = light;
    update();
}

void RibbonPanel::rebuildLayout()
{
    // Clear existing layout items
    auto* layout = qobject_cast<QHBoxLayout*>(this->layout());
    if (!layout) return;

    // Clear existing layout items
    while (layout->count() > 0) {
        QLayoutItem* item = layout->takeAt(0);
        if (item->spacerItem()) {
            delete item->spacerItem();
        } else if (item->widget()) {
            // Don't delete widgets that are in groups_, just remove from layout
        } else {
            delete item;
        }
    }

    for (int g = 0; g < groups_.size(); ++g) {
        const auto& group = groups_[g];

        // Group container
        auto* groupWidget = new QWidget(this);
        auto* groupLayout = new QVBoxLayout(groupWidget);
        groupLayout->setContentsMargins(0, 0, 0, 0);
        groupLayout->setSpacing(2);

        // Group title
        auto* groupTitle = new QLabel(group.name, groupWidget);
        groupTitle->setFont(QFont(QStringLiteral("Segoe UI"), 8, QFont::DemiBold));
        groupTitle->setStyleSheet(
            QString("color:%1; padding:2px 0px;").arg(
                themeLight_ ? "#808080" : "#808080"));
        groupTitle->setAlignment(Qt::AlignCenter);
        groupLayout->addWidget(groupTitle);

        // Group buttons/widgets
        auto* contentLayout = new QGridLayout;
        contentLayout->setContentsMargins(2, 2, 2, 2);
        contentLayout->setSpacing(2);

        int row = 0, col = 0;
        for (auto* w : group.widgets) {
            if (w) {
                contentLayout->addWidget(w, row, col);
                ++col;
                if (col > 2) { col = 0; ++row; }
            }
        }

        groupLayout->addLayout(contentLayout);
        groupLayout->addStretch();

        layout->addWidget(groupWidget);

        // Separator between groups
        if (g < groups_.size() - 1) {
            auto* sep = new QFrame(this);
            sep->setFrameShape(QFrame::VLine);
            sep->setStyleSheet("color:" + QString(themeLight_ ? "#e5e5e5" : "#3e3e42"));
            layout->addWidget(sep);
        }
    }

    layout->addStretch();
}

void RibbonPanel::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    const QColor bgColor = themeLight_ ? QColor(0xf8, 0xf9, 0xfa, 200) : QColor(0x1e, 0x1e, 0x1e, 200);
    p.fillRect(rect(), bgColor);
}

void RibbonPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    rebuildLayout();
}

} // namespace easy

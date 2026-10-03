#pragma once

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QString>
#include <QVector>
#include <QWidget>

namespace easy {

class RibbonTabBar : public QWidget {
    Q_OBJECT

public:
    explicit RibbonTabBar(QWidget* parent = nullptr);

    void addTab(const QString& name);
    void setAccentColor(const QColor& accent);
    void setThemeLight(bool light);
    int currentIndex() const { return current_; }
    void setCurrentIndex(int index);

    static constexpr int Height = 32;

signals:
    void currentTabChanged(int index);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    struct Tab {
        QString name;
        QRect rect;
        bool hovered = false;
    };

    QVector<Tab> tabs_;
    QColor accent_;
    bool themeLight_ = true;
    int current_ = 0;
    QPoint dragStart_;
    QPoint mousePos_;

    void updateTabRects();
};

} // namespace easy

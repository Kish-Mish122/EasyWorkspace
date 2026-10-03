#pragma once

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QStyle>
#include <QWidget>

namespace easy {

class CustomTitleBar : public QWidget {
    Q_OBJECT

public:
    explicit CustomTitleBar(QWidget* parent = nullptr);

    void setWindowTitle(const QString& title);
    void setAccentColor(const QColor& accent);
    void setWindowIcon(const QIcon& icon);
    void setMaximized(bool maximized);
    void setThemeLight(bool light);

    static constexpr int Height = 36;

signals:
    void closeRequested();
    void minimizeRequested();
    void maximizeRequested();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void hitTest(const QPoint& pos, bool& inClose, bool& inMaximize, bool& inMinimize);

    QString title_;
    QIcon icon_;
    QColor accent_;
    QPoint dragStart_;
    bool maximized_ = false;
    bool themeLight_ = true;

    // Hover states for buttons
    bool hoverClose_ = false;
    bool hoverMaximize_ = false;
    bool hoverMinimize_ = false;

    // Current mouse position for hit testing
    QPoint mousePosition_;

    // Button rectangles (updated in paintEvent)
    QRect closeRect_;
    QRect maximizeRect_;
    QRect minimizeRect_;
};

} // namespace easy

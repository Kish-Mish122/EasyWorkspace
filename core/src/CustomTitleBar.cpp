#include "CustomTitleBar.h"

#include <QApplication>

namespace easy {

CustomTitleBar::CustomTitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(Height);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
}

void CustomTitleBar::setWindowTitle(const QString& title)
{
    title_ = title;
    update();
}

void CustomTitleBar::setAccentColor(const QColor& accent)
{
    accent_ = accent;
    update();
}

void CustomTitleBar::setWindowIcon(const QIcon& icon)
{
    icon_ = icon;
    update();
}

void CustomTitleBar::setMaximized(bool maximized)
{
    maximized_ = maximized;
    update();
}

void CustomTitleBar::setThemeLight(bool light)
{
    themeLight_ = light;
    update();
}

void CustomTitleBar::hitTest(const QPoint& pos, bool& inClose, bool& inMaximize, bool& inMinimize)
{
    inClose = false;
    inMaximize = false;
    inMinimize = false;

    const int btnSize = Height - 6;
    const int rightStart = width() - btnSize;

    closeRect_ = QRect(rightStart, 3, btnSize, btnSize);
    maximizeRect_ = QRect(rightStart - btnSize, 3, btnSize, btnSize);
    minimizeRect_ = QRect(rightStart - btnSize * 2, 3, btnSize, btnSize);

    inClose = closeRect_.contains(pos);
    inMaximize = maximizeRect_.contains(pos);
    inMinimize = minimizeRect_.contains(pos);
}

void CustomTitleBar::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect rect = this->rect();

    // Semi-transparent background (Mica-like transparency)
    QColor bgColor;
    if (themeLight_) {
        bgColor = QColor(0xf8, 0xf9, 0xfa, 200); // ~78% opacity
    } else {
        bgColor = QColor(0x1e, 0x1e, 0x1e, 200);
    }
    p.fillRect(rect, bgColor);

    // Accent gradient strip at top (8px)
    QLinearGradient gradient(0, 0, 0, 8);
    gradient.setColorAt(0, accent_.lighter(115));
    gradient.setColorAt(1, accent_);
    p.fillRect(0, 0, width(), 8, gradient);

    // Subtle bottom border
    QColor borderColor = themeLight_ ? QColor(0xe5, 0xe5, 0xe5, 180) : QColor(0x3e, 0x3e, 0x42, 180);
    p.fillRect(0, height() - 1, width(), 1, borderColor);

    // Window icon
    if (!icon_.isNull()) {
        QPixmap pix = icon_.pixmap(16, 16);
        p.drawPixmap(10, (height() - 16) / 2, pix);
    }

    // Window title
    QColor textColor = themeLight_ ? QColor(0x20, 0x20, 0x20) : QColor(0xf0, 0xf0, 0xf0);
    p.setPen(textColor);
    p.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    p.drawText(QRect(32, 0, width() - 200, height()), Qt::AlignLeft | Qt::AlignVCenter, title_);

    // Window control buttons
    bool inClose, inMaximize, inMinimize;
    hitTest(mousePosition_, inClose, inMaximize, inMinimize);

    QColor btnColor;
    if (themeLight_) {
        btnColor.setRgb(0xff, 0xff, 0xff, 60);
    } else {
        btnColor.setRgb(0xff, 0xff, 0xff, 40);
    }
    const QColor btnHoverClose(0xd4, 0x3a, 0x2b); // Windows 11 red close
    QColor btnHoverMax = themeLight_ ? QColor(0xe5, 0xe5, 0xe5) : QColor(0x3e, 0x3e, 0x42);
    QColor btnHoverMin = themeLight_ ? QColor(0xe5, 0xe5, 0xe5) : QColor(0x3e, 0x3e, 0x42);
    const QColor btnText = themeLight_ ? QColor(0x20, 0x20, 0x20) : QColor(0xf0, 0xf0, 0xf0);

    // Minimize button
    {
        QRect r = minimizeRect_;
        p.setBrush(inMinimize ? QColor(btnHoverMin) : btnColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(r, 4, 4);
        p.setPen(btnText);
        p.drawLine(r.center().x() - 5, r.center().y(), r.center().x() + 5, r.center().y());
    }

    // Maximize/Restore button
    {
        QRect r = maximizeRect_;
        p.setBrush(inMaximize ? QColor(btnHoverMax) : btnColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(r, 4, 4);
        p.setPen(btnText);
        if (maximized_) {
            // Restore icon: two overlapping rectangles
            p.drawRect(r.center().x() - 5, r.center().y() - 3, 8, 8);
            p.drawRect(r.center().x() - 3, r.center().y() - 5, 8, 8);
        } else {
            // Maximize icon: single rectangle
            p.drawRect(r.center().x() - 5, r.center().y() - 4, 10, 9);
        }
    }

    // Close button
    {
        QRect r = closeRect_;
        p.setBrush(inClose ? QColor(btnHoverClose) : btnColor);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(r, 4, 4);
        p.setPen(inClose ? QColor(0xffffff) : btnText);
        // X icon
        p.drawLine(r.center().x() - 4, r.center().y() - 4, r.center().x() + 4, r.center().y() + 4);
        p.drawLine(r.center().x() + 4, r.center().y() - 4, r.center().x() - 4, r.center().y() + 4);
    }
}

void CustomTitleBar::mousePressEvent(QMouseEvent* event)
{
    mousePosition_ = event->pos();
    if (event->button() == Qt::LeftButton) {
        bool inClose, inMaximize, inMinimize;
        hitTest(event->pos(), inClose, inMaximize, inMinimize);

        if (inClose) {
            emit closeRequested();
        } else if (inMaximize) {
            emit maximizeRequested();
        } else if (inMinimize) {
            emit minimizeRequested();
        } else {
            // Start drag
            dragStart_ = event->globalPosition().toPoint() - parentWidget()->frameGeometry().topLeft();
            event->accept();
        }
    }
}

void CustomTitleBar::mouseMoveEvent(QMouseEvent* event)
{
    mousePosition_ = event->pos();
    if (event->buttons() & Qt::LeftButton) {
        if (parentWidget() && !maximized_) {
            parentWidget()->move(event->globalPosition().toPoint() - dragStart_);
        }
    }

    // Update hover states
    bool inClose, inMaximize, inMinimize;
    hitTest(event->pos(), inClose, inMaximize, inMinimize);

    bool changed = (inClose != hoverClose_) || (inMaximize != hoverMaximize_) || (inMinimize != hoverMinimize_);
    if (changed) {
        hoverClose_ = inClose;
        hoverMaximize_ = inMaximize;
        hoverMinimize_ = inMinimize;
        update();
        setCursor((inClose || inMaximize || inMinimize) ? Qt::ArrowCursor : Qt::ArrowCursor);
    }
}

void CustomTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        bool inClose, inMaximize, inMinimize;
        hitTest(event->pos(), inClose, inMaximize, inMinimize);
        if (!inClose && !inMaximize && !inMinimize) {
            emit maximizeRequested();
        }
    }
}

void CustomTitleBar::leaveEvent(QEvent*)
{
    hoverClose_ = false;
    hoverMaximize_ = false;
    hoverMinimize_ = false;
    update();
}

} // namespace easy

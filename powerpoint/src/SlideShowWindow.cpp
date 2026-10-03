#include "SlideShowWindow.h"

#include "SlideScene.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <algorithm>

SlideShowWindow::SlideShowWindow(const QVector<Slide>& slides, int startIndex, QWidget* parent)
    : QWidget(parent, Qt::Window), slides_(slides), index_(std::clamp(startIndex, 0, int(slides.size()) - 1))
{
    setWindowTitle(QStringLiteral("Показ слайдов"));
    setCursor(Qt::BlankCursor);
    setFocusPolicy(Qt::StrongFocus);
}

void SlideShowWindow::go(int index)
{
    const int clamped = std::clamp(index, 0, int(slides_.size()) - 1);
    if (clamped == index_)
        return;
    index_ = clamped;
    update();
}

void SlideShowWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    if (slides_.isEmpty())
        return;

    const double scale = std::min(width() / SlideScene::kWidth, height() / SlideScene::kHeight);
    const QSizeF size(SlideScene::kWidth * scale, SlideScene::kHeight * scale);
    const QRectF target((width() - size.width()) / 2, (height() - size.height()) / 2, size.width(), size.height());
    SlideScene::paintSlide(&p, slides_[index_], target);

    p.setPen(QColor(255, 255, 255, 120));
    p.drawText(rect().adjusted(0, 0, -12, -8), Qt::AlignRight | Qt::AlignBottom,
               QStringLiteral("%1 / %2").arg(index_ + 1).arg(slides_.size()));
}

void SlideShowWindow::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        close();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_PageDown:
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        go(index_ + 1);
        break;
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_PageUp:
    case Qt::Key_Backspace:
        go(index_ - 1);
        break;
    case Qt::Key_Home:
        go(0);
        break;
    case Qt::Key_End:
        go(int(slides_.size()) - 1);
        break;
    default:
        QWidget::keyPressEvent(e);
    }
}

void SlideShowWindow::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton)
        go(index_ - 1);
    else
        go(index_ + 1);
}

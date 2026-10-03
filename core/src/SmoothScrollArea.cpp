#include "SmoothScrollArea.h"

#include <QScrollBar>
#include <QWheelEvent>

namespace easy {

SmoothScrollArea::SmoothScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setSmoothScroll(true);
}

void SmoothScrollArea::setSmoothScroll(bool enabled)
{
    smooth_ = enabled;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void SmoothScrollArea::smoothScrollTo(int value, int duration)
{
    QScrollBar* bar = verticalScrollBar();
    if (!bar) return;

    if (scrollAnim_ && scrollAnim_->state() == QAbstractAnimation::Running)
        scrollAnim_->stop();

    scrollAnim_ = new QPropertyAnimation(bar, "value");
    scrollAnim_->setDuration(duration);
    scrollAnim_->setStartValue(bar->value());
    scrollAnim_->setEndValue(qBound(bar->minimum(), value, bar->maximum()));
    scrollAnim_->setEasingCurve(QEasingCurve::InOutQuad);
    scrollAnim_->start(QAbstractAnimation::DeleteWhenStopped);
}

void SmoothScrollArea::wheelEvent(QWheelEvent* event)
{
    if (smooth_) {
        int delta = event->angleDelta().y();
        if (delta == 0) {
            QScrollArea::wheelEvent(event);
            return;
        }

        QScrollBar* bar = verticalScrollBar();
        if (!bar) {
            QScrollArea::wheelEvent(event);
            return;
        }

        int step = qRound(delta / 8.0 / 15.0 * 10); // finer step
        int newValue = bar->value() + step;

        smoothScrollTo(newValue, 200);
        event->accept();
    } else {
        QScrollArea::wheelEvent(event);
    }
}

} // namespace easy

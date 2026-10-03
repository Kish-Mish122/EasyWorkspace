#pragma once

#include <QPropertyAnimation>
#include <QScrollArea>
#include <QWheelEvent>
#include <QWidget>

namespace easy {

class SmoothScrollArea : public QScrollArea {
    Q_OBJECT

public:
    explicit SmoothScrollArea(QWidget* parent = nullptr);

    void setSmoothScroll(bool enabled);
    void smoothScrollTo(int value, int duration = 300);

protected:
    void wheelEvent(QWheelEvent* event) override;

private:
    QPropertyAnimation* scrollAnim_ = nullptr;
    bool smooth_ = true;
};

} // namespace easy

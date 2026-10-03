#pragma once
#include "Slide.h"

#include <QWidget>

// Полноэкранный показ слайдов: стрелки/пробел/клик — листать, Esc — выход.
class SlideShowWindow : public QWidget {
    Q_OBJECT
public:
    SlideShowWindow(const QVector<Slide>& slides, int startIndex, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void go(int index);

    QVector<Slide> slides_;
    int index_ = 0;
};

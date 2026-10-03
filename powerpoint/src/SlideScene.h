#pragma once
#include "Slide.h"

#include <QGraphicsScene>

class SlideItem;

class SlideScene : public QGraphicsScene {
    Q_OBJECT
public:
    static constexpr qreal kWidth = 960;
    static constexpr qreal kHeight = 540;

    explicit SlideScene(QObject* parent = nullptr);

    void load(const Slide& slide);          // заменяет содержимое сцены; сигнал edited не посылает
    Slide collect() const;                  // собирает слайд из текущего состояния сцены
    SlideItem* addSlideItem(const ItemData& data);
    QList<SlideItem*> selectedSlideItems() const;
    void bringToFront(SlideItem* item);

    QColor background() const { return bg_; }
    void setBackground(const QColor& color);

    // Рисует слайд в заданный прямоугольник (миниатюры, PDF, показ).
    static void paintSlide(QPainter* painter, const Slide& slide, const QRectF& target);

signals:
    void edited();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
    QColor bg_ = Qt::white;
};

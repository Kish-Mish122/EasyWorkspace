#pragma once
#include "Slide.h"

#include <QGraphicsItem>
#include <functional>

// Графический элемент слайда: перетаскивается мышью, размер меняется за правый нижний угол,
// двойной щелчок — редактирование текста.
class SlideItem : public QGraphicsItem {
public:
    SlideItem(const ItemData& data, std::function<void()> onEdited);

    ItemData data() const;               // с актуальной позицией
    void setItemData(const ItemData& d); // полная замена содержимого/оформления

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QRectF handleRect() const;
    void notify() const;

    ItemData d_;
    bool resizing_ = false;
    std::function<void()> onEdited_;
};

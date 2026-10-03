#include "SlideScene.h"

#include "SlideItem.h"

#include <QPainter>
#include <algorithm>

SlideScene::SlideScene(QObject* parent) : QGraphicsScene(parent)
{
    setSceneRect(0, 0, kWidth, kHeight);
}

void SlideScene::load(const Slide& slide)
{
    clear();
    bg_ = slide.background;
    qreal z = 0;
    for (const ItemData& d : slide.items) {
        SlideItem* it = addSlideItem(d);
        it->setZValue(z++);
    }
    invalidate(sceneRect(), BackgroundLayer);
}

SlideItem* SlideScene::addSlideItem(const ItemData& data)
{
    auto* item = new SlideItem(data, [this] { emit edited(); });
    qreal maxZ = -1;
    for (QGraphicsItem* other : items())
        maxZ = std::max(maxZ, other->zValue());
    item->setZValue(maxZ + 1);
    QGraphicsScene::addItem(item);
    return item;
}

Slide SlideScene::collect() const
{
    Slide s;
    s.background = bg_;
    QList<QGraphicsItem*> all = items();
    std::sort(all.begin(), all.end(), [](QGraphicsItem* a, QGraphicsItem* b) { return a->zValue() < b->zValue(); });
    for (QGraphicsItem* gi : std::as_const(all))
        if (auto* it = dynamic_cast<SlideItem*>(gi))
            s.items.push_back(it->data());
    return s;
}

QList<SlideItem*> SlideScene::selectedSlideItems() const
{
    QList<SlideItem*> out;
    for (QGraphicsItem* gi : selectedItems())
        if (auto* it = dynamic_cast<SlideItem*>(gi))
            out.push_back(it);
    return out;
}

void SlideScene::bringToFront(SlideItem* item)
{
    qreal maxZ = 0;
    for (QGraphicsItem* other : items())
        maxZ = std::max(maxZ, other->zValue());
    item->setZValue(maxZ + 1);
    emit edited();
}

void SlideScene::setBackground(const QColor& color)
{
    bg_ = color;
    invalidate(sceneRect(), BackgroundLayer);
    emit edited();
}

void SlideScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->fillRect(rect, QColor(0x9c, 0xa3, 0xaf)); // серое поле вокруг слайда
    painter->fillRect(sceneRect(), bg_);
}

void SlideScene::paintSlide(QPainter* painter, const Slide& slide, const QRectF& target)
{
    SlideScene scene;
    scene.load(slide);
    painter->setRenderHint(QPainter::Antialiasing);
    scene.render(painter, target, QRectF(0, 0, kWidth, kHeight), Qt::KeepAspectRatio);
}

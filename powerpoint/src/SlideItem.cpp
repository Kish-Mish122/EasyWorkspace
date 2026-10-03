#include "SlideItem.h"

#include <QApplication>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QPainter>
#include <algorithm>

SlideItem::SlideItem(const ItemData& data, std::function<void()> onEdited)
    : d_(data), onEdited_(std::move(onEdited))
{
    setPos(d_.rect.topLeft());
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
}

ItemData SlideItem::data() const
{
    ItemData d = d_;
    d.rect = QRectF(pos(), d_.rect.size());
    return d;
}

void SlideItem::setItemData(const ItemData& d)
{
    prepareGeometryChange();
    d_ = d;
    update();
    notify();
}

QRectF SlideItem::boundingRect() const
{
    return QRectF(-2, -2, d_.rect.width() + 4, d_.rect.height() + 4);
}

QRectF SlideItem::handleRect() const
{
    return QRectF(d_.rect.width() - 14, d_.rect.height() - 14, 14, 14);
}

void SlideItem::notify() const
{
    if (onEdited_)
        onEdited_();
}

void SlideItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
    const QRectF r(0, 0, d_.rect.width(), d_.rect.height());
    p->setRenderHint(QPainter::Antialiasing);

    p->setPen(Qt::NoPen);
    p->setBrush(d_.fill);
    if (d_.type == ItemData::Ellipse)
        p->drawEllipse(r);
    else
        p->drawRect(r);

    if (!d_.text.isEmpty()) {
        QFont f = QApplication::font();
        f.setPixelSize(d_.fontSize);
        f.setBold(d_.bold);
        p->setFont(f);
        p->setPen(d_.textColor);
        const int align = d_.type == ItemData::Text ? (Qt::AlignLeft | Qt::AlignVCenter) : Qt::AlignCenter;
        p->drawText(r.adjusted(10, 6, -10, -6), align | Qt::TextWordWrap, d_.text);
    }

    if (isSelected()) {
        QPen pen(QColor(0x25, 0x63, 0xeb), 1.5, Qt::DashLine);
        pen.setCosmetic(true);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        p->drawRect(r);
        p->setPen(Qt::NoPen);
        p->setBrush(QColor(0x25, 0x63, 0xeb));
        p->drawRect(handleRect());
    }
}

QVariant SlideItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionHasChanged)
        notify();
    return QGraphicsItem::itemChange(change, value);
}

void SlideItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && isSelected() && handleRect().contains(e->pos())) {
        resizing_ = true;
        e->accept();
        return;
    }
    QGraphicsItem::mousePressEvent(e);
}

void SlideItem::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
{
    if (resizing_) {
        prepareGeometryChange();
        d_.rect.setSize(QSizeF(std::max<qreal>(30, e->pos().x()), std::max<qreal>(30, e->pos().y())));
        update();
        notify();
        return;
    }
    QGraphicsItem::mouseMoveEvent(e);
}

void SlideItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    if (resizing_) {
        resizing_ = false;
        e->accept();
        return;
    }
    QGraphicsItem::mouseReleaseEvent(e);
}

void SlideItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    bool ok = false;
    const QString t = QInputDialog::getMultiLineText(nullptr, QStringLiteral("Текст"),
                                                     QStringLiteral("Содержимое:"), d_.text, &ok);
    if (ok) {
        d_.text = t;
        update();
        notify();
    }
}

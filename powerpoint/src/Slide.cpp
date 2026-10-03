#include "Slide.h"

#include <QJsonArray>

QJsonObject ItemData::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral("type"), int(type));
    o.insert(QStringLiteral("x"), rect.x());
    o.insert(QStringLiteral("y"), rect.y());
    o.insert(QStringLiteral("w"), rect.width());
    o.insert(QStringLiteral("h"), rect.height());
    o.insert(QStringLiteral("text"), text);
    o.insert(QStringLiteral("fill"), fill.name(QColor::HexArgb));
    o.insert(QStringLiteral("textColor"), textColor.name(QColor::HexArgb));
    o.insert(QStringLiteral("fontSize"), fontSize);
    o.insert(QStringLiteral("bold"), bold);
    return o;
}

ItemData ItemData::fromJson(const QJsonObject& o)
{
    ItemData d;
    const int t = o.value(QStringLiteral("type")).toInt();
    d.type = (t >= Text && t <= Ellipse) ? static_cast<Type>(t) : Text;
    d.rect = QRectF(o.value(QStringLiteral("x")).toDouble(), o.value(QStringLiteral("y")).toDouble(),
                    o.value(QStringLiteral("w")).toDouble(100), o.value(QStringLiteral("h")).toDouble(50));
    d.text = o.value(QStringLiteral("text")).toString();
    d.fill = QColor(o.value(QStringLiteral("fill")).toString());
    d.textColor = QColor(o.value(QStringLiteral("textColor")).toString());
    if (!d.textColor.isValid())
        d.textColor = QColor(0x11, 0x18, 0x27);
    d.fontSize = o.value(QStringLiteral("fontSize")).toInt(28);
    d.bold = o.value(QStringLiteral("bold")).toBool();
    return d;
}

QJsonObject Slide::toJson() const
{
    QJsonArray arr;
    for (const ItemData& it : items)
        arr.append(it.toJson());
    QJsonObject o;
    o.insert(QStringLiteral("background"), background.name());
    o.insert(QStringLiteral("items"), arr);
    return o;
}

Slide Slide::fromJson(const QJsonObject& o)
{
    Slide s;
    s.background = QColor(o.value(QStringLiteral("background")).toString());
    if (!s.background.isValid())
        s.background = Qt::white;
    const QJsonArray arr = o.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& v : arr)
        s.items.push_back(ItemData::fromJson(v.toObject()));
    return s;
}

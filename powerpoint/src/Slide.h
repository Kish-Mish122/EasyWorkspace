#pragma once
#include <QColor>
#include <QJsonObject>
#include <QRectF>
#include <QString>
#include <QVector>

// Один объект на слайде: текстовое поле, прямоугольник или эллипс.
struct ItemData {
    enum Type { Text = 0, Rect = 1, Ellipse = 2 };

    Type type = Text;
    QRectF rect{100, 100, 300, 80}; // позиция и размер в координатах слайда 960x540
    QString text;
    QColor fill = Qt::transparent;
    QColor textColor = QColor(0x11, 0x18, 0x27);
    int fontSize = 28; // в пикселях слайда
    bool bold = false;

    QJsonObject toJson() const;
    static ItemData fromJson(const QJsonObject& obj);
};

struct Slide {
    QColor background = Qt::white;
    QVector<ItemData> items; // порядок = порядок наложения (последний сверху)

    QJsonObject toJson() const;
    static Slide fromJson(const QJsonObject& obj);
};

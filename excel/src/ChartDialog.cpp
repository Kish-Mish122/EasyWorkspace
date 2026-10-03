#include "ChartDialog.h"

#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

namespace {
const QList<QColor>& palette()
{
    static const QList<QColor> colors = {
        QColor(0x25, 0x63, 0xeb), QColor(0x16, 0xa3, 0x4a), QColor(0xf5, 0x9e, 0x0b), QColor(0xdc, 0x26, 0x26),
        QColor(0x7c, 0x3a, 0xed), QColor(0x08, 0x91, 0xb2), QColor(0xdb, 0x27, 0x77), QColor(0x65, 0xa3, 0x0d)};
    return colors;
}
} // namespace

ChartWidget::ChartWidget(const QStringList& labels, const QVector<double>& values, QWidget* parent)
    : QWidget(parent), labels_(labels), values_(values)
{
    setMinimumSize(560, 360);
}

void ChartWidget::setType(Type type)
{
    type_ = type;
    update();
}

void ChartWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);
    if (values_.isEmpty())
        return;
    if (type_ == Pie)
        paintPie(p);
    else
        paintAxes(p, type_ == Line);
}

void ChartWidget::paintAxes(QPainter& p, bool line)
{
    const QRectF plot(64, 20, width() - 84, height() - 70);
    double mn = std::min(0.0, *std::min_element(values_.begin(), values_.end()));
    double mx = std::max(0.0, *std::max_element(values_.begin(), values_.end()));
    if (mx == mn)
        mx = mn + 1.0;

    auto yOf = [&](double v) { return plot.bottom() - (v - mn) / (mx - mn) * plot.height(); };

    for (int i = 0; i <= 5; ++i) {
        const double v = mn + (mx - mn) * i / 5.0;
        const double y = yOf(v);
        p.setPen(QColor(0xe5, 0xe7, 0xeb));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor(0x37, 0x41, 0x51));
        p.drawText(QRectF(0, y - 9, plot.left() - 8, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(v, 'g', 4));
    }

    const double step = plot.width() / values_.size();
    const double zeroY = yOf(0.0);

    if (line) {
        QPolygonF poly;
        for (int i = 0; i < values_.size(); ++i)
            poly << QPointF(plot.left() + step * (i + 0.5), yOf(values_[i]));
        p.setPen(QPen(::palette()[0], 2.5));
        p.drawPolyline(poly);
        p.setBrush(::palette()[0]);
        for (const QPointF& pt : poly)
            p.drawEllipse(pt, 4, 4);
    } else {
        p.setPen(Qt::NoPen);
        for (int i = 0; i < values_.size(); ++i) {
            const double bw = step * 0.6;
            const double y = yOf(values_[i]);
            p.setBrush(::palette()[i % ::palette().size()]);
            p.drawRect(QRectF(plot.left() + step * i + (step - bw) / 2, std::min(y, zeroY), bw, std::abs(y - zeroY)));
        }
    }

    p.setPen(QColor(0x37, 0x41, 0x51));
    p.drawLine(QPointF(plot.left(), zeroY), QPointF(plot.right(), zeroY));
    for (int i = 0; i < values_.size(); ++i) {
        const QString text = fontMetrics().elidedText(labels_.value(i), Qt::ElideRight, int(step) - 4);
        p.drawText(QRectF(plot.left() + step * i, plot.bottom() + 8, step, 20), Qt::AlignHCenter | Qt::AlignTop, text);
    }
}

void ChartWidget::paintPie(QPainter& p)
{
    double total = 0;
    for (double v : std::as_const(values_))
        total += std::abs(v);
    if (total == 0.0)
        return;

    const double side = std::min(width() * 0.6, height() - 40.0);
    const QRectF pie(20, (height() - side) / 2, side, side);

    int start = 90 * 16;
    p.setPen(Qt::white);
    for (int i = 0; i < values_.size(); ++i) {
        const int span = int(-std::abs(values_[i]) / total * 360.0 * 16.0);
        p.setBrush(::palette()[i % ::palette().size()]);
        p.drawPie(pie, start, span);
        start += span;
    }

    double y = 30;
    const double x = pie.right() + 30;
    for (int i = 0; i < values_.size(); ++i) {
        p.setPen(Qt::NoPen);
         p.setBrush(::palette()[i % ::palette().size()]);
        p.drawRect(QRectF(x, y, 14, 14));
        p.setPen(QColor(0x37, 0x41, 0x51));
        const QString text = QStringLiteral("%1 — %2%")
                                 .arg(labels_.value(i))
                                 .arg(std::abs(values_[i]) / total * 100.0, 0, 'f', 1);
        p.drawText(QPointF(x + 22, y + 12), text);
        y += 24;
    }
}

ChartDialog::ChartDialog(const QStringList& labels, const QVector<double>& values, QWidget* parent)
    : QDialog(parent), chart_(new ChartWidget(labels, values))
{
    setWindowTitle(QStringLiteral("Диаграмма"));

    auto* typeBox = new QComboBox;
    typeBox->addItems({QStringLiteral("Столбцы"), QStringLiteral("Линия"), QStringLiteral("Круговая")});
    connect(typeBox, &QComboBox::currentIndexChanged, this,
            [this](int i) { chart_->setType(static_cast<ChartWidget::Type>(i)); });

    auto* saveBtn = new QPushButton(QStringLiteral("Сохранить как PNG…"));
    connect(saveBtn, &QPushButton::clicked, this, [this] {
        QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить диаграмму"), {},
                                                    QStringLiteral("PNG (*.png)"));
        if (path.isEmpty())
            return;
        if (!path.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive))
            path += QStringLiteral(".png");
        if (!chart_->grab().save(path))
            QMessageBox::warning(this, windowTitle(), QStringLiteral("Не удалось сохранить файл."));
    });

    auto* top = new QHBoxLayout;
    top->addWidget(new QLabel(QStringLiteral("Тип:")));
    top->addWidget(typeBox);
    top->addStretch();
    top->addWidget(saveBtn);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(top);
    layout->addWidget(chart_, 1);
}

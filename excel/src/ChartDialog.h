#pragma once
#include <QDialog>
#include <QStringList>
#include <QVector>
#include <QWidget>

// Простая диаграмма (столбцы / линия / круговая), нарисованная вручную через QPainter.
class ChartWidget : public QWidget {
    Q_OBJECT
public:
    enum Type { Bar, Line, Pie };

    ChartWidget(const QStringList& labels, const QVector<double>& values, QWidget* parent = nullptr);
    void setType(Type type);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paintAxes(QPainter& p, bool line);
    void paintPie(QPainter& p);

    QStringList labels_;
    QVector<double> values_;
    Type type_ = Bar;
};

class ChartDialog : public QDialog {
    Q_OBJECT
public:
    ChartDialog(const QStringList& labels, const QVector<double>& values, QWidget* parent = nullptr);

private:
    ChartWidget* chart_;
};

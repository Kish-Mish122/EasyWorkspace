#pragma once
#include <QAbstractTableModel>
#include <QColor>
#include <QHash>
#include <QJsonObject>
#include <QSet>

// Модель электронной таблицы: хранит «сырой» текст ячеек, вычисляет формулы, форматирование.
class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT
public:
    static constexpr int kRows = 500;
    static constexpr int kCols = 52;

    explicit SpreadsheetModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& = {}) const override { return kRows; }
    int columnCount(const QModelIndex& = {}) const override { return kCols; }
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QString rawText(int row, int col) const;     // то, что ввёл пользователь (формула или значение)
    QString displayText(int row, int col) const; // то, что видно в ячейке
    bool numberAt(int row, int col, double& out) const;

    bool isBold(int row, int col) const;
    void setBold(const QModelIndexList& cells, bool bold);
    void setFill(const QModelIndexList& cells, const QColor& color); // невалидный цвет = убрать заливку
    void setFontSize(const QModelIndexList& cells, int size);
    void clearCells(const QModelIndexList& cells);
    void clearAll();

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject& obj);
    QString toCsv() const;
    void fromCsv(const QString& text);

private:
    struct Value {
        bool isNumber = false;
        bool isError = false;
        double number = 0.0;
        QString text;
    };
    struct CellFormat {
        bool bold = false;
        QColor fill;
        int fontSize = 12;
    };

    static quint64 key(int row, int col) { return (quint64(row) << 32) | quint32(col); }
    static int rowOf(quint64 k) { return int(k >> 32); }
    static int colOf(quint64 k) { return int(k & 0xffffffffu); }

    Value value(int row, int col) const;
    void invalidate();

    QHash<quint64, QString> raw_;
    QHash<quint64, CellFormat> formats_;
    mutable QHash<quint64, Value> cache_;
    mutable QSet<quint64> evaluating_;
};

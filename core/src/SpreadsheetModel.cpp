#include "SpreadsheetModel.h"
#include "FormulaEngine.h"

#include <QFont>
#include <QJsonArray>
#include <QStringList>
#include <algorithm>
#include <QBrush>

SpreadsheetModel::SpreadsheetModel(QObject* parent) : QAbstractTableModel(parent) {}

SpreadsheetModel::Value SpreadsheetModel::value(int row, int col) const
{
    const quint64 k = key(row, col);
    if (const auto it = cache_.constFind(k); it != cache_.constEnd())
        return it.value();

    Value v;
    const QString raw = raw_.value(k);
    if (raw.isEmpty())
        return v;

    if (raw.startsWith('=')) {
        if (evaluating_.contains(k)) {
            v.isError = true;
            v.text = QStringLiteral("#CYCLE!");
            return v; // в кэш не кладём
        }
        evaluating_.insert(k);
        try {
            v.number = FormulaEngine::evaluate(raw.mid(1), [this](int r, int c) -> std::optional<double> {
                if (r < 0 || c < 0 || r >= kRows || c >= kCols)
                    throw FormulaError{QStringLiteral("#REF!")};
                const Value x = value(r, c);
                if (x.isError)
                    throw FormulaError{x.text};
                if (x.isNumber)
                    return x.number;
                return std::nullopt;
            });
            v.isNumber = true;
            v.text = QString::number(v.number, 'g', 10);
        } catch (const FormulaError& e) {
            v.isError = true;
            v.text = e.message;
        }
        evaluating_.remove(k);
    } else {
        bool ok = false;
        const double d = raw.toDouble(&ok);
        if (ok) {
            v.isNumber = true;
            v.number = d;
        }
        v.text = raw;
    }
    cache_.insert(k, v);
    return v;
}

void SpreadsheetModel::invalidate()
{
    cache_.clear();
    emit dataChanged(index(0, 0), index(kRows - 1, kCols - 1));
}

QVariant SpreadsheetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};
    const int r = index.row(), c = index.column();
    switch (role) {
    case Qt::DisplayRole:
        return value(r, c).text;
    case Qt::EditRole:
        return raw_.value(key(r, c));
    case Qt::TextAlignmentRole: {
        const Value v = value(r, c);
        if (v.isError)
            return int(Qt::AlignCenter);
        return int((v.isNumber ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter);
    }
    case Qt::ForegroundRole:
        if (value(r, c).isError)
            return QColor(0xdc, 0x26, 0x26);
        return {};
    case Qt::FontRole:
        if (isBold(r, c)) {
            QFont f;
            f.setBold(true);
            return f;
        }
        // Apply font size
        if (const auto it = formats_.constFind(key(r, c)); it != formats_.constEnd() && it->fontSize != 12) {
            QFont f;
            f.setPointSize(it->fontSize);
            return f;
        }
        return {};
    case Qt::BackgroundRole:
        if (const auto it = formats_.constFind(key(r, c)); it != formats_.constEnd() && it->fill.isValid())
            return QBrush(it->fill);
        return {};
    default:
        return {};
    }
}

bool SpreadsheetModel::setData(const QModelIndex& index, const QVariant& val, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;
    const quint64 k = key(index.row(), index.column());
    const QString text = val.toString();
    if (text.isEmpty())
        raw_.remove(k);
    else
        raw_.insert(k, text);
    invalidate();
    return true;
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation o, int role) const
{
    if (role != Qt::DisplayRole)
        return {};
    return o == Qt::Horizontal ? FormulaEngine::colName(section) : QString::number(section + 1);
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

QString SpreadsheetModel::rawText(int row, int col) const
{
    return raw_.value(key(row, col));
}

QString SpreadsheetModel::displayText(int row, int col) const
{
    return value(row, col).text;
}

bool SpreadsheetModel::numberAt(int row, int col, double& out) const
{
    const Value v = value(row, col);
    if (!v.isNumber)
        return false;
    out = v.number;
    return true;
}

bool SpreadsheetModel::isBold(int row, int col) const
{
    const auto it = formats_.constFind(key(row, col));
    return it != formats_.constEnd() && it->bold;
}

void SpreadsheetModel::setBold(const QModelIndexList& cells, bool bold)
{
    for (const QModelIndex& i : cells)
        formats_[key(i.row(), i.column())].bold = bold;
    invalidate();
}

void SpreadsheetModel::setFontSize(const QModelIndexList& cells, int size)
{
    for (const QModelIndex& i : cells)
        formats_[key(i.row(), i.column())].fontSize = size;
    invalidate();
}

void SpreadsheetModel::setFill(const QModelIndexList& cells, const QColor& color)
{
    for (const QModelIndex& i : cells)
        formats_[key(i.row(), i.column())].fill = color;
    invalidate();
}

void SpreadsheetModel::clearCells(const QModelIndexList& cells)
{
    for (const QModelIndex& i : cells)
        raw_.remove(key(i.row(), i.column()));
    invalidate();
}

void SpreadsheetModel::clearAll()
{
    beginResetModel();
    raw_.clear();
    formats_.clear();
    cache_.clear();
    endResetModel();
}

QJsonObject SpreadsheetModel::toJson() const
{
    QJsonObject cells;
    for (auto it = raw_.constBegin(); it != raw_.constEnd(); ++it)
        cells.insert(FormulaEngine::cellName(rowOf(it.key()), colOf(it.key())), it.value());

    QJsonObject formats;
    for (auto it = formats_.constBegin(); it != formats_.constEnd(); ++it) {
        if (!it->bold && !it->fill.isValid())
            continue;
        QJsonObject f;
        if (it->bold)
            f.insert(QStringLiteral("bold"), true);
        if (it->fill.isValid())
            f.insert(QStringLiteral("fill"), it->fill.name());
        formats.insert(FormulaEngine::cellName(rowOf(it.key()), colOf(it.key())), f);
    }

    QJsonObject root;
    root.insert(QStringLiteral("format"), QStringLiteral("ezx"));
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("cells"), cells);
    root.insert(QStringLiteral("formats"), formats);
    return root;
}

bool SpreadsheetModel::fromJson(const QJsonObject& obj)
{
    if (obj.value(QStringLiteral("format")).toString() != QLatin1String("ezx"))
        return false;

    beginResetModel();
    raw_.clear();
    formats_.clear();
    cache_.clear();

    const QJsonObject cells = obj.value(QStringLiteral("cells")).toObject();
    for (auto it = cells.begin(); it != cells.end(); ++it) {
        int r = 0, c = 0;
        if (FormulaEngine::parseRef(it.key(), r, c) && r < kRows && c < kCols)
            raw_.insert(key(r, c), it.value().toString());
    }
    const QJsonObject formats = obj.value(QStringLiteral("formats")).toObject();
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        int r = 0, c = 0;
        if (!FormulaEngine::parseRef(it.key(), r, c) || r >= kRows || c >= kCols)
            continue;
        const QJsonObject f = it.value().toObject();
        CellFormat cf;
        cf.bold = f.value(QStringLiteral("bold")).toBool();
        if (f.contains(QStringLiteral("fill")))
            cf.fill = QColor(f.value(QStringLiteral("fill")).toString());
        formats_.insert(key(r, c), cf);
    }
    endResetModel();
    return true;
}

QString SpreadsheetModel::toCsv() const
{
    int maxR = -1, maxC = -1;
    for (auto it = raw_.constBegin(); it != raw_.constEnd(); ++it) {
        maxR = std::max(maxR, rowOf(it.key()));
        maxC = std::max(maxC, colOf(it.key()));
    }
    QString out;
    for (int r = 0; r <= maxR; ++r) {
        QStringList line;
        for (int c = 0; c <= maxC; ++c) {
            QString f = displayText(r, c);
            if (f.contains(',') || f.contains('"') || f.contains('\n'))
                f = QLatin1Char('"') + f.replace('"', QStringLiteral("\"\"")) + QLatin1Char('"');
            line << f;
        }
        out += line.join(',') + QLatin1Char('\n');
    }
    return out;
}

void SpreadsheetModel::fromCsv(const QString& text)
{
    beginResetModel();
    raw_.clear();
    formats_.clear();
    cache_.clear();

    const QString firstLine = text.section('\n', 0, 0);
    QChar delim = ',';
    if (!firstLine.contains(','))
        delim = firstLine.contains(';') ? QChar(';') : (firstLine.contains('\t') ? QChar('\t') : QChar(','));

    int row = 0, col = 0;
    QString field;
    bool inQuotes = false;

    auto endField = [&] {
        if (row < kRows && col < kCols && !field.isEmpty())
            raw_.insert(key(row, col), field);
        field.clear();
        ++col;
    };
    auto endRow = [&] {
        endField();
        col = 0;
        ++row;
    };

    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += ch;
            }
        } else if (ch == '"') {
            inQuotes = true;
        } else if (ch == delim) {
            endField();
        } else if (ch == '\n') {
            endRow();
        } else if (ch != '\r') {
            field += ch;
        }
    }
    if (!field.isEmpty() || col > 0)
        endRow();

    endResetModel();
}

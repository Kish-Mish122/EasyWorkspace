#pragma once
#include <QString>
#include <functional>
#include <optional>

// Исключение вычисления формулы: в ячейке показывается message (#DIV/0!, #NAME? ...).
struct FormulaError {
    QString message;
};

class FormulaEngine {
public:
    // Возвращает число для ячейки или std::nullopt, если ячейка пуста / содержит текст.
    using Resolver = std::function<std::optional<double>(int row, int col)>;

    // Вычисляет выражение без ведущего '='. Бросает FormulaError.
    // Поддержка: + - * / ^, скобки, унарный минус, ссылки A1, диапазоны A1:B5 внутри функций,
    // функции SUM, AVG/AVERAGE, MIN, MAX, COUNT (а также СУММ, СРЗНАЧ, МИН, МАКС, СЧЁТ).
    static double evaluate(const QString& expression, const Resolver& resolver);

    static bool parseRef(const QString& text, int& row, int& col); // "B3" -> row=2, col=1
    static QString colName(int col);                               // 0 -> "A", 27 -> "AB"
    static QString cellName(int row, int col);                     // (2,1) -> "B3"
};

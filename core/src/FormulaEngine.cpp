#include "FormulaEngine.h"

#include <QVector>
#include <algorithm>
#include <cmath>

namespace {

class Parser {
public:
    Parser(const QString& s, const FormulaEngine::Resolver& r) : s_(s), r_(r) {}

    double parse()
    {
        const double v = expr();
        skip();
        if (pos_ < s_.size())
            throw FormulaError{QStringLiteral("#SYNTAX!")};
        return v;
    }

private:
    QString s_;
    const FormulaEngine::Resolver& r_;
    int pos_ = 0;

    void skip()
    {
        while (pos_ < s_.size() && s_[pos_].isSpace())
            ++pos_;
    }

    bool eat(QChar c)
    {
        skip();
        if (pos_ < s_.size() && s_[pos_] == c) {
            ++pos_;
            return true;
        }
        return false;
    }

    double expr()
    {
        double v = term();
        for (;;) {
            if (eat('+'))
                v += term();
            else if (eat('-'))
                v -= term();
            else
                return v;
        }
    }

    double term()
    {
        double v = unary();
        for (;;) {
            if (eat('*')) {
                v *= unary();
            } else if (eat('/')) {
                const double d = unary();
                if (d == 0.0)
                    throw FormulaError{QStringLiteral("#DIV/0!")};
                v /= d;
            } else {
                return v;
            }
        }
    }

    double unary()
    {
        if (eat('-'))
            return -unary();
        if (eat('+'))
            return unary();
        return power();
    }

    double power()
    {
        const double b = primary();
        if (eat('^'))
            return std::pow(b, unary());
        return b;
    }

    double primary()
    {
        skip();
        if (eat('(')) {
            const double v = expr();
            if (!eat(')'))
                throw FormulaError{QStringLiteral("#SYNTAX!")};
            return v;
        }
        if (pos_ >= s_.size())
            throw FormulaError{QStringLiteral("#SYNTAX!")};
        if (s_[pos_].isDigit() || s_[pos_] == '.')
            return number();
        if (s_[pos_].isLetter())
            return identifier();
        throw FormulaError{QStringLiteral("#SYNTAX!")};
    }

    double number()
    {
        const int st = pos_;
        while (pos_ < s_.size() && (s_[pos_].isDigit() || s_[pos_] == '.'))
            ++pos_;
        bool ok = false;
        const double v = s_.mid(st, pos_ - st).toDouble(&ok);
        if (!ok)
            throw FormulaError{QStringLiteral("#SYNTAX!")};
        return v;
    }

    QString word()
    {
        const int st = pos_;
        while (pos_ < s_.size() && s_[pos_].isLetterOrNumber())
            ++pos_;
        return s_.mid(st, pos_ - st).toUpper();
    }

    double identifier()
    {
        const QString name = word();
        skip();
        if (pos_ < s_.size() && s_[pos_] == '(') {
            ++pos_;
            return function(name);
        }
        int r = 0, c = 0;
        if (!FormulaEngine::parseRef(name, r, c))
            throw FormulaError{QStringLiteral("#NAME?")};
        skip();
        if (pos_ < s_.size() && s_[pos_] == ':')
            throw FormulaError{QStringLiteral("#RANGE!")};
        return r_(r, c).value_or(0.0);
    }

    void collectArg(QVector<double>& out)
    {
        skip();
        const int save = pos_;
        if (pos_ < s_.size() && s_[pos_].isLetter()) {
            const QString a = word();
            int r1 = 0, c1 = 0;
            if (FormulaEngine::parseRef(a, r1, c1) && eat(':')) {
                skip();
                const QString b = word();
                int r2 = 0, c2 = 0;
                if (!FormulaEngine::parseRef(b, r2, c2))
                    throw FormulaError{QStringLiteral("#REF!")};
                for (int r = std::min(r1, r2); r <= std::max(r1, r2); ++r)
                    for (int c = std::min(c1, c2); c <= std::max(c1, c2); ++c)
                        if (const auto v = r_(r, c))
                            out.push_back(*v);
                return;
            }
            pos_ = save;
        }
        out.push_back(expr());
    }

    double function(const QString& name)
    {
        QVector<double> v;
        if (!eat(')')) {
            for (;;) {
                collectArg(v);
                if (eat(',') || eat(';'))
                    continue;
                if (eat(')'))
                    break;
                throw FormulaError{QStringLiteral("#SYNTAX!")};
            }
        }
        double sum = 0;
        for (double x : v)
            sum += x;

        if (name == "SUM" || name == QStringLiteral("СУММ"))
            return sum;
        if (name == "AVG" || name == "AVERAGE" || name == QStringLiteral("СРЗНАЧ"))
            return v.isEmpty() ? 0.0 : sum / v.size();
        if (name == "MIN" || name == QStringLiteral("МИН"))
            return v.isEmpty() ? 0.0 : *std::min_element(v.begin(), v.end());
        if (name == "MAX" || name == QStringLiteral("МАКС"))
            return v.isEmpty() ? 0.0 : *std::max_element(v.begin(), v.end());
        if (name == "COUNT" || name == QStringLiteral("СЧЁТ"))
            return v.size();
        throw FormulaError{QStringLiteral("#NAME?")};
    }
};

} // namespace

double FormulaEngine::evaluate(const QString& expression, const Resolver& resolver)
{
    return Parser(expression, resolver).parse();
}

bool FormulaEngine::parseRef(const QString& s, int& row, int& col)
{
    int i = 0;
    int c = 0;
    while (i < s.size() && s[i].unicode() < 128 && s[i].isLetter()) {
        c = c * 26 + (s[i].toUpper().unicode() - 'A' + 1);
        ++i;
    }
    if (i == 0 || i == s.size())
        return false;
    bool ok = false;
    const int r = s.mid(i).toInt(&ok);
    if (!ok || r < 1)
        return false;
    col = c - 1;
    row = r - 1;
    return true;
}

QString FormulaEngine::colName(int col)
{
    QString s;
    int n = col + 1;
    while (n > 0) {
        const int rem = (n - 1) % 26;
        s.prepend(QChar('A' + rem));
        n = (n - 1) / 26;
    }
    return s;
}

QString FormulaEngine::cellName(int row, int col)
{
    return colName(col) + QString::number(row + 1);
}

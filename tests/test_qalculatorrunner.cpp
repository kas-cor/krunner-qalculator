/*
 *   Copyright (C) 2025 Your Name
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <libqalculate/Calculator.h>
#include <libqalculate/includes.h>

#include <algorithm>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTest>

namespace {

// Helper: parse a locale-aware numeric result (handles both "3.14" and "3,14")
double parseNumeric(const QString &str, bool *ok = nullptr)
{
    // libqalculate respects system locale — handle both "." and "," decimal separators
    QString normalized = str;
    if (normalized.contains(QLatin1Char(',')) && !normalized.contains(QLatin1Char('.')))
    {
        normalized.replace(QLatin1Char(','), QLatin1Char('.'));
    }
    return normalized.toDouble(ok);
}

} // anonymous namespace

/**
 * Tests the libqalculate C++ API directly, mirroring the logic used by
 * QalculatorRunner::calculate() but without requiring a full KDE session.
 *
 * Exchange rates are NOT loaded in initTestCase (network-dependent). The
 * dedicated testCurrencyConversion() calls loadExchangeRates() and skips
 * itself gracefully when no network is available.
 */
class TestQalculatorRunner : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testBasicCalculation();
    void testComplexExpression();
    void testUnitConversion();
    void testCurrencyConversion();
    void testE2eWithTrigger();
    void testInputEqualsResult();
    void testErrorHandling();
    void testExpressionUnlocalization();

private:
    QString calculate(const QString &term);
    QString resultNoRepeat(const QString &term, const QString &result);

    // Timeout for calculations (mirrors QalculatorRunner)
    static constexpr int TIMEOUT_MS = 2000;
};

void TestQalculatorRunner::initTestCase()
{
    // Initialize the Calculator singleton just like QalculatorRunner does
    // loadExchangeRates() is skipped — it requires network and slows down tests
    if (!CALCULATOR)
    {
        CALCULATOR = new Calculator();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
    }
    QVERIFY2(CALCULATOR != nullptr, "Calculator singleton should be initialized");
}

QString TestQalculatorRunner::calculate(const QString &term)
{
    std::string expr = CALCULATOR->unlocalizeExpression(term.toStdString());

    // Mirror the currency-code normalization from QalculatorRunner::calculate():
    // uppercase 3-letter alphabetic tokens in a conversion pair
    // (NUMBER TOKEN1 to/in TOKEN2) — targets ISO 4217 currency codes
    // while skipping 2-letter units (kg, lb, cm) and compound units (km/h).
    {
        const QStringList tokens = QString::fromStdString(expr).split(QLatin1Char(' '), Qt::SkipEmptyParts);
        QStringList normalized;
        normalized.reserve(tokens.size());

        for (qsizetype i = 0; i < tokens.size(); ++i)
        {
            if (i + 3 < tokens.size())
            {
                bool isNumber = false;
                tokens[i].toDouble(&isNumber);

                if (isNumber)
                {
                    const QString &unit1 = tokens[i + 1];
                    const QString &kw    = tokens[i + 2];
                    const QString &unit2 = tokens[i + 3];

                    const QString kwLower = kw.toLower();
                    if ((kwLower == QStringLiteral("to") || kwLower == QStringLiteral("in"))
                        && unit1.length() == 3 && unit2.length() == 3
                        && std::none_of(unit1.begin(), unit1.end(), [](QChar c) { return !c.isLetter(); })
                        && std::none_of(unit2.begin(), unit2.end(), [](QChar c) { return !c.isLetter(); }))
                    {
                        normalized.append(tokens[i]);
                        normalized.append(unit1.toUpper());
                        normalized.append(kwLower);
                        normalized.append(unit2.toUpper());
                        i += 3;
                        continue;
                    }
                }
            }
            normalized.append(tokens[i]);
        }

        expr = normalized.join(QLatin1Char(' ')).toStdString();
    }

    MathStructure mstruct;
    EvaluationOptions eo;
    bool success = CALCULATOR->calculate(&mstruct, expr, TIMEOUT_MS, eo);
    if (!success)
    {
        return {};
    }

    PrintOptions po;
    po.interval_display = INTERVAL_DISPLAY_MIDPOINT;
    std::string res_str = CALCULATOR->print(mstruct, TIMEOUT_MS, po);

    return QString::fromStdString(res_str);
}

QString TestQalculatorRunner::resultNoRepeat(const QString &term, const QString &result)
{
    std::string expr = CALCULATOR->unlocalizeExpression(term.toStdString());
    std::string res_str = result.toStdString();

    // Don't return the original expression if it is identical to the result
    if (res_str == expr)
    {
        return {};
    }

    return result;
}

void TestQalculatorRunner::testBasicCalculation()
{
    // 2 + 2 = 4
    QString result = calculate(QStringLiteral("2+2"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 4.0);

    // 10 * 5 = 50
    result = calculate(QStringLiteral("10*5"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 50.0);

    // 100 / 4 = 25
    result = calculate(QStringLiteral("100/4"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 25.0);

    // 7 - 3 = 4
    result = calculate(QStringLiteral("7-3"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 4.0);

    // 2^10 = 1024
    result = calculate(QStringLiteral("2^10"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 1024.0);
}

void TestQalculatorRunner::testComplexExpression()
{
    // sqrt(16) = 4
    QString result = calculate(QStringLiteral("sqrt(16)"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 4.0);

    // pi ≈ 3.14159… (locale-agnostic numeric check)
    result = calculate(QStringLiteral("pi"));
    QVERIFY(!result.isEmpty());
    bool ok = false;
    double val = parseNumeric(result, &ok);
    QVERIFY(ok);
    QVERIFY(qAbs(val - 3.14159) < 0.001);

    // sin(pi/2) ≈ 1
    result = calculate(QStringLiteral("sin(pi/2)"));
    QVERIFY(!result.isEmpty());
    val = parseNumeric(result, &ok);
    QVERIFY(ok);
    QVERIFY(qAbs(val - 1.0) < 0.001);

    // 5! = 120
    result = calculate(QStringLiteral("5!"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 120.0);

    // log10(100) = 2 — note: libqalculate's log() is natural log, use log10() for base-10
    result = calculate(QStringLiteral("log10(100)"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 2.0);

    // (2+3)*(4+1) = 25
    result = calculate(QStringLiteral("(2+3)*(4+1)"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 25.0);
}

void TestQalculatorRunner::testUnitConversion()
{
    // 1 meter to cm — result may show "100 cm" or "100 см" depending on locale
    QString result = calculate(QStringLiteral("1 m to cm"));
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains(QStringLiteral("100")));
    // Verify that the result mentions some form of "centimeter" (locale-aware)
    QVERIFY(result.contains(QStringLiteral("cm")) || result.contains(QStringLiteral("см")));

    // 100 km/h to mph — result may include "mph" or locale variant
    result = calculate(QStringLiteral("100 km/h to mph"));
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains(QStringLiteral("mph")));
    // Extract the numeric part for approximate comparison
    bool ok = false;
    double val = parseNumeric(result.section(QLatin1Char(' '), 0, 0), &ok);
    if (!ok)
    {
        val = parseNumeric(result, &ok);
    }
    QVERIFY(ok);
    QVERIFY(qAbs(val - 62.14) < 1.0);

    // Uppercase compound unit (MPH = 3-letter all-alpha) — verify that the
    // currency-code normalization does NOT mistakenly treat it as a currency
    // code. "MPH" is 3 letters (would pass the length check) but "km/h"
    // is 4 chars (fails length check), so the normalization skips it.
    // This also confirms libqalculate handles uppercase "MPH" correctly.
    result = calculate(QStringLiteral("100 MPH to km/h"));
    QVERIFY(!result.isEmpty());
    // Must NOT echo input — verify conversion happened
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    // Should mention km/h or its locale variant
    QVERIFY(result.contains(QStringLiteral("km")) || result.contains(QStringLiteral("км")));
    // Validate numeric: 100 mph ≈ 160.934 km/h
    ok = false;
    val = parseNumeric(result.section(QLatin1Char(' '), 0, 0), &ok);
    if (!ok)
    {
        val = parseNumeric(result, &ok);
    }
    QVERIFY(ok);
    QVERIFY(qAbs(val - 160.93) < 1.0);
}

void TestQalculatorRunner::testCurrencyConversion()
{
    // Exchange rates require network — skip gracefully if unavailable
    if (!CALCULATOR->loadExchangeRates())
    {
        QSKIP("Exchange rates not available (no network?)");
    }

    // Helper: check that result contains at least one digit (proves numeric conversion)
    auto hasDigit = [](const QString &s) {
        for (int i = 0; i < s.size(); ++i)
        {
            if (s.at(i).isDigit())
                return true;
        }
        return false;
    };

    // 100 USD to EUR — result format varies by locale/print options:
    // e.g. "87,5197 €", "87.5197 EUR", "~87.5 EUR"
    QString result = calculate(QStringLiteral("100 USD to EUR"));
    QVERIFY(!result.isEmpty());
    // The result must be a real conversion — not just echoing the input
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    // Must contain at least one digit (proves it's a numeric result)
    QVERIFY(hasDigit(result));
    // Should reference EUR or € somewhere
    QVERIFY(result.contains(QStringLiteral("EUR")) || result.contains(QLatin1Char('€')));

    // 100 TRY to USD — the specific bug scenario
    result = calculate(QStringLiteral("100 TRY to USD"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    // Should reference USD or $
    QVERIFY(result.contains(QStringLiteral("USD")) || result.contains(QStringLiteral("$")));

    // Lowercase currency codes now work thanks to the normalization in calculate()
    // which uppercases 3-letter tokens in a conversion context.
    result = calculate(QStringLiteral("100 try to usd"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    QVERIFY(result.contains(QStringLiteral("USD")) || result.contains(QStringLiteral("$")));

    // Additional lowercase variant — verify it converts correctly
    result = calculate(QStringLiteral("100 usd to rub"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    // RUB check is sufficient — libqalculate returns "RUB" not the ₽ symbol
    QVERIFY(result.contains(QStringLiteral("RUB")));
}

void TestQalculatorRunner::testE2eWithTrigger()
{
    // Exchange rates require network — skip gracefully if unavailable
    if (!CALCULATOR->loadExchangeRates())
    {
        QSKIP("Exchange rates not available (no network?)");
    }

    // Helper: check that result contains at least one digit (proves numeric conversion)
    auto hasDigit = [](const QString &s) {
        for (int i = 0; i < s.size(); ++i)
        {
            if (s.at(i).isDigit())
                return true;
        }
        return false;
    };

    // Mock the KRunner match pipeline:
    //   1. Strip trigger prefix (just like QalculatorRunner::match())
    //   2. Call calculate() with the stripped term
    //   3. The result should contain the converted value
    //
    // This simulates what happens when user types "=100 usd to rub" in KRunner.
    auto evaluateWithTrigger = [this](const QString &query) -> QString {
        QString term = query;
        // Replicate trigger stripping from QalculatorRunner::match():
        // the "=" trigger is prepended by KRunner before passing to match()
        const QStringList triggers = {QStringLiteral("=")};
        for (const QString &trigger : triggers)
        {
            if (term.startsWith(trigger))
            {
                term = term.mid(trigger.length());
                break;
            }
        }
        // Call calculate() which includes currency-code normalization
        return this->calculate(term);
    };

    // ═══════════════════════════════════════════════════════════════
    // E2E: trigger + lowercase currency conversion
    // ═══════════════════════════════════════════════════════════════
    QString result = evaluateWithTrigger(QStringLiteral("=100 usd to rub"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    QVERIFY(result.contains(QStringLiteral("RUB")));

    // ═══════════════════════════════════════════════════════════════
    // E2E: trigger + uppercase currency (regression)
    // ═══════════════════════════════════════════════════════════════
    result = evaluateWithTrigger(QStringLiteral("=100 TRY to USD"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    QVERIFY(result.contains(QStringLiteral("USD")) || result.contains(QStringLiteral("$")));

    // ═══════════════════════════════════════════════════════════════
    // E2E: trigger + lowercase + "to" keyword
    // ═══════════════════════════════════════════════════════════════
    result = evaluateWithTrigger(QStringLiteral("=100 eur to usd"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!result.startsWith(QStringLiteral("100")));
    QVERIFY(hasDigit(result));
    QVERIFY(result.contains(QStringLiteral("USD")) || result.contains(QStringLiteral("$")));

    // ═══════════════════════════════════════════════════════════════
    // E2E: trigger + basic arithmetic (no currency involved)
    // ═══════════════════════════════════════════════════════════════
    result = evaluateWithTrigger(QStringLiteral("=2+2"));
    QVERIFY(!result.isEmpty());
    QCOMPARE(parseNumeric(result), 4.0);

    // ═══════════════════════════════════════════════════════════════
    // E2E: trigger + unit conversion (should not break)
    // ═══════════════════════════════════════════════════════════════
    result = evaluateWithTrigger(QStringLiteral("=100 km/h to mph"));
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains(QStringLiteral("mph")));

    // ═══════════════════════════════════════════════════════════════
    // Negative: just "=" with nothing → should not crash
    // ═══════════════════════════════════════════════════════════════
    result = evaluateWithTrigger(QStringLiteral("="));
    // No assertion — just confirming no crash
}

void TestQalculatorRunner::testInputEqualsResult()
{
    // If the expression evaluates to itself, resultNoRepeat should return empty.
    // For input "5", calculate() returns "5" (or similar), and since the
    // expression matches the result, it should be suppressed.
    QVERIFY(resultNoRepeat(QStringLiteral("5"), QStringLiteral("5")).isEmpty());

    // For "2+2", the result "4" differs from expression "2+2" → should NOT suppress
    QString result = calculate(QStringLiteral("2+2"));
    QVERIFY(!result.isEmpty());
    QVERIFY(!resultNoRepeat(QStringLiteral("2+2"), result).isEmpty());
}

void TestQalculatorRunner::testErrorHandling()
{
    // Note: libqalculate handles many "invalid" inputs gracefully by interpreting
    // them as variables or expressions. The key test is that a truly unparseable
    // expression returns empty.

    // libqalculate is very permissive — it may return something for most inputs.
    // We verify it doesn't crash rather than asserting exact results.

    // Empty string → should not crash
    QString result = calculate(QString());
    // No assertion — just verifying no crash

    // Garbage input should not crash libqalculate
    result = calculate(QStringLiteral("@#$%^&*"));
    // No assertion — just verifying no crash

    // Input that induces a genuine calculation error should not crash
    result = calculate(QStringLiteral("1/0"));
    // libqalculate may return infinity or empty — no assertion, just crash check
}

void TestQalculatorRunner::testExpressionUnlocalization()
{
    // Standard decimal point should stay as-is
    std::string result = CALCULATOR->unlocalizeExpression(std::string("2.5+2.5"));
    QCOMPARE(QString::fromStdString(result), QStringLiteral("2.5+2.5"));

    // Simple expression should remain unchanged
    result = CALCULATOR->unlocalizeExpression(std::string("2+2"));
    QCOMPARE(QString::fromStdString(result), QStringLiteral("2+2"));

    // Function names should remain unchanged
    result = CALCULATOR->unlocalizeExpression(std::string("sqrt(16)"));
    QCOMPARE(QString::fromStdString(result), QStringLiteral("sqrt(16)"));
}

QTEST_MAIN(TestQalculatorRunner)

#include "test_qalculatorrunner.moc"

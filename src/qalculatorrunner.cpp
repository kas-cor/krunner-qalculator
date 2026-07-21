/*
 *   Copyright (C) 2009 Jason Siefken <siefkenj@gmail.com>
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright (C) 2006 David Faure <faure@kde.org>
 *   Copyright (C) 2007 Richard Moore <rich@kde.org>
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
 *
 *
 *   This program was modified from the original krunner_calculate plasmoid
 */

#include "qalculatorrunner.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/includes.h>

#include <KLocalizedString>

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>

QalculatorRunner::QalculatorRunner(QObject *parent, const KPluginMetaData &pluginMetaData)
    : KRunner::AbstractRunner(parent, pluginMetaData)
{
    setObjectName(QStringLiteral("Qalculator"));

    // Pre-initialize the library once when the plugin loads including all the exchange rates
    if (!CALCULATOR)
    {
        CALCULATOR = new Calculator();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
        CALCULATOR->loadExchangeRates();
    }

    // Read triggers from manifest.json (e.g. "=") dynamically so that
    // changing them in the metadata does not require a code change.
    const QJsonObject root = metadata().rawData();
    const QJsonArray triggersArray = root.value(QStringLiteral("KRunner")).toObject()
                                        .value(QStringLiteral("Triggers")).toArray();
    for (const QJsonValue &val : triggersArray)
    {
        m_triggers.append(val.toString());
    }
}

QalculatorRunner::~QalculatorRunner() = default;

void QalculatorRunner::match(KRunner::RunnerContext &context)
{
    const QString query = context.query();

    if (query.isEmpty())
    {
        return;
    }

    // Strip any trigger prefix that KRunner prepends to the query.
    // E.g. with trigger "=", KRunner passes "=100 USD to EUR" but
    // libqalculate interprets leading "=" as boolean equality.
    QString term = query;
    for (const QString &trigger : std::as_const(m_triggers))
    {
        if (term.startsWith(trigger))
        {
            term = term.mid(trigger.length());
            break;
        }
    }

    const QString result = calculate(term);
    if (!result.isEmpty())
    {
        KRunner::QueryMatch match(this);
        match.setRelevance(1.0);
        match.setText(result);
        match.setIconName(QStringLiteral("accessories-calculator"));

        KRunner::Action copyAction(QStringLiteral("copy"), QStringLiteral("edit-copy"), i18n("Copy to clipboard"));
        match.addAction(copyAction);

        context.addMatch(match);
    }
}

void QalculatorRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    const QString result = match.text();
    const QString action = match.selectedAction().id();

    if (action == QLatin1String("copy"))
    {
        if (!copyToClipboard(result))
        {
            qWarning() << "Failed to copy result to clipboard";
        }
        context.requestQueryStringUpdate(QString(), 0); // Close KRunner
    }
    else
    {
        // Insert result into query line without closing KRunner
        context.requestQueryStringUpdate(result, result.length());
    }
}

bool QalculatorRunner::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard)
    {
        qWarning() << "Failed to access system clipboard";
        return false;
    }

    clipboard->setText(text);
    return true;
}

QString QalculatorRunner::calculate(const QString &term)
{
    int timeout = 2000;

    std::string expr = CALCULATOR->unlocalizeExpression(term.toStdString());

    // libqalculate is case-sensitive for currency codes (ISO 4217).
    // "100 usd to rub" is NOT recognized, but "100 USD to RUB" is.
    // Uppercase any 3-letter alphabetic token in a conversion pair
    // (NUMBER TOKEN1 to/in TOKEN2) — this targets currency codes
    // while skipping 2-letter unit tokens (kg, lb, cm) and compound
    // units (km/h, mph).
    {
        const QStringList tokens = QString::fromStdString(expr).split(QLatin1Char(' '), Qt::SkipEmptyParts);
        QStringList normalized;
        normalized.reserve(tokens.size());

        for (qsizetype i = 0; i < tokens.size(); ++i)
        {
            // Look ahead for the conversion pattern
            if (i + 3 < tokens.size())
            {
                bool isNumber = false;
                tokens[i].toDouble(&isNumber);

                if (isNumber)
                {
                    const QString &unit1 = tokens[i + 1];
                    const QString &kw    = tokens[i + 2];
                    const QString &unit2 = tokens[i + 3];

                    // Check keyword is "to" or "in" (case-insensitive)
                    const QString kwLower = kw.toLower();
                    if ((kwLower == QStringLiteral("to") || kwLower == QStringLiteral("in"))
                        && unit1.length() == 3 && unit2.length() == 3
                        && std::none_of(unit1.begin(), unit1.end(), [](QChar c) { return !c.isLetter(); })
                        && std::none_of(unit2.begin(), unit2.end(), [](QChar c) { return !c.isLetter(); }))
                    {
                        // 3-letter tokens in a conversion context are
                        // almost certainly ISO 4217 currency codes.
                        normalized.append(tokens[i]);              // number
                        normalized.append(unit1.toUpper());        // source currency
                        normalized.append(kwLower);                 // libqalculate only recognizes lowercase keywords
                        normalized.append(unit2.toUpper());        // target currency
                        i += 3;
                        continue;
                    }
                }
            }
            normalized.append(tokens[i]);
        }

        expr = normalized.join(QLatin1Char(' ')).toStdString();
    }

    // TODO: Both `eo` and `po` should be configured to have more sane defaults, or even make them user configurable
    // (but that is a problem for future me, now I have studies to continue)

    MathStructure mstruct;
    EvaluationOptions eo;
    bool success = CALCULATOR->calculate(&mstruct, expr, timeout, eo);
    if (!success)
    {
        return {}; // abort if calculation failed
    }

    PrintOptions po;
    po.interval_display = INTERVAL_DISPLAY_MIDPOINT; // approximate result instead of "interval()"
    std::string res_str = CALCULATOR->print(mstruct, timeout, po);

    if (res_str == expr)
    {
        return {}; // dont return the original expression if it is the result
    }

    QString result = QString::fromStdString(res_str);

    return result;
}

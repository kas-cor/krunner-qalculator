/*
 *   Copyright (C) 2009 Jason Siefken <siefkenj@gmail.com>
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright (C) 2006 David Faure <faure@kde.org>
 *   Copyright (C) 2007 Richard Moore <rich@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 *   This program was modified from the original krunner_calculate plasmoid
 */

#include "qalculatorrunner.h"

#include <libqalculate/Calculator.h>

#include <QProcess>
#include <QRegularExpression>
#include <KLocalizedString>
#include <libqalculate/includes.h>

QalculatorRunner::QalculatorRunner(QObject* parent, const KPluginMetaData &pluginMetaData)
    : KRunner::AbstractRunner(parent, pluginMetaData)
{
    setObjectName(QStringLiteral("Qalculator"));

    // Pre-initialize the library once when the plugin loads including all the exchange rates
    if (!CALCULATOR) {
        CALCULATOR = new Calculator();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
        CALCULATOR->loadExchangeRates();
    }
}

QalculatorRunner::~QalculatorRunner()
{
}

void QalculatorRunner::match(KRunner::RunnerContext &context)
{
    const QString term = context.query();

    if (term.isEmpty()) {
        return;
    }

    const QString result = calculate(term);
    if (!result.isEmpty()) {
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

    if (action == QLatin1String("copy")) {
        if (!copyToClipboard(result)) {
            qWarning() << "Failed to copy result to clipboard";
        }
        context.requestQueryStringUpdate(QString(), 0); // Close KRunner
    } else {
        // Insert result into query line without closing KRunner
        context.requestQueryStringUpdate(result, result.length());
    }
}

bool QalculatorRunner::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard) {
        qWarning() << "Failed to access system clipboard";
        return false;
    }

    clipboard->setText(text);
    return true;
}

QString QalculatorRunner::calculate(const QString &term)
{
    int timeout  = 2000;

    std::string expr = CALCULATOR->unlocalizeExpression(term.toStdString());

    // TODO: Both `eo` and `po` should be configured to have more sane defaults, or even make them user configurable (but that is a problem for future me, now I have studies to continue)

    MathStructure mstruct;
    EvaluationOptions eo;
    bool success = CALCULATOR->calculate(&mstruct, expr, timeout, eo);
    if (!success) return QString(); // abort if calculation failed

    PrintOptions po;
    po.interval_display = INTERVAL_DISPLAY_MIDPOINT; // approximate result instead of "interval()"
    std::string res_str = CALCULATOR->print(mstruct, timeout, po);

    if (res_str == expr) return QString(); // dont return the original expression if it is the result

    QString result = QString::fromStdString(res_str);

    return result;
}

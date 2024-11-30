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

#include <QProcess>
#include <KLocalizedString>

QalculatorRunner::QalculatorRunner(QObject* parent, const KPluginMetaData &pluginMetaData)
    : KRunner::AbstractRunner(parent, pluginMetaData)
{
    setObjectName(QStringLiteral("Qalculator"));
}

QalculatorRunner::~QalculatorRunner()
{
}

void QalculatorRunner::match(KRunner::RunnerContext &context)
{
    const QString term = context.query();
    
    if (term.length() < 3) {
        return;
    }

    const QString result = calculate(term);
    if (!result.isEmpty()) {
        KRunner::QueryMatch match(this);
        match.setRelevance(1.0);
        match.setText(result);
        match.setIconName(QStringLiteral("accessories-calculator"));
        match.setData(QStringLiteral("copy"));

        KRunner::Action copyAction(QStringLiteral("copy"), QStringLiteral("edit-copy"), i18n("Copy to clipboard"));
        match.addAction(copyAction);
        
        context.addMatch(match);
    }
}

void QalculatorRunner::run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match)
{
    Q_UNUSED(context)
    
    const QString result = match.text();
    const QString action = match.selectedAction().id();

    if (action == QLatin1String("copy")) {
        copyToClipboard(result);
        context.requestQueryStringUpdate(QString(), 0); // Close KRunner
    } else {
        // Insert result into query line without closing KRunner
        context.requestQueryStringUpdate(result, result.length());
    }
}

void QalculatorRunner::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(text);
}

QString QalculatorRunner::calculate(const QString &term)
{
    QProcess qalculateProcess;
    QStringList args;
    args << QStringLiteral("--defaults")
         << QStringLiteral("-e")
         << QStringLiteral("-t")
         << QStringLiteral("+u8")
         << term;
    
    qalculateProcess.start(QStringLiteral("qalc"), args);

    if (!qalculateProcess.waitForStarted()) {
        return QString();
    }

    if (!qalculateProcess.waitForFinished()) {
        return QString();
    }

    if (qalculateProcess.exitCode() != 0) {
        return QString();
    }

    QString result = QString::fromUtf8(qalculateProcess.readAllStandardOutput());
    result = result.trimmed();

    if (result.contains(QLatin1Char('\n'))) {
        result = result.split(QLatin1Char('\n')).last().trimmed();
    }

    return result;
}

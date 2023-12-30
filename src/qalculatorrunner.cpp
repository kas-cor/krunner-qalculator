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

#include <QWidget>
#include <QString>
#include <QProcess>
#include <QIcon>
#include <KLocalizedString>

QalculatorRunner::QalculatorRunner(QObject* parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)

    setObjectName("Qalculator");
    setPriority(AbstractRunner::HighestPriority);
}

QalculatorRunner::~QalculatorRunner()
{
}

void QalculatorRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QString cmd = term;

    //no meanless space between friendly guys: helps simplify code
    cmd = cmd.trimmed().replace(" ", "");

    if (cmd.length() < 4) {
        return;
    }

    //make sure there is an equals sign. if not
    //we shouldn't be computing anything!
    if (!cmd.contains("=", Qt::CaseSensitive)) {
        return;
    }

    if (cmd[0] == '=') {
        cmd.remove(0, cmd.indexOf('=') + 1);
    } else if (cmd.endsWith('=')) {
        cmd.chop(1);
    }
    
    //santity check that the user didn't just type "   ="
    //or somesuch nonsense
    if (cmd.isEmpty()) {
        return;
    }

    QString result = calculate(cmd);
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::InformationalMatch);
    match.setIconName(QStringLiteral("accessories-calculator"));
    match.setText(result);
    match.setData("=" + result);
    match.setId(QString());
    match.setRelevance(1.0);
    if (!context.isValid()) {
        return;
    }
    QList<Plasma::QueryMatch> matches;
    matches << match;
    context.addMatches(matches);
}

QString QalculatorRunner::calculate(const QString& term)
{
    QStringList argList;

    //load up the options
    argList << "-t" << "-u8";

    QProcess qalculateProcess;

    qalculateProcess.start("qalc", QStringList() << argList << term);

    if (!qalculateProcess.waitForFinished()){
        return "qalc failed to execute";
    }

    //make sure to read our data in as utf-8 so QString doesn't mangle it!
    QString result = QString::fromUtf8(qalculateProcess.readAll());

    return result.trimmed();
}

K_PLUGIN_CLASS_WITH_JSON(QalculatorRunner, "manifest.json");

#include "qalculatorrunner.moc"

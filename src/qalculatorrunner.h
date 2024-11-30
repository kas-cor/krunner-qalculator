/*
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
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
 */

#pragma once

#include <QObject>
#include <QString>
#include <KRunner/AbstractRunner>
#include <KRunner/RunnerContext>
#include <KRunner/Action>
#include <KPluginFactory>
#include <QClipboard>
#include <QGuiApplication>

/**
 * This class evaluates the basic expressions given in the interface.
 */
class QalculatorRunner : public KRunner::AbstractRunner
{
    Q_OBJECT

public:
    QalculatorRunner(QObject* parent, const KPluginMetaData &pluginMetaData);
    ~QalculatorRunner() override;

public Q_SLOTS:
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

private:
    QString calculate(const QString &term);
    void copyToClipboard(const QString &text);
};

K_PLUGIN_CLASS_WITH_JSON(QalculatorRunner, "manifest.json")

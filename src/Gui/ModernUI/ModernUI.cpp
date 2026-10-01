// SPDX-License-Identifier: LGPL-2.1-or-later
/****************************************************************************
 *                                                                          *
 *   Copyright (c) 2026 The FreeCAD Project Association AISBL               *
 *                                                                          *
 *   This file is part of FreeCAD.                                          *
 *                                                                          *
 *   FreeCAD is free software: you can redistribute it and/or modify it     *
 *   under the terms of the GNU Lesser General Public License as            *
 *   published by the Free Software Foundation, either version 2.1 of the   *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   FreeCAD is distributed in the hope that it will be useful, but         *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with FreeCAD. If not, see                                *
 *   <https://www.gnu.org/licenses/>.                                       *
 *                                                                          *
 ***************************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QRegularExpression>

#include <App/Application.h>

#include "ModernUI.h"
#include "../Action.h"
#include "../BitmapFactory.h"
#include "../Command.h"
#include "CommandSearch.h"
#include "../ToolBarManager.h"

using namespace Gui;

ParameterGrp::handle ModernUI::parameters()
{
    return App::GetApplication().GetParameterGroupByPath(
        "User parameter:BaseApp/Preferences/ModernUI"
    );
}

bool ModernUI::isEnabled()
{
    return parameters()->GetBool("Enabled", false);
}

ModernUI::Settings::Settings()
{
    ParameterGrp::handle group = parameters();  // groups live as long as the parameter manager
    connParam = App::GetApplication().GetUserParameter().signalParamChanged.connect(
        [this, group](ParameterGrp* hParam, ParameterGrp::ParamType, const char* name, const char*) {
            if (hParam != static_cast<ParameterGrp*>(group) || !name) {
                return;
            }
            const QByteArray key(name);
            // Queued, so that receivers never run inside the parameter manager's notification
            QMetaObject::invokeMethod(this, [this, key]() {
                Q_EMIT parameterChanged(key);
                if (key == "Enabled") {
                    Q_EMIT enabledChanged(isEnabled());
                }
            }, Qt::QueuedConnection);
        }
    );
}

ModernUI::Settings* ModernUI::Settings::instance()
{
    // Intentionally never deleted: it is only a relay and must outlive the widgets using it
    static auto settings = new Settings();
    return settings;
}

QString ModernUI::commandLabel(const Command* cmd)
{
    if (!cmd) {
        return {};
    }

    // Labels for commands whose menu text is long or generic. Other commands use their menu
    // text, so translations of the command itself are reused.
    static const QHash<QByteArray, const char*> shortLabels = {
        {"Std_DlgPreferences", QT_TRANSLATE_NOOP("CommandPanel", "Preferences")},
        {"Std_ProjectInfo", QT_TRANSLATE_NOOP("CommandPanel", "Project Information")},
        {"Std_UnitsCalculator", QT_TRANSLATE_NOOP("CommandPanel", "Units")},
        {"Std_Export", QT_TRANSLATE_NOOP("CommandPanel", "Export Drawing")},
        {"Std_Import", QT_TRANSLATE_NOOP("CommandPanel", "Import DXF/DWG")},
        {"Std_Measure", QT_TRANSLATE_NOOP("CommandPanel", "Measure")},
        {"Part_CheckGeometry", QT_TRANSLATE_NOOP("CommandPanel", "Check Geometry")},
        {"Part_ExplodeCompound", QT_TRANSLATE_NOOP("CommandPanel", "Explode")},
        {"Draft_Trimex", QT_TRANSLATE_NOOP("CommandPanel", "Trim / Extend")},
        {"Draft_Heal", QT_TRANSLATE_NOOP("CommandPanel", "Heal")},
        {"Draft_SetStyle", QT_TRANSLATE_NOOP("CommandPanel", "Drawing Style")},
        {"Draft_AnnotationStyleEditor", QT_TRANSLATE_NOOP("CommandPanel", "Annotation Styles")},
        {"Draft_SubelementHighlight", QT_TRANSLATE_NOOP("CommandPanel", "Highlight Subelements")},
    };

    auto it = shortLabels.find(QByteArray(cmd->getName()));
    if (it != shortLabels.end()) {
        return QCoreApplication::translate("CommandPanel", it.value());
    }

    QString text = Action::commandMenuText(cmd);
    text.remove(QLatin1Char('&'));
    static const QRegularExpression ellipsis(QStringLiteral("\\s*(\\.\\.\\.|\\x2026)\\s*$"));
    text.remove(ellipsis);
    return text.trimmed();
}

void ModernUI::installModernIcons()
{
    static const QString modernPath = QStringLiteral(":/icons/modern");

    // Make sure the default search paths exist, then put the modern icons right before the
    // built-in resources. Icons in user folders (e.g. <UserAppData>/icons) keep precedence.
    (void)BitmapFactory();
    QStringList paths = QDir::searchPaths(QStringLiteral("icons"));
    paths.removeAll(modernPath);
    qsizetype index = 0;
    for (; index < paths.size(); ++index) {
        if (paths[index].startsWith(QLatin1String(":/"))) {
            break;
        }
    }
    paths.insert(index, modernPath);
    QDir::setSearchPaths(QStringLiteral("icons"), paths);
}

void ModernUI::setupMainWindow()
{
    // The tool bar manager owns the menu bar corner areas; create it first so that the search
    // field wraps its corner area instead of being replaced by it later.
    (void)ToolBarManager::getInstance();

    const auto update = []() {
        const bool show = isEnabled() && parameters()->GetBool("CommandSearch", true);
        CommandSearchBox* box = CommandSearchBox::instance();
        if (show && !box) {
            box = CommandSearchBox::install();
        }
        if (box) {
            box->setVisible(show);
        }
    };
    update();
    QObject::connect(Settings::instance(), &Settings::parameterChanged, [update](const QByteArray& name) {
        if (name == "Enabled" || name == "CommandSearch") {
            update();
        }
    });
}

#include "moc_ModernUI.cpp"

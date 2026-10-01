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

#include <QActionGroup>

#include "CommandSearch.h"
#include "WorkspaceAppearance.h"
#include "../Action.h"
#include "../Application.h"
#include "../BitmapFactory.h"
#include "../Command.h"
#include "../MainWindow.h"
#include "../Dialogs/DlgPreferencesImp.h"

using namespace Gui;

namespace
{

//===========================================================================
// Std_CommandSearch
//===========================================================================
class StdCmdCommandSearch: public Command
{
public:
    StdCmdCommandSearch()
        : Command("Std_CommandSearch")
    {
        sGroup = "Tools";
        sMenuText = QT_TR_NOOP("Search &Commands…");
        sToolTipText = QT_TR_NOOP("Searches all commands by name, menu text or tool tip and runs the selected one");
        sWhatsThis = "Std_CommandSearch";
        sStatusTip = sToolTipText;
        sPixmap = "ModernUI_Search";
        sAccel = "Ctrl+K";
        eType = 0;
    }

    const char* className() const override
    {
        return "StdCmdCommandSearch";
    }

protected:
    void activated(int) override
    {
        if (auto box = CommandSearchBox::instance()) {
            box->activate();
        }
    }

    bool isActive() override
    {
        return CommandSearchBox::instance() != nullptr;
    }
};

//===========================================================================
// Std_WorkspacePreset
//===========================================================================
class StdCmdWorkspacePreset: public Command
{
public:
    StdCmdWorkspacePreset()
        : Command("Std_WorkspacePreset")
    {
        sGroup = "Standard-View";
        sMenuText = QT_TR_NOOP("Workspace &Background");
        sToolTipText = QT_TR_NOOP("Changes the background of the 3D workspace");
        sWhatsThis = "Std_WorkspacePreset";
        sStatusTip = sToolTipText;
        sPixmap = "ModernUI_Workspace";
        eType = 0;
    }

    const char* className() const override
    {
        return "StdCmdWorkspacePreset";
    }

protected:
    Action* createAction() override
    {
        auto pcAction = new ActionGroup(this, getMainWindow());
        pcAction->setDropDownMenu(true);
        pcAction->setIsMode(true);
        applyCommandData(this->className(), pcAction);

        for (const auto& preset : ModernUI::WorkspaceAppearance::presets()) {
            QAction* action = pcAction->addAction(QString());
            action->setCheckable(true);
            action->setObjectName(QStringLiteral("Std_WorkspacePreset_") + QString::fromLatin1(preset.id));
            action->setWhatsThis(QString::fromLatin1(getWhatsThis()));
        }
        pcAction->setIcon(BitmapFactory().iconFromTheme(getPixmap()));

        _pcAction = pcAction;
        languageChange();
        return pcAction;
    }

    void languageChange() override
    {
        Command::languageChange();
        auto group = qobject_cast<ActionGroup*>(_pcAction);
        if (!group) {
            return;
        }
        const auto& presets = ModernUI::WorkspaceAppearance::presets();
        QList<QAction*> actions = group->actions();
        for (int i = 0; i < actions.size() && i < int(presets.size()); ++i) {
            actions[i]->setText(presets[i].text());
            actions[i]->setToolTip(QObject::tr("Use the '%1' workspace background").arg(presets[i].text()));
            actions[i]->setStatusTip(actions[i]->toolTip());
        }
    }

    void activated(int iMsg) override
    {
        const auto& presets = ModernUI::WorkspaceAppearance::presets();
        if (iMsg >= 0 && iMsg < int(presets.size())) {
            ModernUI::WorkspaceAppearance::applyPreset(presets[iMsg].id);
        }
    }

    bool isActive() override
    {
        // keep the check mark in sync with preferences changed elsewhere
        if (auto group = qobject_cast<ActionGroup*>(_pcAction)) {
            const QByteArray current = ModernUI::WorkspaceAppearance::currentPreset();
            const auto& presets = ModernUI::WorkspaceAppearance::presets();
            QList<QAction*> actions = group->actions();
            for (int i = 0; i < actions.size() && i < int(presets.size()); ++i) {
                const bool checked = presets[i].id == current;
                if (actions[i]->isChecked() != checked) {
                    actions[i]->setChecked(checked);
                }
            }
        }
        return true;
    }
};

//===========================================================================
// Std_WorkspaceGrid
//===========================================================================
class StdCmdWorkspaceGrid: public Command
{
public:
    StdCmdWorkspaceGrid()
        : Command("Std_WorkspaceGrid")
    {
        sGroup = "Standard-View";
        sMenuText = QT_TR_NOOP("Engineering &Grid");
        sToolTipText = QT_TR_NOOP(
            "Shows or hides the engineering grid behind the model.\n"
            "The grid is visual only and does not change any snap setting."
        );
        sWhatsThis = "Std_WorkspaceGrid";
        sStatusTip = sToolTipText;
        sPixmap = "ModernUI_Grid";
        eType = 0;
    }

    const char* className() const override
    {
        return "StdCmdWorkspaceGrid";
    }

protected:
    Action* createAction() override
    {
        Action* pcAction = Command::createAction();
        pcAction->setCheckable(true);
        pcAction->setChecked(ModernUI::WorkspaceAppearance::isGridVisible());
        return pcAction;
    }

    void activated(int) override
    {
        ModernUI::WorkspaceAppearance::setGridVisible(!ModernUI::WorkspaceAppearance::isGridVisible());
    }

    bool isActive() override
    {
        const bool visible = ModernUI::WorkspaceAppearance::isGridVisible();
        if (_pcAction && _pcAction->isChecked() != visible) {
            _pcAction->setChecked(visible);
        }
        return true;
    }
};

//===========================================================================
// Std_WorkspaceAppearance
//===========================================================================
class StdCmdWorkspaceAppearance: public Command
{
public:
    StdCmdWorkspaceAppearance()
        : Command("Std_WorkspaceAppearance")
    {
        sGroup = "Standard-View";
        sMenuText = QT_TR_NOOP("Workspace &Appearance Settings…");
        sToolTipText = QT_TR_NOOP("Opens the preferences of the modern interface and the workspace background");
        sWhatsThis = "Std_WorkspaceAppearance";
        sStatusTip = sToolTipText;
        sPixmap = "preferences-system";
        eType = 0;
    }

    const char* className() const override
    {
        return "StdCmdWorkspaceAppearance";
    }

protected:
    void activated(int) override
    {
        Dialog::DlgPreferencesImp dlg(getMainWindow());
        dlg.activateGroupPageByPageName(
            QStringLiteral("Display"),
            QStringLiteral("Gui::Dialog::DlgSettingsModernUI")
        );
        dlg.exec();
    }

    bool isActive() override
    {
        return true;
    }
};

}  // namespace

namespace Gui
{

void CreateModernUICommands()
{
    CommandManager& rcCmdMgr = Application::Instance->commandManager();
    rcCmdMgr.addCommand(new StdCmdCommandSearch());
    rcCmdMgr.addCommand(new StdCmdWorkspacePreset());
    rcCmdMgr.addCommand(new StdCmdWorkspaceGrid());
    rcCmdMgr.addCommand(new StdCmdWorkspaceAppearance());
}

}  // namespace Gui

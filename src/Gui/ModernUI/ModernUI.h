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

#ifndef GUI_MODERNUI_MODERNUI_H
#define GUI_MODERNUI_MODERNUI_H

#include <QObject>
#include <QString>

#include <Base/Parameter.h>
#include <FCGlobal.h>

namespace Gui
{
class Command;

/**
 * Shared helpers of the modern UI layer (bottom command panel, command search, workspace
 * appearance and modern icons).
 *
 * Colors and metrics are not defined here: they live in the theme tokens
 * (Stylesheets/parameters/<theme>.yaml) and reach the widgets through the style sheet, so a
 * dark or custom theme only needs another token file. The widgets of this module therefore
 * only set object names and dynamic properties that the style sheet can address.
 */
namespace ModernUI
{

/// Parameter group "BaseApp/Preferences/ModernUI" holding the modern UI settings
GuiExport ParameterGrp::handle parameters();

/**
 * Whether the modern UI layer is switched on (parameter ModernUI/Enabled).
 *
 * Off by default, so existing setups keep their look and layout; the "FreeCAD Modern"
 * preference pack switches it on.
 */
GuiExport bool isEnabled();

/**
 * Reports changes of the ModernUI parameters, so that the command panel, the command search
 * and the tree search field follow the preferences (or a preference pack) without a restart.
 */
class GuiExport Settings: public QObject
{
    Q_OBJECT

public:
    static Settings* instance();

Q_SIGNALS:
    /// Emitted when ModernUI/Enabled changes
    void enabledChanged(bool enabled);
    /// Emitted for any parameter of the ModernUI group, with its name
    void parameterChanged(const QByteArray& name);

private:
    Settings();
    boost::signals2::scoped_connection connParam;
};

/// Short, user facing label of a command, suitable for a button with the text under the icon
GuiExport QString commandLabel(const Command* cmd);

/**
 * Makes the modern icon set take precedence over the default icons.
 *
 * The modern icons live in the resource folder ":/icons/modern" and are named like the
 * pixmaps they replace (e.g. "Draft_Line.svg"). Prepending the folder to the "icons:" search
 * path means every command, menu, toolbar and tree item that refers to an icon by name picks
 * up the modern variant if one exists, and the original one otherwise. The mapping is
 * therefore centralized in that single folder. Must be called before icons are loaded.
 */
GuiExport void installModernIcons();

/// Adds the modern UI elements to the main window (command search field), if enabled
GuiExport void setupMainWindow();

}  // namespace ModernUI
}  // namespace Gui

#endif  // GUI_MODERNUI_MODERNUI_H

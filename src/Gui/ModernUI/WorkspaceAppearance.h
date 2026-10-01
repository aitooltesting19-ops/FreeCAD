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

#ifndef GUI_MODERNUI_WORKSPACEAPPEARANCE_H
#define GUI_MODERNUI_WORKSPACEAPPEARANCE_H

#include <cstdint>
#include <vector>

#include <QByteArray>
#include <QString>

#include <Base/Parameter.h>
#include <FCGlobal.h>

namespace Gui::ModernUI
{

/**
 * A named look of the 3D workspace: background color(s) plus the engineering grid.
 *
 * Presets only write the regular view parameters (BaseApp/Preferences/View: BackgroundColor*,
 * Simple, Gradient, ... and the WorkspaceGrid* parameters). The viewers observe those
 * parameters, so a preset applies immediately and persists like any other preference. The
 * Draft working plane grid and its snap settings are not touched.
 */
struct GuiExport WorkspacePreset
{
    QByteArray id;
    const char* label;          // translatable with context "WorkspaceAppearance"
    std::uint32_t background;   // packed RGBA, used for the plain background
    bool legacyGradient;        // true: restore the classic FreeCAD gradient instead
    bool grid;
    std::uint32_t minorColor;   // packed RGBA
    std::uint32_t majorColor;   // packed RGBA

    QString text() const;
};

class GuiExport WorkspaceAppearance
{
public:
    static const std::vector<WorkspacePreset>& presets();
    static const WorkspacePreset* preset(const QByteArray& id);

    /// The view parameter group the workspace settings are stored in
    static ParameterGrp::handle viewParameters();

    /// Id of the preset stored in the preferences (empty if none was chosen yet)
    static QByteArray currentPreset();
    /// Writes the background and grid parameters of the preset and remembers it
    static void applyPreset(const QByteArray& id);

    static bool isGridVisible();
    static void setGridVisible(bool on);
};

}  // namespace Gui::ModernUI

#endif  // GUI_MODERNUI_WORKSPACEAPPEARANCE_H

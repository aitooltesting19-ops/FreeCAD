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

#include <App/Application.h>

#include "WorkspaceAppearance.h"

using namespace Gui::ModernUI;

QString WorkspacePreset::text() const
{
    return QCoreApplication::translate("WorkspaceAppearance", label);
}

const std::vector<WorkspacePreset>& WorkspaceAppearance::presets()
{
    // clang-format off
    static const std::vector<WorkspacePreset> list = {
        {"EngineeringBlue", QT_TRANSLATE_NOOP("WorkspaceAppearance", "Engineering Blue"),
         0xEAF5FFFF, false, true,  0xD8EAF8FF, 0xC3DDF1FF},
        {"White",           QT_TRANSLATE_NOOP("WorkspaceAppearance", "White"),
         0xFFFFFFFF, false, true,  0xEEF2F6FF, 0xDCE3EBFF},
        {"LightGray",       QT_TRANSLATE_NOOP("WorkspaceAppearance", "Light Gray"),
         0xEEF1F4FF, false, false, 0xE1E6EBFF, 0xD0D7DFFF},
        {"Dark",            QT_TRANSLATE_NOOP("WorkspaceAppearance", "Dark"),
         0x1E232BFF, false, true,  0x2B3440FF, 0x3B4758FF},
        {"Legacy",          QT_TRANSLATE_NOOP("WorkspaceAppearance", "Legacy FreeCAD"),
         0x141414FF, true,  false, 0xD8EAF8FF, 0xC3DDF1FF},
    };
    // clang-format on
    return list;
}

const WorkspacePreset* WorkspaceAppearance::preset(const QByteArray& id)
{
    for (const auto& it : presets()) {
        if (it.id == id) {
            return &it;
        }
    }
    return nullptr;
}

ParameterGrp::handle WorkspaceAppearance::viewParameters()
{
    return App::GetApplication().GetParameterGroupByPath("User parameter:BaseApp/Preferences/View");
}

QByteArray WorkspaceAppearance::currentPreset()
{
    return QByteArray::fromStdString(viewParameters()->GetASCII("WorkspacePreset"));
}

void WorkspaceAppearance::applyPreset(const QByteArray& id)
{
    const WorkspacePreset* p = preset(id);
    if (!p) {
        return;
    }

    ParameterGrp::handle hGrp = viewParameters();
    if (p->legacyGradient) {
        // Values of the classic FreeCAD look (see the "FreeCAD Classic" preference pack)
        hGrp->SetUnsigned("BackgroundColor", 336897023UL);
        hGrp->SetUnsigned("BackgroundColor2", 859006463UL);
        hGrp->SetUnsigned("BackgroundColor3", 2543299327UL);
        hGrp->SetBool("UseBackgroundColorMid", false);
        hGrp->SetBool("Simple", false);
        hGrp->SetBool("RadialGradient", false);
        hGrp->SetBool("Gradient", true);
    }
    else {
        hGrp->SetBool("Gradient", false);
        hGrp->SetBool("RadialGradient", false);
        hGrp->SetBool("Simple", true);
        hGrp->SetUnsigned("BackgroundColor", p->background);
    }

    hGrp->SetUnsigned("WorkspaceGridMinorColor", p->minorColor);
    hGrp->SetUnsigned("WorkspaceGridMajorColor", p->majorColor);
    hGrp->SetBool("WorkspaceGrid", p->grid);
    hGrp->SetASCII("WorkspacePreset", p->id.toStdString());
}

bool WorkspaceAppearance::isGridVisible()
{
    return viewParameters()->GetBool("WorkspaceGrid", false);
}

void WorkspaceAppearance::setGridVisible(bool on)
{
    viewParameters()->SetBool("WorkspaceGrid", on);
}

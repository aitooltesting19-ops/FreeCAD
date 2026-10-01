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

#ifndef GUI_SOFCBACKGROUNDGRID_H
#define GUI_SOFCBACKGROUNDGRID_H

#include <Inventor/SbColor.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoSubNode.h>
#include <FCGlobal.h>

class SoCamera;
class SoGLRenderAction;

namespace Gui
{

/**
 * Draws a technical "graph paper" grid behind the scene.
 *
 * The grid is purely visual: it is part of the viewer's background scene graph, it is not
 * pickable and it has no relation to the Draft working plane grid used for snapping.
 * Lines are anchored to the projection of the world origin on the view plane of the scene
 * camera, so the grid pans and zooms with the model. The minor spacing is picked from a
 * 1-2-5 sequence so the on-screen density stays constant at any zoom level, and every
 * N-th line is drawn as a major line.
 */
class GuiExport SoFCBackgroundGrid: public SoNode
{
    using inherited = SoNode;

    SO_NODE_HEADER(Gui::SoFCBackgroundGrid);

public:
    static void initClass();
    static void finish();
    SoFCBackgroundGrid();

    void GLRender(SoGLRenderAction* action) override;

    /// The camera of the scene the grid is aligned to (the background graph has its own).
    void setSceneCamera(SoCamera* camera);
    void setColors(const SbColor& minor, const SbColor& major);
    /// Overall opacity of the grid lines in the range [0, 1].
    void setOpacity(float opacity);
    /// Smallest distance in device independent pixels between two minor lines.
    void setMinimumSpacing(float pixels);
    /// Number of minor intervals between two major lines.
    void setMajorEvery(int count);

protected:
    ~SoFCBackgroundGrid() override;

private:
    SoCamera* camera {nullptr};
    SbColor minorColor;
    SbColor majorColor;
    float opacity {1.0F};
    float minimumSpacing {14.0F};
    int majorEvery {5};
};

}  // namespace Gui

#endif  // GUI_SOFCBACKGROUNDGRID_H

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

#ifdef FC_OS_WIN32
# include <windows.h>
#endif

#ifdef FC_OS_MACOSX
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

#include <algorithm>
#include <cmath>

#include <Inventor/SbViewVolume.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/nodes/SoCamera.h>

#include "SoFCBackgroundGrid.h"
#include <Gui/SoDevicePixelRatioElement.h>

using namespace Gui;

SO_NODE_SOURCE(SoFCBackgroundGrid)

void SoFCBackgroundGrid::finish()
{
    atexit_cleanup();
}

SoFCBackgroundGrid::SoFCBackgroundGrid()
    : minorColor(0.847F, 0.918F, 0.973F)  // NOLINT
    , majorColor(0.765F, 0.867F, 0.945F)  // NOLINT
{
    SO_NODE_CONSTRUCTOR(SoFCBackgroundGrid);
}

SoFCBackgroundGrid::~SoFCBackgroundGrid()
{
    if (camera) {
        camera->unref();
    }
}

void SoFCBackgroundGrid::initClass()
{
    SO_NODE_INIT_CLASS(SoFCBackgroundGrid, SoNode, "Node");
}

void SoFCBackgroundGrid::setSceneCamera(SoCamera* cam)
{
    if (cam == camera) {
        return;
    }
    if (cam) {
        cam->ref();
    }
    if (camera) {
        camera->unref();
    }
    camera = cam;
}

void SoFCBackgroundGrid::setColors(const SbColor& minor, const SbColor& major)
{
    minorColor = minor;
    majorColor = major;
}

void SoFCBackgroundGrid::setOpacity(float value)
{
    opacity = std::clamp(value, 0.0F, 1.0F);
}

void SoFCBackgroundGrid::setMinimumSpacing(float pixels)
{
    minimumSpacing = std::max(pixels, 4.0F);  // NOLINT
}

void SoFCBackgroundGrid::setMajorEvery(int count)
{
    majorEvery = std::max(count, 2);
}

namespace
{

/// Returns the smallest value of the 1-2-5 sequence that is not below \a minimum.
double niceSpacing(double minimum)
{
    const double exponent = std::floor(std::log10(minimum));
    const double base = std::pow(10.0, exponent);
    for (double mantissa : {1.0, 2.0, 5.0, 10.0}) {  // NOLINT
        if (mantissa * base >= minimum) {
            return mantissa * base;
        }
    }
    return 10.0 * base;  // NOLINT
}

}  // namespace

void SoFCBackgroundGrid::GLRender(SoGLRenderAction* action)
{
    if (!camera || opacity <= 0.0F) {
        return;
    }

    SoState* state = action->getState();
    const SbViewportRegion& vp = SoViewportRegionElement::get(state);
    const SbVec2s size = vp.getViewportSizePixels();
    if (size[0] <= 0 || size[1] <= 0) {
        return;
    }

    // Corners of the viewport on the focal plane of the scene camera, in world coordinates
    const SbViewVolume vv = camera->getViewVolume(vp.getViewportAspectRatio());
    const float distance = camera->focalDistance.getValue();
    const SbVec3f origin = vv.getPlanePoint(distance, SbVec2f(0.0F, 0.0F));
    SbVec3f right = vv.getPlanePoint(distance, SbVec2f(1.0F, 0.0F)) - origin;
    SbVec3f up = vv.getPlanePoint(distance, SbVec2f(0.0F, 1.0F)) - origin;
    const double width = right.length();
    const double height = up.length();
    if (width <= 0.0 || height <= 0.0) {
        return;
    }
    right.normalize();
    up.normalize();

    // Visible range expressed in view plane coordinates relative to the world origin
    const double u0 = origin.dot(right);
    const double v0 = origin.dot(up);
    const double pixelsPerUnit = double(size[0]) / width;

    double ratio = SoDevicePixelRatioElement::get(state);
    if (ratio <= 0.0) {
        ratio = 1.0;
    }
    const double minPixels = minimumSpacing * ratio;
    const double minor = niceSpacing(minPixels / pixelsPerUnit);
    const double major = minor * majorEvery;

    // Fade the minor lines in while they approach the minimum spacing, so that switching
    // to the next step of the sequence does not pop.
    const double minorPixels = minor * pixelsPerUnit;
    const double fade = std::clamp((minorPixels - minPixels) / minPixels + 0.35, 0.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, size[0], 0.0, size[1], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(float(std::max(1.0, std::floor(ratio))));

    // Lines are snapped to pixel centers so they stay crisp at any zoom level.
    const auto drawLines = [&](double spacing, bool skipMajor) {
        const auto first = [&](double start) {
            return static_cast<long long>(std::ceil(start / spacing));
        };
        const long long uFirst = first(u0);
        const long long uLast = static_cast<long long>(std::floor((u0 + width) / spacing));
        const long long vFirst = first(v0);
        const long long vLast = static_cast<long long>(std::floor((v0 + height) / spacing));
        const long long mod = skipMajor ? majorEvery : 0;
        for (long long i = uFirst; i <= uLast; ++i) {
            if (mod && i % mod == 0) {
                continue;
            }
            const double x = std::floor((i * spacing - u0) * pixelsPerUnit) + 0.5;
            glVertex2d(x, 0.0);
            glVertex2d(x, size[1]);
        }
        for (long long j = vFirst; j <= vLast; ++j) {
            if (mod && j % mod == 0) {
                continue;
            }
            const double y = std::floor((j * spacing - v0) * pixelsPerUnit) + 0.5;
            glVertex2d(0.0, y);
            glVertex2d(size[0], y);
        }
    };

    glBegin(GL_LINES);
    glColor4f(minorColor[0], minorColor[1], minorColor[2], float(opacity * fade));
    drawLines(minor, true);
    glColor4f(majorColor[0], majorColor[1], majorColor[2], opacity);
    drawLines(major, false);
    glEnd();

    glPopAttrib();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

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

#ifndef GUI_DIALOG_DLGSETTINGSMODERNUI_H
#define GUI_DIALOG_DLGSETTINGSMODERNUI_H

#include <QByteArray>
#include <Gui/PropertyPage.h>

class QComboBox;
class QGroupBox;
class QLabel;

namespace Gui
{
class PrefCheckBox;
class PrefColorButton;
class PrefSpinBox;
class PrefWidget;

namespace Dialog
{

/**
 * Display > Modern UI: modern interface layer, bottom command panel and workspace background.
 */
class DlgSettingsModernUI: public PreferencePage
{
    Q_OBJECT

public:
    explicit DlgSettingsModernUI(QWidget* parent = nullptr);
    ~DlgSettingsModernUI() override;

    void saveSettings() override;
    void loadSettings() override;
    void resetSettingsToDefaults() override;

protected:
    void changeEvent(QEvent* event) override;

private:
    void retranslateUi();
    void onPresetActivated(int index);

    QGroupBox* interfaceGroup;
    QGroupBox* panelGroup;
    QGroupBox* workspaceGroup;
    PrefCheckBox* enabled;
    PrefCheckBox* modernIcons;
    QLabel* densityLabel;
    QComboBox* density;
    PrefCheckBox* panelVisible;
    PrefCheckBox* panelCollapsed;
    QLabel* presetLabel;
    QComboBox* preset;
    PrefCheckBox* gridVisible;
    QLabel* minorLabel;
    PrefColorButton* minorColor;
    QLabel* majorLabel;
    PrefColorButton* majorColor;
    QLabel* opacityLabel;
    PrefSpinBox* opacity;
    QLabel* spacingLabel;
    PrefSpinBox* spacing;
    QLabel* majorEveryLabel;
    PrefSpinBox* majorEvery;
    QLabel* restartNote;
    QList<PrefWidget*> prefWidgets;
    QByteArray pendingPreset;
    int loadedDensity {-1};
};

}  // namespace Dialog
}  // namespace Gui

#endif  // GUI_DIALOG_DLGSETTINGSMODERNUI_H

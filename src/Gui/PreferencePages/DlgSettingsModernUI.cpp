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

#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

#include <App/Application.h>
#include <Base/Color.h>

#include "DlgSettingsModernUI.h"
#include "../ModernUI/ModernUI.h"
#include "../ModernUI/WorkspaceAppearance.h"
#include "../PrefWidgets.h"

using namespace Gui;
using namespace Gui::Dialog;
using Gui::ModernUI::WorkspaceAppearance;

namespace
{
// Tree row spacing written to TreeView/ItemSpacing for the two densities
constexpr long compactSpacing = 0;  // FreeCAD's default
constexpr long comfortableSpacing = 8;

QColor fromPacked(std::uint32_t packed)
{
    return Base::Color::fromPackedRGBA<QColor>(packed);
}
}  // namespace

/* TRANSLATOR Gui::Dialog::DlgSettingsModernUI */

DlgSettingsModernUI::DlgSettingsModernUI(QWidget* parent)
    : PreferencePage(parent)
{
    auto layout = new QVBoxLayout(this);

    // Interface ---------------------------------------------------------------
    interfaceGroup = new QGroupBox(this);
    auto interfaceForm = new QFormLayout(interfaceGroup);
    enabled = new PrefCheckBox(interfaceGroup);
    enabled->setChecked(false);
    enabled->setEntryName("Enabled");
    enabled->setParamGrpPath("ModernUI");
    interfaceForm->addRow(enabled);
    modernIcons = new PrefCheckBox(interfaceGroup);
    modernIcons->setChecked(true);
    modernIcons->setEntryName("ModernIcons");
    modernIcons->setParamGrpPath("ModernUI");
    interfaceForm->addRow(modernIcons);
    densityLabel = new QLabel(interfaceGroup);
    density = new QComboBox(interfaceGroup);
    density->addItem(QString());
    density->addItem(QString());
    interfaceForm->addRow(densityLabel, density);
    layout->addWidget(interfaceGroup);

    // Command panel -----------------------------------------------------------
    panelGroup = new QGroupBox(this);
    auto panelForm = new QFormLayout(panelGroup);
    panelVisible = new PrefCheckBox(panelGroup);
    panelVisible->setChecked(true);
    panelVisible->setEntryName("CommandPanelVisible");
    panelVisible->setParamGrpPath("ModernUI");
    panelForm->addRow(panelVisible);
    panelCollapsed = new PrefCheckBox(panelGroup);
    panelCollapsed->setChecked(false);
    panelCollapsed->setEntryName("CommandPanelCollapsed");
    panelCollapsed->setParamGrpPath("ModernUI");
    panelForm->addRow(panelCollapsed);
    layout->addWidget(panelGroup);

    // Workspace ---------------------------------------------------------------
    workspaceGroup = new QGroupBox(this);
    auto workspaceForm = new QFormLayout(workspaceGroup);
    presetLabel = new QLabel(workspaceGroup);
    preset = new QComboBox(workspaceGroup);
    for (const auto& it : WorkspaceAppearance::presets()) {
        preset->addItem(it.text(), it.id);
    }
    workspaceForm->addRow(presetLabel, preset);

    gridVisible = new PrefCheckBox(workspaceGroup);
    gridVisible->setChecked(false);
    gridVisible->setEntryName("WorkspaceGrid");
    gridVisible->setParamGrpPath("View");
    workspaceForm->addRow(gridVisible);

    minorLabel = new QLabel(workspaceGroup);
    minorColor = new PrefColorButton(workspaceGroup);
    minorColor->setColor(fromPacked(0xD8EAF8FF));
    minorColor->setEntryName("WorkspaceGridMinorColor");
    minorColor->setParamGrpPath("View");
    workspaceForm->addRow(minorLabel, minorColor);

    majorLabel = new QLabel(workspaceGroup);
    majorColor = new PrefColorButton(workspaceGroup);
    majorColor->setColor(fromPacked(0xC3DDF1FF));
    majorColor->setEntryName("WorkspaceGridMajorColor");
    majorColor->setParamGrpPath("View");
    workspaceForm->addRow(majorLabel, majorColor);

    opacityLabel = new QLabel(workspaceGroup);
    opacity = new PrefSpinBox(workspaceGroup);
    opacity->setRange(0, 100);
    opacity->setSuffix(QStringLiteral(" %"));
    opacity->setValue(100);
    opacity->setEntryName("WorkspaceGridOpacity");
    opacity->setParamGrpPath("View");
    workspaceForm->addRow(opacityLabel, opacity);

    spacingLabel = new QLabel(workspaceGroup);
    spacing = new PrefSpinBox(workspaceGroup);
    spacing->setRange(6, 100);
    spacing->setSuffix(QStringLiteral(" px"));
    spacing->setValue(14);
    spacing->setEntryName("WorkspaceGridSpacing");
    spacing->setParamGrpPath("View");
    workspaceForm->addRow(spacingLabel, spacing);

    majorEveryLabel = new QLabel(workspaceGroup);
    majorEvery = new PrefSpinBox(workspaceGroup);
    majorEvery->setRange(2, 20);
    majorEvery->setValue(5);
    majorEvery->setEntryName("WorkspaceGridMajorEvery");
    majorEvery->setParamGrpPath("View");
    workspaceForm->addRow(majorEveryLabel, majorEvery);
    layout->addWidget(workspaceGroup);

    restartNote = new QLabel(this);
    restartNote->setWordWrap(true);
    layout->addWidget(restartNote);
    layout->addStretch();

    prefWidgets = {
        enabled, modernIcons, panelVisible, panelCollapsed, gridVisible,
        minorColor, majorColor, opacity, spacing, majorEvery,
    };

    connect(preset, qOverload<int>(&QComboBox::activated), this, &DlgSettingsModernUI::onPresetActivated);
    // Only the icon set needs a restart; the rest follows the settings right away.
    // "clicked" (not "toggled"), so that loading the stored values does not ask for a restart.
    connect(enabled, &QCheckBox::clicked, this, [this]() {
        if (modernIcons->isChecked()) {
            requireRestart();
        }
    });
    connect(modernIcons, &QCheckBox::clicked, this, [this]() { requireRestart(); });

    retranslateUi();
}

DlgSettingsModernUI::~DlgSettingsModernUI() = default;

void DlgSettingsModernUI::onPresetActivated(int index)
{
    const auto* p = WorkspaceAppearance::preset(preset->itemData(index).toByteArray());
    if (!p) {
        return;
    }
    // Show the grid values of the preset; they are written together with it on Apply/OK.
    pendingPreset = p->id;
    gridVisible->setChecked(p->grid);
    minorColor->setColor(fromPacked(p->minorColor));
    majorColor->setColor(fromPacked(p->majorColor));
}

void DlgSettingsModernUI::saveSettings()
{
    if (!pendingPreset.isEmpty()) {
        WorkspaceAppearance::applyPreset(pendingPreset);
        pendingPreset.clear();
    }

    for (PrefWidget* widget : prefWidgets) {
        widget->onSave();
    }

    // Only touch the tree spacing when the density was changed here, so that saving the
    // preferences for another reason keeps a custom TreeView/ItemSpacing value.
    if (density->currentIndex() != loadedDensity) {
        const bool comfortable = density->currentIndex() == 1;
        ModernUI::parameters()->SetInt("TreeDensity", comfortable ? 1 : 0);
        App::GetApplication()
            .GetParameterGroupByPath("User parameter:BaseApp/Preferences/TreeView")
            ->SetInt("ItemSpacing", comfortable ? comfortableSpacing : compactSpacing);
        loadedDensity = density->currentIndex();
    }
}

void DlgSettingsModernUI::loadSettings()
{
    for (PrefWidget* widget : prefWidgets) {
        widget->onRestore();
    }

    const long spacingValue = App::GetApplication()
                                  .GetParameterGroupByPath("User parameter:BaseApp/Preferences/TreeView")
                                  ->GetInt("ItemSpacing", 0);
    density->setCurrentIndex(
        int(ModernUI::parameters()->GetInt("TreeDensity", spacingValue >= comfortableSpacing ? 1 : 0))
    );
    loadedDensity = density->currentIndex();

    const int index = preset->findData(WorkspaceAppearance::currentPreset());
    preset->setCurrentIndex(index);
    pendingPreset.clear();
}

void DlgSettingsModernUI::resetSettingsToDefaults()
{
    PreferencePage::resetSettingsToDefaults();
    ModernUI::parameters()->RemoveInt("TreeDensity");
    WorkspaceAppearance::viewParameters()->RemoveASCII("WorkspacePreset");
}

void DlgSettingsModernUI::retranslateUi()
{
    setWindowTitle(tr("Modern UI"));
    interfaceGroup->setTitle(tr("Interface"));
    enabled->setText(tr("Enable the modern interface (command panel, command search)"));
    modernIcons->setText(tr("Use the modern icon set where available"));
    densityLabel->setText(tr("Model tree rows"));
    density->setItemText(0, tr("Compact"));
    density->setItemText(1, tr("Comfortable"));

    panelGroup->setTitle(tr("Bottom command panel"));
    panelVisible->setText(tr("Show the command panel in workbenches that provide one"));
    panelCollapsed->setText(tr("Collapse the command panel to its tabs"));

    workspaceGroup->setTitle(tr("Workspace background"));
    presetLabel->setText(tr("Preset"));
    for (int i = 0; i < preset->count(); ++i) {
        if (const auto* p = WorkspaceAppearance::preset(preset->itemData(i).toByteArray())) {
            preset->setItemText(i, p->text());
        }
    }
    gridVisible->setText(tr("Show engineering grid (visual only, does not affect snapping)"));
    minorLabel->setText(tr("Minor grid color"));
    majorLabel->setText(tr("Major grid color"));
    opacityLabel->setText(tr("Grid opacity"));
    spacingLabel->setText(tr("Minimum distance between minor lines"));
    majorEveryLabel->setText(tr("Major line every"));
    majorEvery->setSuffix(tr(" minor lines"));
    restartNote->setText(
        tr("Switching the modern icon set on or off takes effect after a restart. "
           "The theme itself is selected under Display > UI (\"FreeCAD Modern\").")
    );
}

void DlgSettingsModernUI::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    PreferencePage::changeEvent(event);
}

#include "moc_DlgSettingsModernUI.cpp"

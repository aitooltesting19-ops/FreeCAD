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

#ifndef GUI_MODERNUI_COMMANDSEARCH_H
#define GUI_MODERNUI_COMMANDSEARCH_H

#include <QLineEdit>
#include <QPointer>

#include <FCGlobal.h>

namespace Gui
{
class CommandCompleter;

/**
 * "Search commands..." field shown in the upper right corner of the main window.
 *
 * It searches all registered commands by name, menu text and tool tip (using the same
 * completer as the keyboard customization dialog) and runs the selected command through the
 * command manager, so it behaves exactly like the menu entry or toolbar button.
 * The Std_CommandSearch command (Ctrl+K by default) moves the focus into the field.
 */
class GuiExport CommandSearchBox: public QLineEdit
{
    Q_OBJECT

public:
    explicit CommandSearchBox(QWidget* parent = nullptr);

    /// Installs the search field next to the menu bar of the main window (once)
    static CommandSearchBox* install();
    static CommandSearchBox* instance();

    void activate();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void onCommandActivated(const QByteArray& name);
    void retranslate();
    QString shortcutText() const;

    CommandCompleter* completer;
    static QPointer<CommandSearchBox> _instance;
};

}  // namespace Gui

#endif  // GUI_MODERNUI_COMMANDSEARCH_H

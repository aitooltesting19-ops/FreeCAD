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

#ifndef GUI_MODERNUI_COMMANDPANEL_H
#define GUI_MODERNUI_COMMANDPANEL_H

#include <memory>

#include <QHash>
#include <QPointer>
#include <QToolButton>
#include <QWidget>

#include <FCGlobal.h>

class QDockWidget;
class QScrollArea;
class QStackedWidget;
class QTabBar;

namespace Gui
{
class ToolBarItem;

/**
 * Button of the command panel: icon above a label of up to two lines.
 *
 * The button is bound to the QAction of a registered command, so enabled state, check state,
 * tool tip, status tip and shortcut all come from the command system. Clicking it triggers the
 * action exactly like a toolbar button or menu entry would.
 */
class GuiExport CommandPanelButton: public QToolButton
{
    Q_OBJECT

public:
    CommandPanelButton(const QByteArray& command, QAction* action, QWidget* parent = nullptr);

    QByteArray command() const
    {
        return commandName;
    }
    void updateLabel();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool event(QEvent* event) override;

private:
    QStringList labelLines() const;

    QByteArray commandName;
    QString label;
};

/**
 * The widget of the bottom command panel: a row of tabs with a collapse button, and below it
 * one horizontally scrollable strip of command buttons per tab.
 */
class GuiExport CommandPanel: public QWidget
{
    Q_OBJECT

public:
    explicit CommandPanel(QWidget* parent = nullptr);
    ~CommandPanel() override;

    /// Rebuilds the tabs from \a root (one child item per tab). Returns false if it is empty.
    bool setup(const ToolBarItem* root);
    void retranslate();

    void setCollapsed(bool collapsed);
    bool isCollapsed() const
    {
        return collapsed;
    }

    QString currentTab() const;
    bool setCurrentTab(const QString& name);
    QStringList tabNames() const
    {
        return tabs;
    }
    /// Command names of the buttons on the tab \a name
    QList<QByteArray> commands(const QString& name) const;

protected:
    bool eventFilter(QObject* object, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    QWidget* createPage(const ToolBarItem* tab);
    void onCurrentChanged(int index);
    void updateHeight();
    void updateCollapseButton();

    QTabBar* tabBar;
    QToolButton* collapseButton;
    QStackedWidget* stack;
    QWidget* header;
    QStringList tabs;
    QString signature;
    QHash<QString, QWidget*> pages;  // cached pages, keyed by tab signature
    bool collapsed {false};
    bool updating {false};
};

/**
 * Owns the dock widget that hosts the CommandPanel and fills it from the active workbench.
 *
 * Workbenches declare the content with Workbench::setupCommandPanel() (C++) or
 * Workbench.appendCommandPanel(tab, commands) (Python), the same way they declare toolbars.
 * The panel is only shown while the active workbench defines at least one tab.
 */
class GuiExport CommandPanelManager
{
public:
    static CommandPanelManager* instance();
    static void destruct();

    void setup(const ToolBarItem* root);
    void retranslate();

    QDockWidget* dockWidget() const;
    CommandPanel* panel() const;

private:
    CommandPanelManager();
    ~CommandPanelManager();
    void createDockWidget();
    void updateVisibility();
    void updateCorners(bool fullWidth);
    void onParameterChanged(const QByteArray& name);

    QPointer<QDockWidget> dock;
    QPointer<CommandPanel> commandPanel;
    std::unique_ptr<ToolBarItem> content;  // copy of the active workbench's panel structure
    bool hasContent {false};
    bool cornersChanged {false};
    Qt::DockWidgetArea savedBottomLeft {Qt::BottomDockWidgetArea};
    Qt::DockWidgetArea savedBottomRight {Qt::BottomDockWidgetArea};
    static CommandPanelManager* _instance;
};

}  // namespace Gui

#endif  // GUI_MODERNUI_COMMANDPANEL_H

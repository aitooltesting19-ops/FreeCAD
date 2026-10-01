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

#include <algorithm>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDockWidget>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QStyleOptionToolButton>
#include <QTabBar>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <Base/Console.h>

#include "CommandPanel.h"
#include "ModernUI.h"
#include "../Action.h"
#include "../Application.h"
#include "../BitmapFactory.h"
#include "../Command.h"
#include "../MainWindow.h"
#include "../ToolBarManager.h"

using namespace Gui;

namespace
{
// Metrics in device independent pixels. Qt scales them for high DPI screens; colors and
// backgrounds come from the style sheet (see the CommandPanel section of the theme).
constexpr int iconExtent = 24;
constexpr int buttonMinWidth = 68;
constexpr int buttonMaxWidth = 92;
constexpr int buttonPaddingX = 6;
constexpr int buttonPaddingTop = 7;
constexpr int buttonPaddingBottom = 6;
constexpr int iconTextGap = 5;
constexpr int pageMarginX = 8;
constexpr int pageMarginY = 4;
constexpr const char* separatorName = "Separator";
}  // namespace

// ----------------------------------------------------------------------------

CommandPanelButton::CommandPanelButton(const QByteArray& command, QAction* action, QWidget* parent)
    : QToolButton(parent)
    , commandName(command)
{
    setObjectName(QStringLiteral("CommandPanelButton"));
    setAutoRaise(true);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    setIconSize(QSize(iconExtent, iconExtent));
    setDefaultAction(action);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateLabel();
}

void CommandPanelButton::updateLabel()
{
    const Command* cmd = Application::Instance->commandManager().getCommandByName(commandName.constData());
    label = cmd ? ModernUI::commandLabel(cmd) : QString::fromLatin1(commandName);
    setAccessibleName(label);
    updateGeometry();
    update();
}

QStringList CommandPanelButton::labelLines() const
{
    // Greedy word wrap into at most two lines; the second one is elided if needed.
    const QFontMetrics fm(font());
    const int maxWidth = buttonMaxWidth - 2 * buttonPaddingX;
    const QStringList words = label.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QStringList lines;
    QString current;
    for (const QString& word : words) {
        const QString candidate = current.isEmpty() ? word : current + QLatin1Char(' ') + word;
        if (current.isEmpty() || fm.horizontalAdvance(candidate) <= maxWidth || lines.size() == 1) {
            current = candidate;
        }
        else {
            lines << current;
            current = word;
        }
    }
    if (!current.isEmpty()) {
        lines << current;
    }
    for (QString& line : lines) {
        line = fm.elidedText(line, Qt::ElideRight, maxWidth);
    }
    return lines;
}

QSize CommandPanelButton::sizeHint() const
{
    const QFontMetrics fm(font());
    int textWidth = 0;
    for (const QString& line : labelLines()) {
        textWidth = std::max(textWidth, fm.horizontalAdvance(line));
    }
    const int width = std::clamp(textWidth + 2 * buttonPaddingX, buttonMinWidth, buttonMaxWidth);
    const int height = buttonPaddingTop + iconSize().height() + iconTextGap + 2 * fm.height()
        + buttonPaddingBottom;
    return {width, height};
}

QSize CommandPanelButton::minimumSizeHint() const
{
    return sizeHint();
}

void CommandPanelButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Background (hover, pressed, checked) comes from the style sheet through the style.
    if (opt.state & (QStyle::State_MouseOver | QStyle::State_Sunken | QStyle::State_On)) {
        style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &painter, this);
    }

    const bool enabled = isEnabled();
    const QIcon::Mode mode = !enabled ? QIcon::Disabled
        : (opt.state & QStyle::State_MouseOver) ? QIcon::Active
                                                : QIcon::Normal;
    const QIcon::State state = isChecked() ? QIcon::On : QIcon::Off;
    const QSize icon = iconSize();
    const QRect iconRect((width() - icon.width()) / 2, buttonPaddingTop, icon.width(), icon.height());
    this->icon().paint(&painter, iconRect, Qt::AlignCenter, mode, state);

    const QFontMetrics fm(font());
    painter.setPen(palette().color(enabled ? QPalette::Active : QPalette::Disabled, QPalette::ButtonText));
    int y = iconRect.bottom() + 1 + iconTextGap;
    for (const QString& line : labelLines()) {
        painter.drawText(QRect(0, y, width(), fm.height()), Qt::AlignHCenter | Qt::AlignTop, line);
        y += fm.height();
    }

    // Commands with alternatives (e.g. the arc tools) open a menu on press and hold;
    // a small corner mark tells the user.
    if (menu()) {
        painter.setRenderHint(QPainter::Antialiasing);
        const QPointF corner(iconRect.right() + 6, iconRect.bottom() + 1);
        QPainterPath mark;
        mark.moveTo(corner);
        mark.lineTo(corner - QPointF(5, 0));
        mark.lineTo(corner - QPointF(0, 5));
        mark.closeSubpath();
        painter.fillPath(mark, palette().color(QPalette::ButtonText));
    }
}

void CommandPanelButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        updateLabel();
    }
    else if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateGeometry();
    }
    QToolButton::changeEvent(event);
}

bool CommandPanelButton::event(QEvent* event)
{
    // The default action sets the button text; the label is drawn from the command instead.
    if (event->type() == QEvent::ActionChanged) {
        const bool res = QToolButton::event(event);
        update();
        return res;
    }
    return QToolButton::event(event);
}

// ----------------------------------------------------------------------------

CommandPanel::CommandPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("CommandPanel"));
    setAttribute(Qt::WA_StyledBackground);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    header = new QWidget(this);
    header->setObjectName(QStringLiteral("CommandPanelHeader"));
    header->setAttribute(Qt::WA_StyledBackground);
    auto headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(pageMarginX, 4, pageMarginX, 0);
    headerLayout->setSpacing(4);

    tabBar = new QTabBar(header);
    tabBar->setObjectName(QStringLiteral("CommandPanelTabBar"));
    tabBar->setExpanding(false);
    tabBar->setDrawBase(false);
    tabBar->setDocumentMode(true);
    tabBar->setUsesScrollButtons(true);
    tabBar->setFocusPolicy(Qt::TabFocus);
    headerLayout->addWidget(tabBar);
    headerLayout->addStretch();

    collapseButton = new QToolButton(header);
    collapseButton->setObjectName(QStringLiteral("CommandPanelCollapseButton"));
    collapseButton->setAutoRaise(true);
    collapseButton->setIconSize(QSize(16, 16));
    headerLayout->addWidget(collapseButton);

    stack = new QStackedWidget(this);
    stack->setObjectName(QStringLiteral("CommandPanelStack"));

    layout->addWidget(header);
    layout->addWidget(stack);

    connect(tabBar, &QTabBar::currentChanged, this, &CommandPanel::onCurrentChanged);
    connect(tabBar, &QTabBar::tabBarClicked, this, [this](int) {
        if (collapsed) {
            setCollapsed(false);
        }
    });
    connect(collapseButton, &QToolButton::clicked, this, [this]() {
        setCollapsed(!collapsed);
    });

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    collapsed = ModernUI::parameters()->GetBool("CommandPanelCollapsed", false);
    stack->setVisible(!collapsed);
    updateCollapseButton();
    updateHeight();
}

CommandPanel::~CommandPanel() = default;

bool CommandPanel::setup(const ToolBarItem* root)
{
    QStringList names;
    QStringList pageKeys;
    QList<const ToolBarItem*> items;
    for (const ToolBarItem* tab : root->getItems()) {
        if (!tab->hasItems()) {
            continue;
        }
        QString key = QString::fromStdString(tab->command()) + QLatin1Char(':');
        for (const ToolBarItem* cmd : tab->getItems()) {
            key += QString::fromStdString(cmd->command()) + QLatin1Char(',');
        }
        names << QString::fromStdString(tab->command());
        pageKeys << key;
        items << tab;
    }

    const QString newSignature = pageKeys.join(QLatin1Char('|'));
    if (newSignature == signature) {
        return !tabs.isEmpty();  // same content, nothing to rebuild
    }

    updating = true;
    signature = newSignature;
    tabs = names;

    while (tabBar->count() > 0) {
        tabBar->removeTab(0);
    }
    while (stack->count() > 0) {
        stack->removeWidget(stack->widget(0));  // pages stay cached for later reuse
    }

    for (int i = 0; i < items.size(); ++i) {
        QWidget* page = pages.value(pageKeys[i]);
        if (!page) {
            page = createPage(items[i]);
            pages.insert(pageKeys[i], page);
        }
        stack->addWidget(page);
        tabBar->addTab(QApplication::translate("Workbench", names[i].toUtf8().constData()));
    }
    updating = false;

    // Restore the tab the user worked with last, if this workbench has it
    const QString last = QString::fromStdString(ModernUI::parameters()->GetASCII("CommandPanelTab"));
    if (!setCurrentTab(last) && tabBar->count() > 0) {
        tabBar->setCurrentIndex(0);
        stack->setCurrentIndex(0);
    }

    updateHeight();
    return !tabs.isEmpty();
}

QWidget* CommandPanel::createPage(const ToolBarItem* tab)
{
    auto scroll = new QScrollArea(this);
    scroll->setObjectName(QStringLiteral("CommandPanelScrollArea"));
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->viewport()->installEventFilter(this);

    auto page = new QWidget(scroll);
    page->setObjectName(QStringLiteral("CommandPanelPage"));
    auto layout = new QHBoxLayout(page);
    layout->setContentsMargins(pageMarginX, pageMarginY, pageMarginX, pageMarginY);
    layout->setSpacing(2);

    auto& manager = Application::Instance->commandManager();
    bool pendingSeparator = false;
    for (const ToolBarItem* item : tab->getItems()) {
        const std::string& name = item->command();
        if (name == separatorName) {
            pendingSeparator = layout->count() > 0;
            continue;
        }

        Command* cmd = manager.getCommandByName(name.c_str());
        if (!cmd) {
            Base::Console().log("Command panel: unknown command '%s' skipped\n", name.c_str());
            continue;
        }
        cmd->initAction();
        Action* action = cmd->getAction();
        if (!action || !action->action()) {
            continue;
        }

        if (pendingSeparator) {
            auto line = new QFrame(page);
            line->setObjectName(QStringLiteral("CommandPanelSeparator"));
            line->setFrameShape(QFrame::VLine);
            line->setFixedWidth(1);
            layout->addSpacing(4);
            layout->addWidget(line);
            layout->addSpacing(4);
            pendingSeparator = false;
        }

        auto button = new CommandPanelButton(QByteArray(name.c_str()), action->action(), page);
        if (auto group = qobject_cast<ActionGroup*>(action)) {
            // Group commands (e.g. arc or array tools): click runs the current tool,
            // press and hold lists the alternatives.
            auto menu = new QMenu(button);
            menu->addActions(group->actions());
            button->setMenu(menu);
            button->setPopupMode(QToolButton::DelayedPopup);
        }
        layout->addWidget(button, 0, Qt::AlignTop);
    }
    layout->addStretch();

    scroll->setWidget(page);
    return scroll;
}

void CommandPanel::retranslate()
{
    for (int i = 0; i < tabBar->count() && i < tabs.size(); ++i) {
        tabBar->setTabText(i, QApplication::translate("Workbench", tabs[i].toUtf8().constData()));
    }
    updateCollapseButton();
}

QString CommandPanel::currentTab() const
{
    const int index = tabBar->currentIndex();
    return index >= 0 && index < tabs.size() ? tabs[index] : QString();
}

bool CommandPanel::setCurrentTab(const QString& name)
{
    const int index = tabs.indexOf(name);
    if (index < 0) {
        return false;
    }
    tabBar->setCurrentIndex(index);
    stack->setCurrentIndex(index);
    return true;
}

QList<QByteArray> CommandPanel::commands(const QString& name) const
{
    QList<QByteArray> result;
    const int index = tabs.indexOf(name);
    if (index < 0 || index >= stack->count()) {
        return result;
    }
    for (auto button : stack->widget(index)->findChildren<CommandPanelButton*>()) {
        result << button->command();
    }
    return result;
}

void CommandPanel::onCurrentChanged(int index)
{
    if (index < 0 || index >= stack->count()) {
        return;
    }
    stack->setCurrentIndex(index);
    if (!updating) {
        ModernUI::parameters()->SetASCII("CommandPanelTab", tabs[index].toStdString());
    }
}

void CommandPanel::setCollapsed(bool on)
{
    if (collapsed == on) {
        return;
    }
    collapsed = on;
    stack->setVisible(!collapsed);
    ModernUI::parameters()->SetBool("CommandPanelCollapsed", collapsed);
    updateCollapseButton();
    updateHeight();
}

void CommandPanel::updateCollapseButton()
{
    collapseButton->setIcon(BitmapFactory().iconFromTheme(
        collapsed ? "ModernUI_ChevronUp" : "ModernUI_ChevronDown"
    ));
    collapseButton->setToolTip(collapsed ? tr("Expand the command panel") : tr("Collapse the command panel"));
    collapseButton->setProperty("collapsed", collapsed);
}

void CommandPanel::updateHeight()
{
    // The panel has a fixed height: header only when collapsed, header plus one row of
    // buttons (and room for the horizontal scroll bar) when expanded.
    int height = header->sizeHint().height();
    if (!collapsed) {
        const QFontMetrics fm(font());
        const int buttonHeight = buttonPaddingTop + iconExtent + iconTextGap + 2 * fm.height()
            + buttonPaddingBottom;
        const int scrollBar = style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, this);
        height += buttonHeight + 2 * pageMarginY + scrollBar + 2;
    }
    setFixedHeight(height);
}

bool CommandPanel::eventFilter(QObject* object, QEvent* event)
{
    // Let the mouse wheel scroll the button strip horizontally when it overflows
    if (event->type() == QEvent::Wheel) {
        if (auto area = qobject_cast<QAbstractScrollArea*>(object->parent())) {
            auto wheel = static_cast<QWheelEvent*>(event);
            QScrollBar* bar = area->horizontalScrollBar();
            if (bar && bar->maximum() > 0) {
                const QPoint delta = wheel->angleDelta();
                const int step = delta.x() != 0 ? delta.x() : delta.y();
                bar->setValue(bar->value() - step);
                return true;
            }
        }
    }
    return QWidget::eventFilter(object, event);
}

void CommandPanel::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateHeight();
    }
    QWidget::changeEvent(event);
}

// ----------------------------------------------------------------------------

CommandPanelManager* CommandPanelManager::_instance = nullptr;

CommandPanelManager* CommandPanelManager::instance()
{
    if (!_instance) {
        _instance = new CommandPanelManager;
    }
    return _instance;
}

void CommandPanelManager::destruct()
{
    delete _instance;
    _instance = nullptr;
}

CommandPanelManager::CommandPanelManager() = default;

void CommandPanelManager::createDockWidget()
{
    MainWindow* mw = getMainWindow();
    if (!mw || dock) {
        return;
    }

    dock = new QDockWidget(mw);
    dock->setObjectName(QStringLiteral("Std_CommandPanel"));
    dock->setWindowTitle(QDockWidget::tr("Command Panel"));
    dock->setFeatures(QDockWidget::DockWidgetClosable);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    dock->setTitleBarWidget(new QWidget(dock));  // the panel has its own header

    commandPanel = new CommandPanel(dock);
    dock->setWidget(commandPanel);

    // Let the panel span the full width of the window, below the side panels
    mw->setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
    mw->setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);
    mw->addDockWidget(Qt::BottomDockWidgetArea, dock);
    mw->restoreDockWidget(dock);

    // Only an explicit toggle by the user (View > Panels) changes the stored visibility;
    // hiding the panel for workbenches without commands does not.
    QAction* toggle = dock->toggleViewAction();
    QObject::connect(toggle, &QAction::triggered, [](bool checked) {
        ModernUI::parameters()->SetBool("CommandPanelVisible", checked);
    });
}

void CommandPanelManager::setup(const ToolBarItem* root)
{
    const bool enabled = ModernUI::parameters()->GetBool("Enabled", true);
    hasContent = enabled && root && root->hasItems();
    if (hasContent) {
        createDockWidget();
        if (commandPanel) {
            hasContent = commandPanel->setup(root);
        }
    }
    updateVisibility();
}

void CommandPanelManager::updateVisibility()
{
    if (!dock) {
        return;
    }
    const bool visible = ModernUI::parameters()->GetBool("CommandPanelVisible", true);
    dock->toggleViewAction()->setVisible(hasContent);
    dock->setVisible(hasContent && visible);
}

void CommandPanelManager::retranslate()
{
    if (dock) {
        dock->setWindowTitle(QDockWidget::tr("Command Panel"));
    }
    if (commandPanel) {
        commandPanel->retranslate();
    }
}

QDockWidget* CommandPanelManager::dockWidget() const
{
    return dock;
}

CommandPanel* CommandPanelManager::panel() const
{
    return commandPanel;
}

#include "moc_CommandPanel.cpp"

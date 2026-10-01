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

#include <QAbstractItemView>
#include <QAction>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenuBar>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QTimer>

#include "CommandSearch.h"
#include "../Application.h"
#include "../BitmapFactory.h"
#include "../Command.h"
#include "../CommandCompleter.h"
#include "../MainWindow.h"
#include "../MDIView.h"

using namespace Gui;

QPointer<CommandSearchBox> CommandSearchBox::_instance;

CommandSearchBox::CommandSearchBox(QWidget* parent)
    : QLineEdit(parent)
{
    setObjectName(QStringLiteral("CommandSearchBox"));
    setClearButtonEnabled(true);
    addAction(BitmapFactory().iconFromTheme("ModernUI_Search"), QLineEdit::LeadingPosition);
    setMinimumWidth(240);
    setMaximumWidth(300);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    completer = new CommandCompleter(this, this);
    completer->setSearchToolTips(true);
    // Show the matches without replacing the typed text while browsing them
    disconnect(
        completer,
        qOverload<const QString&>(&QCompleter::highlighted),
        this,
        &QLineEdit::setText
    );
    connect(completer, &CommandCompleter::commandActivated, this, &CommandSearchBox::onCommandActivated);
    connect(this, &QLineEdit::textChanged, this, &CommandSearchBox::updateHintMargin);

    retranslate();
}

CommandSearchBox* CommandSearchBox::install()
{
    if (_instance) {
        return _instance;
    }
    MainWindow* mw = getMainWindow();
    QMenuBar* mb = mw ? mw->menuBar() : nullptr;
    if (!mb) {
        return nullptr;
    }

    // Keep the existing corner area (it can host toolbars) and put the search field before it
    auto container = new QWidget(mb);
    container->setObjectName(QStringLiteral("MenuBarSearchArea"));
    auto layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 2, 6, 2);
    layout->setSpacing(6);
    _instance = new CommandSearchBox(container);
    layout->addWidget(_instance);
    if (QWidget* corner = mb->cornerWidget(Qt::TopRightCorner)) {
        layout->addWidget(corner);
        corner->show();
    }
    mb->setCornerWidget(container, Qt::TopRightCorner);
    container->show();
    return _instance;
}

CommandSearchBox* CommandSearchBox::instance()
{
    return _instance;
}

void CommandSearchBox::activate()
{
    setFocus(Qt::ShortcutFocusReason);
    selectAll();
}

void CommandSearchBox::onCommandActivated(const QByteArray& name)
{
    completer->popup()->hide();
    clear();
    clearFocus();
    // Run after the completer has finished processing the event that selected the command
    QTimer::singleShot(0, [name]() {
        Application::Instance->commandManager().runCommandByName(name.constData());
    });
}

QString CommandSearchBox::shortcutText() const
{
    const Command* cmd = Application::Instance->commandManager().getCommandByName("Std_CommandSearch");
    const QString shortcut = cmd ? cmd->getShortcut() : QString();
    return QKeySequence(shortcut).toString(QKeySequence::NativeText);
}

void CommandSearchBox::retranslate()
{
    setPlaceholderText(tr("Search commands..."));
    setToolTip(tr("Search all commands by name, menu text or tool tip, and run the selected one"));
    updateHintMargin();
}

void CommandSearchBox::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    // Show the keyboard shortcut at the right end while the field is idle
    if (!hasFocus() && text().isEmpty()) {
        const QString hint = shortcutText();
        if (hint.isEmpty()) {
            return;
        }
        QPainter painter(this);
        QStyleOptionFrame opt;
        initStyleOption(&opt);
        const QRect area = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
        QColor color = palette().color(QPalette::Disabled, QPalette::Text);
        painter.setPen(color);
        painter.drawText(area.adjusted(0, 0, -4, 0), Qt::AlignRight | Qt::AlignVCenter, hint);
    }
}

void CommandSearchBox::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && text().isEmpty()) {
        clearFocus();
        if (MainWindow* mw = getMainWindow()) {
            if (QWidget* view = mw->activeWindow()) {
                view->setFocus();
            }
        }
        return;
    }
    QLineEdit::keyPressEvent(event);
}

void CommandSearchBox::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    updateHintMargin();
}

void CommandSearchBox::focusOutEvent(QFocusEvent* event)
{
    QLineEdit::focusOutEvent(event);
    updateHintMargin();
    update();
}

void CommandSearchBox::updateHintMargin()
{
    // Reserve room for the shortcut hint, so that the placeholder text does not run into it
    const bool idle = !hasFocus() && text().isEmpty();
    const QString hint = idle ? shortcutText() : QString();
    const int right = hint.isEmpty() ? 0 : fontMetrics().horizontalAdvance(hint) + 10;
    const QMargins margins = textMargins();
    if (margins.right() != right) {
        setTextMargins(margins.left(), margins.top(), right, margins.bottom());
    }
}

void CommandSearchBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
    QLineEdit::changeEvent(event);
}

#include "moc_CommandSearch.cpp"

//-----------------------------------------------------------------------------
//  @file   widgetpopup.cpp
//  @author Douglas S Caskey
//  @date   2 Apr, 2026
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   vwidgetpopup.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   Feb 2, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "vwidgetpopup.h"

#include <QScreen>
#include <QGuiApplication>
#include <QFont>
#include <QLabel>
#include <QLayout>
#include <QMessageLogger>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QApplication>
#include <QScreen>

#include "../vmisc/def.h"

//---------------------------------------------------------------------------------------------------------------------
///  @brief Class showing a widget as popup window.
///
///  @param parent
///
///  @details
///  - If parent not specified (default), then popup widget gets
///    attribute Qt::WA_DeleteOnClose and will be deleted after close.
///  - setParent() function allows you to specify the widget to be popped up.
///    After widget is set, you normally should call show() slot in order to pop the
///    widget up at the specified global position.
///  - WidgetPopup takes care about positioning of your widget on the screen so it will
///    be always visible even if popped beside.
//---------------------------------------------------------------------------------------------------------------------
WidgetPopup::WidgetPopup(QWidget *parent)
    : QFrame(parent, Qt::Popup)
    , m_parent(nullptr)
    , m_owned(true)
    , m_oldParent(nullptr)
    , m_duration(-1)
{
    setAttribute(Qt::WA_WindowPropagation);

    if (parentWidget() == nullptr)
    {
        setAttribute(Qt::WA_DeleteOnClose);
    }

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(10, 10, 10, 10);
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief setParent Set popup parent
///
///  This method sets the parent of the popup.
///
///  @param parent
///  @param owned
///
///  @details
///  - If owned is true then the widget will be reparented to the popup widget.
//---------------------------------------------------------------------------------------------------------------------
void WidgetPopup::setParent(QWidget *parent, bool owned)
{
    if (m_parent)
    {
        layout()->removeWidget(m_parent);

        if (m_owned)
        {
            m_parent->setParent(nullptr);
            delete m_parent;
        }
        else
        {
            m_parent->setParent(m_oldParent);
        }
    }

    m_parent = parent;
    m_owned = owned;
    m_oldParent = nullptr;

    if (m_parent)
    {
        m_oldParent = m_parent->parentWidget();
        m_parent->setParent(this);
        layout()->addWidget(m_parent);
    }
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief popupMessage Creat popup message
///
///  This methaod createa  popup message of a given duratio in ms.
///
///  @param parent
///  @param msg
///  @param duration
//---------------------------------------------------------------------------------------------------------------------
void WidgetPopup::popupMessage(const QString &msg, const int &duration, QWidget *parent)
{
    SCASSERT(parent != nullptr)

    WidgetPopup *popup = new WidgetPopup(parent);
    QLabel *label = new QLabel(msg);
    QFont font = label->font();
    font.setBold(true);
    font.setPixelSize(16);
    label->setFont(font);
    popup->setParent(label);
    popup->setDuration(duration);
    popup->show(parent->frameGeometry().center());
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief show show popup message
///
///  This method shows a popupmesage at the given coordinates
///
///  @param coord
//---------------------------------------------------------------------------------------------------------------------
void WidgetPopup::show(QPoint coord)
{
    // important to do this before following adjustments!
    QFrame::show();

    const QRect screen(QGuiApplication::primaryScreen()->availableGeometry());
    coord.setX(coord.x() - width()/2);

    if (coord.x() < screen.x())
    {
        coord.setX(screen.x());
    }

    if (coord.y() < screen.y())
    {
        coord.setY(screen.y());
    }

    if (coord.x() > (screen.right()-width()))
    {
        coord.setX(screen.right()-width());
    }

    if (coord.y() > (screen.bottom()-height()))
    {
        coord.setY(screen.bottom()-height());
    }
    move(coord);

    if (m_duration > 0)
    {
        QTimer::singleShot(m_duration, this, SLOT(close()));
    }
}

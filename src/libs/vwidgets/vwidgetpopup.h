//-----------------------------------------------------------------------------
//  @file   widgetpopup.h
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
//  @file   vwidgetpopup.h
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

#ifndef WIDGET_POPUP_H
#define WIDGET_POPUP_H

#include <QFrame>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

class WidgetPopup : public QFrame
{
    Q_OBJECT

public:
    explicit    WidgetPopup(QWidget *parent = nullptr);
    void        setParent(QWidget *widget, bool owned = true);
    QWidget*    getParent() const;  /// Returns widget to be popped up.
    bool        isOwned() const;    /// Returns true if widget is owned by this popup widget, false otherwise.
    int         getDuration() const;
    void        setDuration(int duraton);

    static void popupMessage(const QString &msg, const int &duration, QWidget *parent);

public slots:
    void        show(QPoint coord);

protected:
    Q_DISABLE_COPY(WidgetPopup)
    QWidget    *m_parent;
    bool        m_owned;
    QWidget    *m_oldParent;
    int         m_duration;
};

//---------------------------------------------------------------------------------------------------------------------
///  @brief getParent Get popup parent
///
///  This method gets the parent of the popup.
///
///  @return parent
//---------------------------------------------------------------------------------------------------------------------
inline QWidget *WidgetPopup::getParent() const
{
    return m_parent;
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief isOwned Get if popup owns itself
///
///  This method gets whtjer the popup is it's own parent.
///
///  @return isowned
//---------------------------------------------------------------------------------------------------------------------
inline bool WidgetPopup::isOwned() const
{
    return m_owned;
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief getDuration Get popup duration
///
///  This method gets the duration of the popup.
///
///  @return duration
//---------------------------------------------------------------------------------------------------------------------
inline int WidgetPopup::getDuration() const
{
    return m_duration;
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief setDuration Set popup duration
///
///  This method sets the duration of the popupin ms.
///
///  @param duraton
//---------------------------------------------------------------------------------------------------------------------
inline void WidgetPopup::setDuration(int duraton)
{
    m_duration = duraton;
}

#endif // WIDGET_POPUP_H

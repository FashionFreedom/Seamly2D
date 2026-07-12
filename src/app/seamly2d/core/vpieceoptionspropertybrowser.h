/******************************************************************************
*   @file   vpieceoptionspropertybrowser.h
**  @author Douglas S Caskey
**  @date   27 Jun, 2026
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2026 Seamly2D project
**  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
**
**  Seamly2D is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Seamly2D is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef VPIECEOPTIONSPROPERTYBROWSER_H
#define VPIECEOPTIONSPROPERTYBROWSER_H

#include <QObject>
#include <QMap>

#include "../vpropertyexplorer/vproperty.h"
#include "../vpropertyexplorer/vpropertymodel.h"
#include "../vpropertyexplorer/vpropertyformview.h"

class QDockWidget;
class QGraphicsItem;
class QScrollArea;
class VAbstractPattern;

class VPieceOptionsPropertyBrowser : public QObject
{
    Q_OBJECT
public:
    explicit VPieceOptionsPropertyBrowser(VAbstractPattern *doc, QDockWidget *parent);
    void     clearPropertyBrowser();

    QScrollArea *scrollArea() const;

public slots:
    void itemClicked(QGraphicsItem *item);
    void updateOptions();

private slots:
    void userChangedData(VPE::VProperty *property);

private:
    Q_DISABLE_COPY(VPieceOptionsPropertyBrowser)

    VAbstractPattern                *m_doc;
    VPE::VPropertyModel             *m_propertyModel;
    VPE::VPropertyFormView          *m_formView;
    QScrollArea                     *m_scrollArea;

    QGraphicsItem                   *m_currentItem;
    QMap<VPE::VProperty *, QString>  m_propertyToId;
    QMap<QString, VPE::VProperty *>  m_idToProperty;

    void addProperty(VPE::VProperty *property, const QString &id);
    void showPieceOptions(QGraphicsItem *item);
    void updatePieceOptions();
    void changePieceData(VPE::VProperty *property);
};

#endif // VPIECEOPTIONSPROPERTYBROWSER_H

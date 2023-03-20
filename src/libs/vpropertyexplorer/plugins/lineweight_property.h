/******************************************************************************
 *   @file   lineweight_property.h
 **  @author DS Caskey
 **  @date   Feb 7, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

#ifndef LINEWEIGHT_PROPERTY_H
#define LINEWEIGHT_PROPERTY_H

#include <qcompilerdetection.h>
#include <QIcon>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QVector>
#include <Qt>
#include <QtGlobal>

#include "../vproperty.h"

namespace VPE
{

class LineWeightProperty : public VProperty
{
    Q_OBJECT
public:
    //! Constructor
    explicit               LineWeightProperty(const QString &name);

    //! Destructor
    virtual               ~LineWeightProperty() Q_DECL_OVERRIDE {}

    //! Get the data how it should be displayed
    virtual QVariant       data (int column = DPC_Name, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    //! Returns an editor widget, or NULL if it doesn't supply one
    //! \param parent The widget to which the editor will be added as a child
    //! \options Render options
    //! \delegate A pointer to the QAbstractItemDelegate requesting the editor. This can be used to connect signals and
    //! slots.
    virtual QWidget       *createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                  const QAbstractItemDelegate *delegate) Q_DECL_OVERRIDE;

    //! Gets the data from the widget
    virtual QVariant       getEditorData(const QWidget *editor) const Q_DECL_OVERRIDE;

    //! Sets the line styles
    virtual void           setLineWeights(const QMap<QString, QString> &weights);

    //! Get the settings. This function has to be implemented in a subclass in order to have an effect
    virtual QMap<QString, QString> getLineWeights() const;

    //! Sets the value of the property
    virtual void           setValue(const QVariant &value) Q_DECL_OVERRIDE;

    //! Returns a string containing the type of the property
    virtual QString        type() const Q_DECL_OVERRIDE;

    //! Clones this property
    //! \param include_children Indicates whether to also clone the children
    //! \param container If a property is being passed here, no new VProperty is being created but instead it is tried
    //! to fill all the data into container. This can also be used when subclassing this function.
    //! \return Returns the newly created property (or container, if it was not NULL)
    Q_REQUIRED_RESULT virtual VProperty *clone(bool include_children = true,
                                               VProperty *container = nullptr) const Q_DECL_OVERRIDE;

    static int             indexOfLineWeight(const QMap<QString, QString> &lineWeights, const QString &weight);

public slots:
    void                   currentIndexChanged(int index);

protected:
    QMap<QString, QString> m_lineWeights;
    QVector<QString>       m_indexList;
    int                    m_iconWidth;
    int                    m_iconHeight;

private:
    QIcon                  createIcon(const qreal &width);
    Q_DISABLE_COPY(LineWeightProperty)
};

}

#endif // LINEWEIGHT_PROPERTY_H

/************************************************************************
 **
 **  @file   plaintext_property.h
 **  @author DS Caskey
 **  @date   Feb 18, 2023
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

#ifndef PLAINTEXT_PROPERTY_H
#define PLAINTEXT_PROPERTY_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QtGlobal>

#include "../vproperty.h"

namespace VPE
{


//! Class for holding a plain text property
class PlainTextProperty : public VProperty
{
    Q_OBJECT
public:
                        PlainTextProperty(const QString &name, const QMap<QString, QVariant> &settings);

    explicit            PlainTextProperty(const QString &name);

    virtual QWidget    *createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                  const QAbstractItemDelegate *delegate) Q_DECL_OVERRIDE;


    virtual QVariant     getEditorData(const QWidget *editor) const Q_DECL_OVERRIDE;

    void                 setReadOnly(bool readOnly);
    void                 setOsSeparator(bool separator);
    void                 setClearButtonEnable(bool value);


    virtual void        setSetting(const QString &key, const QVariant &value) Q_DECL_OVERRIDE;
    virtual QVariant    getSetting(const QString &key) const Q_DECL_OVERRIDE;
    virtual QStringList getSettingKeys() const Q_DECL_OVERRIDE;

    virtual QString     type() const Q_DECL_OVERRIDE;

    Q_REQUIRED_RESULT virtual VProperty *clone(bool include_children = true,
                                               VProperty *container = nullptr) const Q_DECL_OVERRIDE;

    virtual void        updateParent(const QVariant &value) Q_DECL_OVERRIDE;

    int                 getTypeForParent() const;
    void                setTypeForParent(int value);

protected:
    bool                m_readOnly;
    int                 m_typeForParent;
    bool                m_osSeparator;

    virtual bool        eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(PlainTextProperty)
};

}
#endif // PLAINTEXT_PROPERTY_H

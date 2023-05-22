/******************************************************************************
 *   @file   vlabelproperty.cpp
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

/************************************************************************
 **
 **  @file   vlabelproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vlabelproperty.h"

#include <QKeyEvent>
#include <QLatin1String>
#include <QLabel>
#include <QSizePolicy>
#include <QString>

#include "../vproperty_p.h"

VPE::VLabelProperty::VLabelProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : VProperty(name, QVariant::String)
    , typeForParent(0)
{
    VProperty::setSettings(settings);
    d_ptr->VariantValue.setValue(QString());
    d_ptr->VariantValue.convert(QVariant::String);
}

VPE::VLabelProperty::VLabelProperty(const QString &name)
    : VProperty(name)
    , typeForParent(0)
{
    d_ptr->VariantValue.setValue(QString());
    d_ptr->VariantValue.convert(QVariant::String);
}

QWidget *VPE::VLabelProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                           const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    QLabel *labelTextEditor = new QLabel(parent);
    labelTextEditor->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    labelTextEditor->setLocale(parent->locale());
    labelTextEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    labelTextEditor->setText(d_ptr->VariantValue.toString());

    d_ptr->editor = labelTextEditor;
    return d_ptr->editor;
}

QVariant VPE::VLabelProperty::getEditorData(const QWidget *editor) const
{
    const QLabel *labelTextEditor = qobject_cast<const QLabel*>(editor);
    if (labelTextEditor)
    {
        return labelTextEditor->text();
    }

    return QVariant(QString());
}

void VPE::VLabelProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("TypeForParent"))
    {
        setTypeForParent(value.toInt());
    }
}

QVariant VPE::VLabelProperty::getSetting(const QString &key) const
{
    if (key == QLatin1String("TypeForParent"))
    {
        return typeForParent;
    }
    else
        return VProperty::getSetting(key);
}

QStringList VPE::VLabelProperty::getSettingKeys() const
{
    QStringList settings;
    settings << QStringLiteral("TypeForParent");
    return settings;
}

QString VPE::VLabelProperty::type() const
{
    return QStringLiteral("label");
}

VPE::VProperty *VPE::VLabelProperty::clone(bool include_children, VPE::VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new VLabelProperty(getName(), getSettings()));
}

void VPE::VLabelProperty::updateParent(const QVariant &value)
{
    emit childChanged(value, typeForParent);
}

int VPE::VLabelProperty::getTypeForParent() const
{
    return typeForParent;
}

void VPE::VLabelProperty::setTypeForParent(int value)
{
    typeForParent = value;
}

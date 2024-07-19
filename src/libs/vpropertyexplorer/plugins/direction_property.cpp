//  @file   direction_property.cpp
// @author DS Caskey
// @date   30 Jun, 2024
//
// @brief
// @copyright
// This source code is part of the Seamly2D project, a pattern making
// program, whose allow create and modeling patterns of clothing.
// Copyright (C) 2017-2024 Seamly2D project
// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.


#include "direction_property.h"

#include <QBrush>
#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSize>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QWidget>

#include "../ifc/ifcdef.h"
#include "../vproperty_p.h"

VPE::DirectionProperty::DirectionProperty(const QString &name)
    : VProperty(name, QVariant::Int)
    , m_directions()
    , m_indexList()
{
    VProperty::d_ptr->VariantValue = 0;
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
}

QVariant VPE::DirectionProperty::data(int column, int role) const
{
    if (m_directions.empty())
    {
        return QVariant();
    }

    int tmpIndex = VProperty::d_ptr->VariantValue.toInt();

    if (tmpIndex < 0 || tmpIndex >= m_indexList.count())
    {
        tmpIndex = 0;
    }

    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return m_indexList.at(tmpIndex);
    }
    else if (column == DPC_Data && Qt::EditRole == role)
    {
        return tmpIndex;
    }
    else
    {
        return VProperty::data(column, role);
    }
}

QWidget *VPE::DirectionProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                               const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QComboBox* directionEditor = new QComboBox(parent);

    directionEditor->clear();
    directionEditor->setMinimumWidth(140);
    directionEditor->setLocale(parent->locale());

    QMap<QString, QString>::const_iterator i = m_directions.constBegin();
    while (i != m_directions.constEnd())
    {
        directionEditor->addItem(i.value(), QVariant(i.key()));
        ++i;
    }
    directionEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(directionEditor, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DirectionProperty::currentIndexChanged);

    VProperty::d_ptr->editor = directionEditor;
    return VProperty::d_ptr->editor;
}

QVariant VPE::DirectionProperty::getEditorData(const QWidget *editor) const
{
    const QComboBox* directionEditor = qobject_cast<const QComboBox*>(editor);
    if (directionEditor)
    {
        return directionEditor->currentIndex();
    }

    return QVariant(0);
}

void VPE::DirectionProperty::setDirections(const QMap<QString, QString> &directions)
{
    this->m_directions = directions;
    m_indexList.clear();
    QMap<QString, QString>::const_iterator i = m_directions.constBegin();
    while (i != directions.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
}

QMap<QString, QString> VPE::DirectionProperty::getDirections() const
{
    return m_directions;
}

void VPE::DirectionProperty::setValue(const QVariant &value)
{
    int tmpIndex = value.toInt();

    if (tmpIndex < 0 || tmpIndex >= m_indexList.count())
    {
        tmpIndex = 0;
    }

    VProperty::d_ptr->VariantValue = tmpIndex;
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);

    if (VProperty::d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::d_ptr->editor);
    }
}

QString VPE::DirectionProperty::type() const
{
    return QStringLiteral("direction");
}

VPE::VProperty *VPE::DirectionProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new DirectionProperty(getName()));
}

int VPE::DirectionProperty::indexOfDirection(const QMap<QString, QString> &directions, const QString &direction)
{
    QVector<QString> m_indexList;
    QMap<QString, QString>::const_iterator i = directions.constBegin();
    while (i != directions.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
    return m_indexList.indexOf(direction);
}

void VPE::DirectionProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent (VProperty::d_ptr->editor, event);
}

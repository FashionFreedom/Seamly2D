/******************************************************************************
 *   @file   vobjectproperty.h
 **  @author DS Caskey
 **  @date   Feb 21, 2023
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

#include "vobjectproperty.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QWidget>

#include "../vproperty_p.h"

VPE::VObjectProperty::VObjectProperty(const QString &name)
    : VProperty(name, QVariant::Int)
    , m_objects()
    , m_indexList()
{
    VProperty::d_ptr->VariantValue = 0;
    VProperty::d_ptr->VariantValue.convert(QVariant::UInt);
}

//! Get the data how it should be displayed
QVariant VPE::VObjectProperty::data(int column, int role) const
{
    if (m_objects.empty())
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
        return m_objects.value(m_indexList.at(tmpIndex));
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

//! Returns an editor widget, or NULL if it doesn't supply one
QWidget *VPE::VObjectProperty::createEditor(QWidget  *parent, const QStyleOptionViewItem &options,
                                       const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QComboBox *objEditor = new QComboBox(parent);
    objEditor->clear();
    objEditor->setMinimumWidth(140);
    objEditor->setLocale(parent->locale());
    fillListItems(objEditor, m_objects);
    objEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(objEditor, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &VObjectProperty::currentIndexChanged);

    VProperty::d_ptr->editor = objEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
QVariant VPE::VObjectProperty::getEditorData(const QWidget *editor) const
{
    const QComboBox *objEditor = qobject_cast<const QComboBox*>(editor);
    if (objEditor)
    {
        return objEditor->currentIndex();
    }

    return QVariant(0);
}

//! Sets the objects list
// cppcheck-suppress unusedFunction
void VPE::VObjectProperty::setObjectsList(const QMap<QString, quint32> &objects)
{
    this->m_objects = objects;

    m_indexList.clear();
    QMap<QString, quint32>::const_iterator i = m_objects.constBegin();
    while (i != objects.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
}

//! Get the settings. This function has to be implemented in a subclass in order to have an effect
// cppcheck-suppress unusedFunction
QMap<QString, quint32> VPE::VObjectProperty::getObjects() const
{
    return m_objects;
}

//! Sets the value of the property
void VPE::VObjectProperty::setValue(const QVariant &value)
{
    int tmpIndex = value.toInt();

    if (tmpIndex < 0 || tmpIndex >= m_indexList.count())
    {
        tmpIndex = 0;
    }

    VProperty::d_ptr->VariantValue = value;
    VProperty::d_ptr->VariantValue.convert(QVariant::UInt);

    if (VProperty::d_ptr->editor != nullptr)
    {
        setEditorData(VProperty::d_ptr->editor);
    }
}

QString VPE::VObjectProperty::type() const
{
    return "objectList";
}

VPE::VProperty *VPE::VObjectProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new VObjectProperty(getName()));
}

int VPE::VObjectProperty::indexOfObject(const QMap<QString, quint32> &objects, const QString &object)
{
    QVector<QString> m_indexList;
    QMap<QString, quint32>::const_iterator i = objects.constBegin();
    while (i != objects.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
    return m_indexList.indexOf(object);
}

void VPE::VObjectProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent (VProperty::d_ptr->editor, event );
}

void VPE::VObjectProperty::fillListItems(QComboBox *box, const QMap<QString, quint32> &list) const
{
    box->clear();

    QMap<QString, quint32>::const_iterator i;
    for (i = list.constBegin(); i != list.constEnd(); ++i)
    {
        box->addItem(i.key(), i.value());
    }
}

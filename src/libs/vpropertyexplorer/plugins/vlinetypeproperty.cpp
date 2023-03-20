/******************************************************************************
 *   @file   vlinetypeproperty.cpp
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

/************************************************************************
 **
 **  @file   vlinetypeproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://github.com/fashionfreedom/Valentina> All Rights Reserved.
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

#include "vlinetypeproperty.h"

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

VPE::LineTypeProperty::LineTypeProperty(const QString &name)
    : VProperty(name, QVariant::Int)
    , m_lineTypes()
    , m_indexList()
    , m_iconWidth(40)
    , m_iconHeight(14)
{
    VProperty::d_ptr->VariantValue = 0;
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
}

QVariant VPE::LineTypeProperty::data(int column, int role) const
{
    if (m_lineTypes.empty())
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

QWidget *VPE::LineTypeProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                         const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QComboBox *lineTypeEditor = new QComboBox(parent);

#if defined(Q_OS_MAC)
    // Mac pixmap should be little bit smaller
    lineTypeEditor->setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2));
#else
    // Windows
    lineTypeEditor->setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif
    lineTypeEditor->clear();
    lineTypeEditor->setMinimumWidth(140);
    lineTypeEditor->setLocale(parent->locale());

    QMap<QString, QString>::const_iterator i = m_lineTypes.constBegin();
    while (i != m_lineTypes.constEnd())
    {
        const QString lineType = QVariant(i.key()).toString();
        lineTypeEditor->addItem(createIcon(lineType), i.value(), QVariant(i.key()));
        ++i;
    }
    lineTypeEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(lineTypeEditor, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                     &LineTypeProperty::currentIndexChanged);

    VProperty::d_ptr->editor = lineTypeEditor;
    return VProperty::d_ptr->editor;
}

QVariant VPE::LineTypeProperty::getEditorData(const QWidget *editor) const
{
    const QComboBox *lineTypeEditor = qobject_cast<const QComboBox*>(editor);
    if (lineTypeEditor)
    {
        return lineTypeEditor->currentIndex();
    }

    return QVariant(0);
}

void VPE::LineTypeProperty::setLineTypes(const QMap<QString, QString> &lineTypes)
{
    this->m_lineTypes = lineTypes;
    m_indexList.clear();
    QMap<QString, QString>::const_iterator i = m_lineTypes.constBegin();
    while (i != lineTypes.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
}

// cppcheck-suppress unusedFunction
QMap<QString, QString> VPE::LineTypeProperty::getLineTypes() const
{
    return m_lineTypes;
}

void VPE::LineTypeProperty::setValue(const QVariant &value)
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

QString VPE::LineTypeProperty::type() const
{
    return QStringLiteral("lineType");
}

VPE::VProperty *VPE::LineTypeProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new LineTypeProperty(getName()));
}

int VPE::LineTypeProperty::indexOfLineType(const QMap<QString, QString> &lineTypes, const QString &lineType)
{
    QVector<QString> m_indexList;
    QMap<QString, QString>::const_iterator i = lineTypes.constBegin();
    while (i != lineTypes.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
    return m_indexList.indexOf(lineType);
}

void VPE::LineTypeProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent (VProperty::d_ptr->editor, event );
}

QIcon VPE::LineTypeProperty::createIcon(const QString &type)
{
    const Qt::PenStyle style = lineTypeToPenStyle(type);
    QPixmap pixmap(m_iconWidth, m_iconHeight);
    pixmap.fill(Qt::black);

    QBrush brush(Qt::black);
    QPen pen(brush, 2.5, style);

    QPainter painter(&pixmap);
    painter.fillRect(QRect(1, 1, m_iconWidth-2, m_iconHeight-2), QColor(Qt::white));
    if (style != Qt::NoPen)
    {
        painter.setPen(pen);
        painter.drawLine(0, m_iconHeight/2, m_iconWidth, m_iconHeight/2);
    }

    return QIcon(pixmap);
}

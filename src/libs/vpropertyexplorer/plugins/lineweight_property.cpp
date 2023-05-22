/******************************************************************************
 *   @file   lineweight_property.cpp
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

#include "lineweight_property.h"

#include <QBrush>
#include <QComboBox>
#include <QCoreApplication>
#include <QLocale>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QWidget>

#include "../ifc/ifcdef.h"
#include "../vproperty_p.h"

VPE::LineWeightProperty::LineWeightProperty(const QString &name)
    : VProperty(name, QVariant::Int)
    , m_lineWeights()
    , m_indexList()
    , m_iconWidth(40)
    , m_iconHeight(14)
{
    VProperty::d_ptr->VariantValue = 0;
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
}

QVariant VPE::LineWeightProperty::data(int column, int role) const
{
    if (m_lineWeights.empty())
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

QWidget *VPE::LineWeightProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                               const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QComboBox* lineWeightEditor = new QComboBox(parent);

#if defined(Q_OS_MAC)
    // Mac pixmap should be little bit smaller
    lineWeightEditor->setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2));
#else
    // Windows
    lineWeightEditor->setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif
    lineWeightEditor->clear();
    lineWeightEditor->setMinimumWidth(140);
    lineWeightEditor->setLocale(parent->locale());

    QMap<QString, QString>::const_iterator i = m_lineWeights.constBegin();
    while (i != m_lineWeights.constEnd())
    {
        const qreal lineweight = QVariant(i.key()).toDouble();
        lineWeightEditor->addItem(createIcon(lineweight), i.value(), QVariant(i.key()));
        ++i;
    }
    lineWeightEditor->setCurrentIndex(VProperty::d_ptr->VariantValue.toInt());
    connect(lineWeightEditor, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LineWeightProperty::currentIndexChanged);

    VProperty::d_ptr->editor = lineWeightEditor;
    return VProperty::d_ptr->editor;
}

QVariant VPE::LineWeightProperty::getEditorData(const QWidget *editor) const
{
    const QComboBox* lineWeightEditor = qobject_cast<const QComboBox*>(editor);
    if (lineWeightEditor)
    {
        return lineWeightEditor->currentIndex();
    }

    return QVariant(0);
}

void VPE::LineWeightProperty::setLineWeights(const QMap<QString, QString> &lineWeights)
{
    this->m_lineWeights = lineWeights;
    m_indexList.clear();
    QMap<QString, QString>::const_iterator i = m_lineWeights.constBegin();
    while (i != lineWeights.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
}

// cppcheck-suppress unusedFunction
QMap<QString, QString> VPE::LineWeightProperty::getLineWeights() const
{
    return m_lineWeights;
}

void VPE::LineWeightProperty::setValue(const QVariant &value)
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

QString VPE::LineWeightProperty::type() const
{
    return QStringLiteral("lineWeight");
}

VPE::VProperty *VPE::LineWeightProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new LineWeightProperty(getName()));
}

int VPE::LineWeightProperty::indexOfLineWeight(const QMap<QString, QString> &lineWeights, const QString &weight)
{
    QVector<QString> m_indexList;
    QMap<QString, QString>::const_iterator i = lineWeights.constBegin();
    while (i != lineWeights.constEnd())
    {
        m_indexList.append(i.key());
        ++i;
    }
    return m_indexList.indexOf(weight);
}

void VPE::LineWeightProperty::currentIndexChanged(int index)
{
    Q_UNUSED(index)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent (VProperty::d_ptr->editor, event);
}

QIcon VPE::LineWeightProperty::createIcon(const qreal &width)
{
    QPixmap pixmap(m_iconWidth, m_iconHeight);
    pixmap.fill(Qt::black);

    QPen pen(Qt::black, ToPixel(width, Unit::Mm), Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

    QPainter painter(&pixmap);
    painter.fillRect(QRect(1, 1, m_iconWidth-2, m_iconHeight-2), QColor(Qt::white));
    painter.setPen(pen);
    painter.drawLine(0, m_iconHeight/2, m_iconWidth, m_iconHeight/2);

    return QIcon(pixmap);
}

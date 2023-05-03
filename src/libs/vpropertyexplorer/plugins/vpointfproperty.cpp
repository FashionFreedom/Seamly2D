/************************************************************************
 **
 **  @file   vpointfproperty.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 8, 2014
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#include "vpointfproperty.h"

#include <QFlags>
#include <QList>
#include <QPointF>

#include "../vproperty_p.h"
#include "vnumberproperty.h"

VPE::VPointFProperty::VPointFProperty(const QString &name)
    : VProperty(name, QVariant::PointF)
{
    d_ptr->VariantValue.setValue(0);
    d_ptr->VariantValue.convert(QVariant::PointF);

    DoubleSpinboxProperty *tempX = new DoubleSpinboxProperty("X coordinate:");
    addChild(tempX);
    tempX->setUpdateBehaviour(true, false);

    DoubleSpinboxProperty *tempY = new DoubleSpinboxProperty("Y coordinate:");
    addChild(tempY);
    tempY->setUpdateBehaviour(true, false);

    setValue(QPointF());
}

QVariant VPE::VPointFProperty::data(int column, int role) const
{
    if (column == DPC_Data && Qt::DisplayRole == role)
    {
        return getPointF();
    }
    else
        return VProperty::data(column, role);
}

Qt::ItemFlags VPE::VPointFProperty::flags(int column) const
{
    if (column == DPC_Name || column == DPC_Data)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    else
        return Qt::NoItemFlags;
}

QPointF VPE::VPointFProperty::getPointF() const
{
    QPointF tempValue;

    if (d_ptr->Children.count() < 2)
    {
        return tempValue;
    }

    tempValue.setX(d_ptr->Children.at(0)->getValue().toDouble());
    tempValue.setY(d_ptr->Children.at(1)->getValue().toDouble());

    return tempValue;
}

void VPE::VPointFProperty::setPointF(const QPointF &point)
{
    setPointF(point.x(), point.y());
}

void VPE::VPointFProperty::setPointF(qreal x, qreal y)
{
    if (d_ptr->Children.count() < 2)
    {
        return;
    }

    QVariant tempX(x);
    tempX.convert(QVariant::Double);

    QVariant tempY(y);
    tempY.convert(QVariant::Double);

    d_ptr->Children.at(0)->setValue(tempX);
    d_ptr->Children.at(1)->setValue(tempY);
}

QString VPE::VPointFProperty::type() const
{
    return "pointF";
}

VPE::VProperty *VPE::VPointFProperty::clone(bool include_children, VProperty *container) const
{
    if (!container)
    {
        container = new VPointFProperty(getName());

        if (!include_children)
        {
            QList<VProperty*> tempChildren = container->getChildren();
            foreach(VProperty *tempChild, tempChildren)
            {
                container->removeChild(tempChild);
                delete tempChild;
            }
        }
    }

    return VProperty::clone(false, container);  // Child
}

void VPE::VPointFProperty::setValue(const QVariant &value)
{
    QPointF tempPoint = value.toPointF();
    setPointF(tempPoint);
}

QVariant VPE::VPointFProperty::getValue() const
{
    QPointF tempValue = getPointF();
    return QString("%1,%2").arg(QString::number(tempValue.x()), QString::number(tempValue.y()));
}

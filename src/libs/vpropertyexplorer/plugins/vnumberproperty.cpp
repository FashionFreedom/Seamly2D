/************************************************************************
 **
 **  @file   vnumberproperty.cpp
 **  @author hedgeware <internal(at)hedgeware.net>
 **  @date
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

#include "vnumberproperty.h"

#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QLatin1String>
#include <QLocale>
#include <QSizePolicy>
#include <QSpinBox>
#include <QWidget>

#include "../vproperty_p.h"

const int VPE::SpinboxProperty::StandardMin = -1000000;
const int VPE::SpinboxProperty::StandardMax = 1000000;

VPE::SpinboxProperty::SpinboxProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : VProperty(name, QVariant::Int)
    , minValue(StandardMin)
    , maxValue(StandardMax)
    , singleStep(1.0)
{
    VProperty::setSettings(settings);
    VProperty::d_ptr->VariantValue.setValue(0);
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
}

VPE::SpinboxProperty::SpinboxProperty(const QString &name)
    : VProperty(name)
    , minValue(StandardMin)
    , maxValue(StandardMax)
    , singleStep(1.0)
{
    VProperty::d_ptr->VariantValue.setValue(0);
    VProperty::d_ptr->VariantValue.convert(QVariant::Int);
}

//! Returns an editor widget, or NULL if it doesn't supply one
QWidget *VPE::SpinboxProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                             const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    QSpinBox *tmpEditor = new QSpinBox(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setMinimum(static_cast<int>(minValue));
    tmpEditor->setMaximum(static_cast<int>(maxValue));
    tmpEditor->setSingleStep(static_cast<int>(singleStep));
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tmpEditor->setValue(VProperty::d_ptr->VariantValue.toInt());
    connect(tmpEditor, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &SpinboxProperty::valueChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
QVariant VPE::SpinboxProperty::getEditorData(const QWidget *editor) const
{
    const QSpinBox *tmpEditor = qobject_cast<const QSpinBox*>(editor);
    if (tmpEditor)
    {
        return tmpEditor->value();
    }

    return QVariant(0);
}

void VPE::SpinboxProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("Min"))
    {
        maxValue = value.toInt();
    }
    else if (key == QLatin1String("Max"))
    {
        minValue = value.toInt();
    }
    else if (key == QLatin1String("Step"))
    {
        singleStep = value.toInt();
    }
}

QVariant VPE::SpinboxProperty::getSetting(const QString &key) const
{
    if (key == QLatin1String("Min"))
    {
        return minValue;
    }
    if (key == QLatin1String("Max"))
    {
        return maxValue;
    }
    if (key == QLatin1String("Step"))
    {
        return singleStep;
    }
    else
        return VProperty::getSetting(key);
}

QStringList VPE::SpinboxProperty::getSettingKeys() const
{
    return (QStringList("Min") << "Max" << "Step");
}

QString VPE::SpinboxProperty::type() const
{
    return "integer";
}

VPE::VProperty *VPE::SpinboxProperty::clone(bool include_children, VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new SpinboxProperty(getName()));
}

void VPE::SpinboxProperty::valueChanged(int i)
{
    Q_UNUSED(i)
    UserChangeEvent *event = new UserChangeEvent();
    QCoreApplication::postEvent (VProperty::d_ptr->editor, event );
}

const double VPE::DoubleSpinboxProperty::StandardPrecision = 5;

VPE::DoubleSpinboxProperty::DoubleSpinboxProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : SpinboxProperty(name)
    , Precision(static_cast<int>(StandardPrecision))
{
    VProperty::setSettings(settings);
    VProperty::d_ptr->VariantValue.setValue(0);
    VProperty::d_ptr->VariantValue.convert(QVariant::Double);
    VProperty::d_ptr->PropertyVariantType = QVariant::Double;
}

VPE::DoubleSpinboxProperty::DoubleSpinboxProperty(const QString &name)
    : SpinboxProperty(name)
    , Precision(static_cast<int>(StandardPrecision))
{
    VProperty::d_ptr->VariantValue.setValue(0);
    VProperty::d_ptr->VariantValue.convert(QVariant::Double);
    VProperty::d_ptr->PropertyVariantType = QVariant::Double;
}

//! Returns an editor widget, or NULL if it doesn't supply one
QWidget *VPE::DoubleSpinboxProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                            const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)
    QDoubleSpinBox *tmpEditor = new QDoubleSpinBox(parent);
    tmpEditor->setLocale(parent->locale());
    tmpEditor->setMinimum(minValue);
    tmpEditor->setMaximum(maxValue);
    tmpEditor->setDecimals(Precision);
    tmpEditor->setSingleStep(singleStep);
    tmpEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tmpEditor->setValue(VProperty::d_ptr->VariantValue.toDouble());
    connect(tmpEditor, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                     &SpinboxProperty::valueChanged);

    VProperty::d_ptr->editor = tmpEditor;
    return VProperty::d_ptr->editor;
}

//! Gets the data from the widget
QVariant VPE::DoubleSpinboxProperty::getEditorData(const QWidget *editor) const
{
    const QDoubleSpinBox *tmpEditor = qobject_cast<const QDoubleSpinBox*>(editor);
    if (tmpEditor)
    {
        return tmpEditor->value();
    }

    return QVariant(0);
}

void VPE::DoubleSpinboxProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("Min"))
    {
        minValue = value.toDouble();
    }
    else if (key == QLatin1String("Max"))
    {
        maxValue = value.toDouble();
    }
    else if (key == QLatin1String("Step"))
    {
        singleStep = value.toDouble();
    }
    else if (key == QLatin1String("Precision"))
    {
        Precision = value.toInt();
    }
}

QVariant VPE::DoubleSpinboxProperty::getSetting(const QString &key) const
{
    if (key == QLatin1String("Min"))
    {
        return minValue;
    }
    if (key == QLatin1String("Max"))
    {
        return maxValue;
    }
    if (key == QLatin1String("Step"))
    {
        return singleStep;
    }
    if (key == QLatin1String("Precision"))
    {
        return Precision;
    }
    else
        return VProperty::getSetting(key);
}

QStringList VPE::DoubleSpinboxProperty::getSettingKeys() const
{
    return (QStringList("Min") << "Max" << "Step" << "Precision");
}

QString VPE::DoubleSpinboxProperty::type() const
{
    return "double";
}

VPE::VProperty *VPE::DoubleSpinboxProperty::clone(bool include_children, VProperty *container) const
{
    return SpinboxProperty::clone(include_children, container ? container : new DoubleSpinboxProperty(getName()));
}

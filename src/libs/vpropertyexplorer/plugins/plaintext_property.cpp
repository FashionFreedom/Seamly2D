/************************************************************************
 **
 **  @file   plaintext_property.cpp
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

#include "plaintext_property.h"

#include <QKeyEvent>
#include <QLatin1String>
#include <QLocale>
#include <QSizePolicy>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QWidget>

#include "expandingtextedit.h"
#include "../vproperty_p.h"


VPE::PlainTextProperty::PlainTextProperty(const QString &name, const QMap<QString, QVariant> &settings)
    : VProperty(name, QVariant::String)
    , m_readOnly(false)
    , m_typeForParent(0)
    , m_osSeparator(false)
{
    VProperty::setSettings(settings);
    d_ptr->VariantValue.setValue(QString());
    d_ptr->VariantValue.convert(QVariant::String);
}

VPE::PlainTextProperty::PlainTextProperty(const QString &name)
    : VProperty(name)
    , m_readOnly(false)
    , m_typeForParent(0)
    , m_osSeparator(false)
{
    d_ptr->VariantValue.setValue(QString());
    d_ptr->VariantValue.convert(QVariant::String);
}

/**
 * @brief createEditor Returns an editor widget, or NULL if it doesn't supply one
 * @param parent The widget to which the editor will be added as a child
 * @param options Render options
 * @param delegate A pointer to the QAbstractItemDelegate requesting the editor.
 *                 This can be used to connect signals and slots.
 */
QWidget *VPE::PlainTextProperty::createEditor(QWidget *parent, const QStyleOptionViewItem &options,
                                            const QAbstractItemDelegate *delegate)
{
    Q_UNUSED(options)
    Q_UNUSED(delegate)

    ExpandingTextEdit *textEditor = new ExpandingTextEdit(parent);
    textEditor->setMinimumWidth(140);
    textEditor->setFixedHeight(28);
    textEditor->setLocale(parent->locale());
    textEditor->setReadOnly(m_readOnly);
    textEditor->installEventFilter(this);
    textEditor->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    textEditor->setPlainText(d_ptr->VariantValue.toString());

    d_ptr->editor = textEditor;
    return d_ptr->editor;
}

/**
 * @brief getEditorData Gets the data from the widget
 * @oaram editor
 */
QVariant VPE::PlainTextProperty::getEditorData(const QWidget *editor) const
{
    const QPlainTextEdit* textEditor = qobject_cast<const QPlainTextEdit*>(editor);
    if (textEditor)
    {
        return textEditor->toPlainText();
    }

    return QVariant(QString());
}

void VPE::PlainTextProperty::setReadOnly(bool m_readOnly)
{
    this->m_readOnly = m_readOnly;
}

void VPE::PlainTextProperty::setOsSeparator(bool separator)
{
    m_osSeparator = separator;
}

/**
 * @brief setSetting Sets the settings.
 */
void VPE::PlainTextProperty::setSetting(const QString &key, const QVariant &value)
{
    if (key == QLatin1String("ReadOnly"))
    {
        setReadOnly(value.toBool());
    }
    if (key == QLatin1String("TypeForParent"))
    {
        setTypeForParent(value.toInt());
    }
}

/**
 * @brief getSetting Gets the settings.his function has to be implemented in a subclass in order to have an effect.
 */
QVariant VPE::PlainTextProperty::getSetting(const QString &key) const
{
    if (key == QLatin1String("ReadOnly"))
    {
        return m_readOnly;
    }
    else if (key == QLatin1String("TypeForParent"))
    {
        return m_typeForParent;
    }
    else
        return VProperty::getSetting(key);
}

/**
 * @brief getSettingKeys Returns the list of keys of the property's settings.
 */
QStringList VPE::PlainTextProperty::getSettingKeys() const
{
    QStringList settings;
    settings << QStringLiteral("ReadOnly") << QStringLiteral("TypeForParent");
    return settings;
}

/**
 * @brief type Returns a string containing the type of the property.
 */
QString VPE::PlainTextProperty::type() const
{
    return QStringLiteral("string");
}

/**
 * @brief clone Clones this property
 * @param include_children Indicates whether to also clone the children
 * @param container If a property is being passed here, no new VProperty is being created but instead it is tried
 *                  to fill all the data into container. This can also be used when subclassing this function.
 * @return Returns the newly created property (or container, if it was not NULL)
 */
VPE::VProperty *VPE::PlainTextProperty::clone(bool include_children, VPE::VProperty *container) const
{
    return VProperty::clone(include_children, container ? container : new PlainTextProperty(getName(), getSettings()));
}

void VPE::PlainTextProperty::updateParent(const QVariant &value)
{
    emit childChanged(value, m_typeForParent);
}

// cppcheck-suppress unusedFunction
int VPE::PlainTextProperty::getTypeForParent() const
{
    return m_typeForParent;
}

void VPE::PlainTextProperty::setTypeForParent(int value)
{
    m_typeForParent = value;
}

bool VPE::PlainTextProperty::eventFilter(QObject *object, QEvent *event)
{
    if (QPlainTextEdit *textEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (m_osSeparator)
                {
                    textEdit->insertPlainText(QLocale().decimalPoint());
                }
                else
                {
                    textEdit->insertPlainText(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }
    else
    {
        // pass the event on to the parent class
        return VProperty::eventFilter(object, event);
    }
    return false;// pass the event to the widget
}

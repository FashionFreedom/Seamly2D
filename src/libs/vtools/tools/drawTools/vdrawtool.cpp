//---------------------------------------------------------------------------------------------------------------------
//  @file   vdrawtool.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2015 - 2026 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vdrawtool.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "vdrawtool.h"

#include "../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../../undocommands/addtocalc.h"
#include "../../undocommands/savetooloptions.h"

#include <QDialog>
#include <QDomNode>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QUndoStack>
#include <Qt>
#include <QtDebug>

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/// @brief VDrawTool constructor.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
//---------------------------------------------------------------------------------------------------------------------
VDrawTool::VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent)
    : VInteractiveTool(doc, data, id, parent)
    , m_active_block_name(doc->getActiveDraftBlockName())
    , m_lineType(LineTypeSolidLine)
    , m_lineWeight(DefaultLineWeight)
{
    connect(this->doc, &VAbstractPattern::activeDraftBlockChanged, this, &VDrawTool::activeBlockChanged);
    connect(this->doc, &VAbstractPattern::draftBlockNameChanged, this, &VDrawTool::blockNameChanged);
    connect(this->doc, &VAbstractPattern::ShowTool, this, &VDrawTool::ShowTool);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief isUsed check if the tool can't be deleted.
///
/// Besides the id based reference count, a tool can also be needed through the variables it registered in the
/// data container, for example the length of an arc or the angles of a spline. Formulas reference those variables
/// by name, so the reference counter never sees them. VInternalVariable::Filter() tells which variables belong to
/// this tool, the same test the formula dialog uses to hide a tool's own variables.
///
/// @return true if another tool references this tool by id, or a formula uses one of its variables.
//---------------------------------------------------------------------------------------------------------------------

bool VDrawTool::isUsed() const
{
    if (VInteractiveTool::isUsed())
    {
        return true;
    }

    QStringList variable_names;
    const QHash<QString, QSharedPointer<VInternalVariable>> *variables = data.DataVariables();
    for (auto i = variables->constBegin(); i != variables->constEnd(); ++i)
    {
        if (i.value()->Filter(m_id))
        {
            variable_names.append(i.key());
        }
    }

    return doc->isVariableUsed(variable_names);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ShowTool  highlight tool.
/// @param id object id in container.
/// @param enable enable or disable highlight.
//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ShowTool(quint32 id, bool enable)
{
    Q_UNUSED(id)
    Q_UNUSED(enable)
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief activeBlockChanged disable or enable context menu after change active draft block.
/// @param new_name new name active draft block.
//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::activeBlockChanged(const QString &new_name)
{
    Disable(!(m_active_block_name == new_name), new_name);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief blockNameChanged save new name active draft block.
/// @param old_name old name.
/// @param new_name new name active draft block. new name.
//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::blockNameChanged(const QString &old_name, const QString &new_name)
{
    if (m_active_block_name == old_name)
    {
        m_active_block_name = new_name;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveDialogChange()
{
    qCDebug(vTool, "Saving tool options after using dialog");
    QDomElement old_dom_element = doc->elementById(m_id, getTagName());
    if (old_dom_element.isElement())
    {
        QDomElement new_dom_element = old_dom_element.cloneNode().toElement();
        SaveDialog(new_dom_element);

        SaveToolOptions *save_options = new SaveToolOptions(old_dom_element, new_dom_element, doc, m_id);
        connect(save_options, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(save_options);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief AddToFile add tag with Information about tool into file.
//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::AddToFile()
{
    QDomElement dom_element = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(dom_element, obj);
    AddToCalculation(dom_element);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOption(QSharedPointer<VGObject> &obj)
{
    qCDebug(vTool, "Saving tool options");
    QDomElement old_dom_element = doc->elementById(m_id, getTagName());
    if (old_dom_element.isElement())
    {
        QDomElement new_dom_element = old_dom_element.cloneNode().toElement();

        SaveOptions(new_dom_element, obj);

        SaveToolOptions *save_options = new SaveToolOptions(old_dom_element, new_dom_element, doc, m_id);
        connect(save_options, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(save_options);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    Q_UNUSED(obj)

    doc->SetAttribute(tag, VDomDocument::AttrId, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::makeToolTip() const
{
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
bool VDrawTool::CorrectDisable(bool disable, const QString &draft_block_name) const
{
    if (disable)
    {
        return disable;
    }
    else
    {
        return !(m_active_block_name == draft_block_name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadAttributes()
{
    const QDomElement dom_element = doc->elementById(m_id, getTagName());
    if (dom_element.isElement())
    {
        ReadToolAttributes(dom_element);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::updatePointNameVisibility(quint32 id, bool visible)
{
    Q_UNUSED(id)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::EnableToolMove(bool move)
{
    Q_UNUSED(move)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::piecesMode(bool mode)
{
    Q_UNUSED(mode)
    // Do nothing.
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief AddToCalculation add tool to calculation tag in pattern file.
/// @param dom_element tag in xml tree.
//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::AddToCalculation(const QDomElement &dom_element)
{
    AddToCalc *add_to_calculation = new AddToCalc(dom_element, doc);
    connect(add_to_calculation, &AddToCalc::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(add_to_calculation);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::addDependence(QList<quint32> &list, quint32 object_id) const
{
    if (object_id != NULL_ID)
    {
        auto origin_point = VAbstractTool::data.GetGObject(object_id);
        list.append(origin_point->getIdTool());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::getLineType() const
{
    return m_lineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VDrawTool::getLineWeight() const
{
    return m_lineWeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::setLineWeight(const QString &value)
{
    m_lineWeight = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
bool VDrawTool::isPointNameVisible(quint32 id) const
{
    Q_UNUSED(id)
    return false;
}

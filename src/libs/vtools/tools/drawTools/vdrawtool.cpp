/***************************************************************************
 **  @file   vdrawtool.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   vdrawtool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
 
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
/**
 * @brief VDrawTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 */
VDrawTool::VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent)
    : VInteractiveTool(doc, data, id, parent)
    , activeBlockName(doc->getActiveDraftBlockName())
    , m_lineType(LineTypeSolidLine)
    , m_lineWeight("0.35")
{
    connect(this->doc, &VAbstractPattern::activeDraftBlockChanged, this, &VDrawTool::activeBlockChanged);
    connect(this->doc, &VAbstractPattern::draftBlockNameChanged, this, &VDrawTool::blockNameChanged);
    connect(this->doc, &VAbstractPattern::ShowTool, this, &VDrawTool::ShowTool);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.
 * @param id object id in container.
 * @param enable enable or disable highlight.
 */
void VDrawTool::ShowTool(quint32 id, bool enable)
{
    Q_UNUSED(id)
    Q_UNUSED(enable)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief activeBlockChanged disable or enable context menu after change active draft block.
 * @param newName new name active draft block.
 */
void VDrawTool::activeBlockChanged(const QString &newName)
{
    Disable(!(activeBlockName == newName), newName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief blockNameChanged save new name active draft block.
 * @param oldName old name.
 * @param newName new name active draft block. new name.
 */
void VDrawTool::blockNameChanged(const QString &oldName, const QString &newName)
{
    if (activeBlockName == oldName)
    {
        activeBlockName = newName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveDialogChange()
{
    qCDebug(vTool, "Saving tool options after using dialog");
    QDomElement oldDomElement = doc->elementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();
        SaveDialog(newDomElement);

        SaveToolOptions *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCWarning(vTool, "Can't find tool with id = %u", m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with Information about tool into file.
 */
void VDrawTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::SaveOption(QSharedPointer<VGObject> &obj)
{
    qCDebug(vTool, "Saving tool options");
    QDomElement oldDomElement = doc->elementById(m_id, getTagName());
    if (oldDomElement.isElement())
    {
        QDomElement newDomElement = oldDomElement.cloneNode().toElement();

        SaveOptions(newDomElement, obj);

        SaveToolOptions *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(saveOptions);
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
bool VDrawTool::CorrectDisable(bool disable, const QString &draftBlockName) const
{
    if (disable)
    {
        return disable;
    }
    else
    {
        return !(activeBlockName == draftBlockName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::ReadAttributes()
{
    const QDomElement domElement = doc->elementById(m_id, getTagName());
    if (domElement.isElement())
    {
        ReadToolAttributes(domElement);
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
/**
 * @brief AddToCalculation add tool to calculation tag in pattern file.
 * @param domElement tag in xml tree.
 */
void VDrawTool::AddToCalculation(const QDomElement &domElement)
{
    AddToCalc *addToCal = new AddToCalc(domElement, doc);
    connect(addToCal, &AddToCalc::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(addToCal);
}

//---------------------------------------------------------------------------------------------------------------------
void VDrawTool::addDependence(QList<quint32> &list, quint32 objectId) const
{
    if (objectId != NULL_ID)
    {
        auto originPoint = VAbstractTool::data.GetGObject(objectId);
        list.append(originPoint->getIdTool());
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

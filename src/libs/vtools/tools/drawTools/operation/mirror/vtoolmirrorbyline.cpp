/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "vtoolmirrorbyline.h"

#include <limits.h>
#include <qiterator.h>
#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QUndoStack>
#include <new>

#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../dialogs/tools/dialogmirrorbyline.h"
#include "../../../../visualization/line/operation/vistoolmirrorbyline.h"
#include "../../../../visualization/visualization.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../ifc/ifcdef.h"
#include "../ifc/exception/vexception.h"
#include "../vwidgets/vabstractsimple.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../vabstracttool.h"
#include "../../../vdatatool.h"
#include "../../vdrawtool.h"

template <class T> class QSharedPointer;

const QString VToolMirrorByLine::ToolType = QStringLiteral("flippingByLine");

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogMirrorByLine> dialogTool = m_dialog.objectCast<DialogMirrorByLine>();
    SCASSERT(not dialogTool.isNull())
    dialogTool->setFirstLinePointId(m_firstLinePointId);
    dialogTool->setSecondLinePointId(m_secondLinePointId);
    dialogTool->setSuffix(suffix);
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByLine *VToolMirrorByLine::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                             VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogMirrorByLine> dialogTool = dialog.objectCast<DialogMirrorByLine>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstLinePointId  = dialogTool->getFirstLinePointId();
    const quint32 secondLinePointId = dialogTool->getSecondLinePointId();
    const QString suffix            = dialogTool->getSuffix();
    const QVector<quint32> source   = dialogTool->getObjects();
    VToolMirrorByLine* operation = Create(0, firstLinePointId, secondLinePointId, suffix, source,
                                            QVector<DestinationItem>(), scene, doc, data, Document::FullParse,
                                            Source::FromGui);
    if (operation != nullptr)
    {
        operation->m_dialog = dialogTool;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByLine *VToolMirrorByLine::Create(const quint32 _id, quint32 firstLinePointId, quint32 secondLinePointId,
                                                 const QString &suffix, const QVector<quint32> &source,
                                                 const QVector<DestinationItem> &destination, VMainGraphicsScene *scene,
                                                 VAbstractPattern *doc, VContainer *data, const Document &parse,
                                                 const Source &typeCreation)
{
    const auto firstPoint = *data->GeometricObject<VPointF>(firstLinePointId);
    const QPointF fPoint = static_cast<QPointF>(firstPoint);

    const auto secondPoint = *data->GeometricObject<VPointF>(secondLinePointId);
    const QPointF sPoint = static_cast<QPointF>(secondPoint);

    QVector<DestinationItem> dest = destination;

    quint32 id = _id;
    createDestination(typeCreation, id, dest, source, fPoint, sPoint, suffix, doc, data, parse);

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::MirrorByLine, doc);
        VToolMirrorByLine *tool = new VToolMirrorByLine(doc, data, id, firstLinePointId, secondLinePointId, suffix,
                                                            source, dest, typeCreation);
        scene->addItem(tool);
        initOperationToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);
        doc->IncrementReferens(firstPoint.getIdTool());
        doc->IncrementReferens(secondPoint.getIdTool());
        for (int i = 0; i < source.size(); ++i)
        {
            doc->IncrementReferens(data->GetGObject(source.at(i))->getIdTool());
        }
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMirrorByLine::firstLinePointName() const
{
    return VAbstractTool::data.GetGObject(m_firstLinePointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMirrorByLine::secondLinePointName() const
{
    return VAbstractTool::data.GetGObject(m_secondLinePointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolMirrorByLine>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolMirrorByLine *visual = qobject_cast<VisToolMirrorByLine *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObjects(source);
        visual->setFirstLinePointId(m_firstLinePointId);
        visual->setSecondLinePointId(m_secondLinePointId);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogMirrorByLine> dialogTool = m_dialog.objectCast<DialogMirrorByLine>();
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrP1Line, QString().setNum(dialogTool->getFirstLinePointId()));
    doc->SetAttribute(domElement, AttrP2Line, QString().setNum(dialogTool->getSecondLinePointId()));
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->getSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::ReadToolAttributes(const QDomElement &domElement)
{
    m_firstLinePointId = doc->GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
    m_secondLinePointId = doc->GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
    suffix = doc->GetParametrString(domElement, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrP1Line, QString().setNum(m_firstLinePointId));
    doc->SetAttribute(tag, AttrP2Line, QString().setNum(m_secondLinePointId));
    doc->SetAttribute(tag, AttrSuffix, suffix);

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    try
    {
        ContextMenu<DialogMirrorByLine>(this, event);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByLine::VToolMirrorByLine(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 firstLinePointId,
                                         quint32 secondLinePointId, const QString &suffix,
                                         const QVector<quint32> &source, const QVector<DestinationItem> &destination,
                                         const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractMirror(doc, data, id, suffix, source, destination, parent)
    , m_firstLinePointId(firstLinePointId)
    , m_secondLinePointId(secondLinePointId)
{
    InitOperatedObjects();
    ToolCreation(typeCreation);
}

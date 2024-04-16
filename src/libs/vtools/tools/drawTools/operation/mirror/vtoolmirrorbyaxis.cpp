/***************************************************************************
 **  @file   vtoolmirrorbyaxis.cpp
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
 **  @file   vtoolmirrorbyaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2016 Valentina project
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

#include "vtoolmirrorbyaxis.h"

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
#include "../../../../dialogs/tools/dialogmirrorbyaxis.h"
#include "../../../../visualization/line/operation/vistoolmirrorbyaxis.h"
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

const QString VToolMirrorByAxis::ToolType = QStringLiteral("flippingByAxis");

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogMirrorByAxis> dialogTool = m_dialog.objectCast<DialogMirrorByAxis>();
    SCASSERT(not dialogTool.isNull())
    dialogTool->setOriginPointId(m_originPointId);
    dialogTool->setAxisType(m_axisType);
    dialogTool->setSuffix(suffix);
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByAxis *VToolMirrorByAxis::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                                 VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogMirrorByAxis> dialogTool = dialog.objectCast<DialogMirrorByAxis>();
    SCASSERT(not dialogTool.isNull())
    const quint32 originPointId      = dialogTool->getOriginPointId();
    const AxisType axisType          = dialogTool->getAxisType();
    const QString suffix             = dialogTool->getSuffix();
    const QVector<SourceItem> source = dialogTool->getSourceObjects();
    VToolMirrorByAxis* operation  = Create(0, originPointId, axisType, suffix, source, QVector<DestinationItem>(),
                                           scene, doc, data, Document::FullParse, Source::FromGui);
    if (operation != nullptr)
    {
        operation->m_dialog = dialogTool;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByAxis *VToolMirrorByAxis::Create(const quint32 _id, quint32 originPointId, AxisType axisType,
                                                 const QString &suffix, const QVector<SourceItem> &source,
                                                 const QVector<DestinationItem> &destination, VMainGraphicsScene *scene,
                                                 VAbstractPattern *doc, VContainer *data, const Document &parse,
                                                 const Source &typeCreation)
{
    const auto originPoint = *data->GeometricObject<VPointF>(originPointId);
    const QPointF fPoint = static_cast<QPointF>(originPoint);

    QPointF sPoint;
    if (axisType == AxisType::VerticalAxis)
    {
        sPoint = QPointF(fPoint.x(), fPoint.y() + 100);
    }
    else
    {
        sPoint = QPointF(fPoint.x() + 100, fPoint.y());
    }

    QVector<DestinationItem> dest = destination;

    quint32 id = _id;
    createDestination(typeCreation, id, dest, source, fPoint, sPoint, suffix, doc, data, parse);

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::MirrorByAxis, doc);
        VToolMirrorByAxis *tool = new VToolMirrorByAxis(doc, data, id, originPointId, axisType, suffix, source,
                                                            dest, typeCreation);
        scene->addItem(tool);
        initOperationToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);
        doc->IncrementReferens(originPoint.getIdTool());
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            doc->IncrementReferens(data->GetGObject(item.id)->getIdTool());
        }
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
AxisType VToolMirrorByAxis::getAxisType() const
{
    return m_axisType;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::setAxisType(AxisType value)
{
    m_axisType = value;

    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMirrorByAxis::getOriginPointName() const
{
    return VAbstractTool::data.GetGObject(m_originPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolMirrorByAxis::getOriginPointId() const
{
    return m_originPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::setOriginPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        m_originPointId = value;

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolMirrorByAxis>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolMirrorByAxis *visual = qobject_cast<VisToolMirrorByAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObjects(sourceToObjects(source));
        visual->setOriginPointId(m_originPointId);
        visual->setAxisType(m_axisType);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogMirrorByAxis> dialogTool = m_dialog.objectCast<DialogMirrorByAxis>();
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->getOriginPointId()));
    doc->SetAttribute(domElement, AttrAxisType, QString().setNum(static_cast<int>(dialogTool->getAxisType())));
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->getSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::ReadToolAttributes(const QDomElement &domElement)
{
    m_originPointId = doc->GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    m_axisType = static_cast<AxisType>(doc->GetParametrUInt(domElement, AttrAxisType, "1"));
    suffix = doc->GetParametrString(domElement, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, QString().setNum(m_originPointId));
    doc->SetAttribute(tag, AttrAxisType, QString().setNum(static_cast<int>(m_axisType)));
    doc->SetAttribute(tag, AttrSuffix, suffix);

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMirrorByAxis::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogMirrorByAxis>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMirrorByAxis::makeToolTip() const
{
    const QString toolTipStr = QString("<tr> <td><b>%1:</b> %2</td> </tr>")
                                       .arg(tr("Origin point"))
                                       .arg(getOriginPointName());
    return toolTipStr;
}

//---------------------------------------------------------------------------------------------------------------------
VToolMirrorByAxis::VToolMirrorByAxis(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 originPointId,
                                         AxisType axisType, const QString &suffix,
                                         const QVector<SourceItem> &source, const QVector<DestinationItem> &destination,
                                         const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractMirror(doc, data, id, suffix, source, destination, parent)
    , m_originPointId(originPointId)
    , m_axisType(axisType)
{
    InitOperatedObjects();
    ToolCreation(typeCreation);
}

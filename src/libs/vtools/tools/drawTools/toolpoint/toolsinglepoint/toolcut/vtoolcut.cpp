//  @file   vtoolcut.cpp
//  @author Douglas S Caskey
//  @date   8 Jun, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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

//-----------------------------------------------------------------------------
//  @file   vtoolcut.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   25 6, 2014
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "vtoolcut.h"

#include <QDomElement>
#include <QSharedPointer>
#include <new>

#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../vtoolsinglepoint.h"

//---------------------------------------------------------------------------------------------------------------------
VToolCut::VToolCut(VAbstractPattern *doc, VContainer *data, const quint32 &id, QString &direction,
                   const QString &formula, const QString &lineColor, const quint32 &curveCutId, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(lineColor), parent)
    , m_direction(direction)
    , formula(formula)
    , lineColor(lineColor)
    , curveCutId(curveCutId)
    , m_piecesMode(false)
{
    setPointColor(lineColor);

    Q_ASSERT_X(curveCutId != 0, Q_FUNC_INFO, "curveCutId == 0"); //-V654 //-V712
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::Disable(bool disable, const QString &draftBlockName)
{
    VToolSinglePoint::Disable(disable, draftBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::piecesMode(bool mode)
{
    m_piecesMode = mode;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolCut::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
quint32 VToolCut::getCurveCutId() const
{
    return curveCutId;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VToolCut::setCurveCutId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        curveCutId = value;
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

QString VToolCut::getDirection() const
{
    return m_direction;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::setDirection(const QString &value)
{
        m_direction = value;
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCut::GetFormula() const
{
    VFormula val(formula, getData());
    val.setCheckZero(true);
    val.setToolId(m_id);
    val.setPostfix(UnitsToStr(qApp->patternUnit()));
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetFormula(const VFormula &value)
{
    if (value.error() == false)
    {
        formula = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCut::getLineColor() const
{
    return lineColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::setLineColor(const QString &value)
{
    lineColor = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCut::CurveName() const
{
    return VAbstractTool::data.GetGObject(curveCutId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RefreshGeometry  refresh item on scene.
 */
void VToolCut::RefreshGeometry()
{
    setPointColor(lineColor);
    VToolSinglePoint::refreshPointGeometry(*VDrawTool::data.GeometricObject<VPointF>(m_id));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement referens value for used objects.
 */
void VToolCut::RemoveReferens()
{
    const auto curve = VAbstractTool::data.GetGObject(curveCutId);

    doc->DecrementReferens(curve->getIdTool());
}

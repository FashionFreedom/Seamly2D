/***************************************************************************
 **  @file   vpattern.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
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
 **
 **  @file   vpattern.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 2, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vpattern.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vtools/tools/vdatatool.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/tools/union_tool.h"
#include "../vtools/tools/drawTools/drawtools.h"
#include "../vtools/tools/nodeDetails/nodedetails.h"
#include "../tools/images/image_tool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionundo.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vmisc/customevents.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/vmath.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vabstractapplication.h"
#include "../qmuparser/qmuparsererror.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../core/application_2d.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/vnodedetail.h"

#include <QMessageBox>
#include <QUndoStack>
#include <QtNumeric>
#include <QDebug>
#include <QFileInfo>

const QString VPattern::AttrReadOnly = QStringLiteral("readOnly");

namespace
{
//---------------------------------------------------------------------------------------------------------------------
QString FileComment()
{
    return QString("Pattern created with Seamly2D v%1 (https://seamly.io).").arg(APP_VERSION_STR);
}
}

//---------------------------------------------------------------------------------------------------------------------
VPattern::VPattern(VContainer *data, Draw *mode, VMainGraphicsScene *draftScene,
                   VMainGraphicsScene *pieceScene, QObject *parent)
    : VAbstractPattern(parent),
      data(data),
      mode(mode),
      draftScene(draftScene),
      pieceScene(pieceScene)
{
    SCASSERT(draftScene != nullptr)
    SCASSERT(pieceScene != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyFile create minimal empty file.
 */
void VPattern::CreateEmptyFile()
{
    this->clear();
    QDomElement patternElement = this->createElement(TagPattern);

    patternElement.appendChild(createComment(FileComment()));

    QDomElement version = createElement(TagVersion);
    QDomText newNodeText = createTextNode(VPatternConverter::PatternMaxVerStr);
    version.appendChild(newNodeText);
    patternElement.appendChild(version);

    QDomElement unit = createElement(TagUnit);
    newNodeText = createTextNode(UnitsToStr(qApp->patternUnit()));
    unit.appendChild(newNodeText);
    patternElement.appendChild(unit);

    patternElement.appendChild(createElement(TagDescription));
    patternElement.appendChild(createElement(TagNotes));

    patternElement.appendChild(createElement(TagMeasurements));
    patternElement.appendChild(createElement(TagVariables));

    this->appendChild(patternElement);
    insertBefore(createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::setXMLContent(const QString &fileName)
{
    VDomDocument::setXMLContent(fileName);
    GarbageCollector();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Parse parse file.
 * @param parse parser file mode.
 */
void VPattern::Parse(const Document &parse)
{
    qCDebug(vXML, "Parsing pattern.");
    switch (parse)
    {
        case Document::FullParse:
            qCDebug(vXML, "Full parse.");
            break;
        case Document::LiteParse:
            qCDebug(vXML, "Lite parse.");
            break;
        case Document::LiteBlockParse:
            qCDebug(vXML, "Lite draft block parse.");
            break;
        default:
            break;
    }

    SCASSERT(draftScene != nullptr)
    SCASSERT(pieceScene != nullptr)
    QStringList tags = QStringList() << TagDraftBlock << TagVariables << TagDescription << TagNotes
                                     << TagMeasurements << TagVersion << TagGradation << TagImage << TagUnit
                                     << TagPatternName << TagPatternNum << TagCompanyName << TagCustomerName
                                     << TagPatternLabel;
    PrepareForParse(parse);
    QDomNode domNode = documentElement().firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                switch (tags.indexOf(domElement.tagName()))
                {
                    case 0: // TagDraftBlock
                        qCDebug(vXML, "Tag draw.");
                        if (parse == Document::FullParse)
                        {
                            if (activeDraftBlock.isEmpty())
                            {
                                setActiveDraftBlock(GetParametrString(domElement, AttrName));
                            }
                            else
                            {
                                changeActiveDraftBlock(GetParametrString(domElement, AttrName));
                            }
                            patternPieces << GetParametrString(domElement, AttrName);
                        }
                        else
                        {
                            changeActiveDraftBlock(GetParametrString(domElement, AttrName), Document::LiteParse);
                        }
                        parseDraftBlockElement(domElement, parse);
                        break;
                    case 1: // TagVariables
                        qCDebug(vXML, "Tag variables.");
                        parseVariablesElement(domElement);
                        break;
                    case 2: // TagDescription
                        qCDebug(vXML, "Tag description.");
                        break;
                    case 3: // TagNotes
                        qCDebug(vXML, "Tag notes.");
                        break;
                    case 4: // TagMeasurements
                        qCDebug(vXML, "Tag measurements.");
                        break;
                    case 5: // TagVersion
                        qCDebug(vXML, "Tag version.");
                        break;
                    case 6: // TagGradation
                        qCDebug(vXML, "Tag gradation.");
                        break;
                    case 7: // TagImage
                        qCDebug(vXML, "Tag image.");
                        break;
                    case 8: // TagUnit
                        qCDebug(vXML, "Tag unit.");
                        break;
                    case 9: // TagPatternName
                        qCDebug(vXML, "Pattern name.");
                        break;
                    case 10: // TagPatternNumber
                        qCDebug(vXML, "Pattern number.");
                        break;
                    case 11: // TagCompanyName
                        qCDebug(vXML, "Company name.");
                        break;
                    case 12: // TagCustomerName
                        qCDebug(vXML, "Customer name.");
                        break;
                    case 13: // TagPatternLabel
                        qCDebug(vXML, "Pattern label.");
                        break;
                    default:
                        qCDebug(vXML, "Wrong tag name %s", qUtf8Printable(domElement.tagName()));
                        break;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
    emit patternParsed();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentData set current data set.
 *
 * Each time after parsing need set correct data set for current draft block. After parsing it is always last.
 * Current data set for draft block it is data set for last object in draft block (point, arc, spline, spline path so
 * on).
 */
void VPattern::setCurrentData()
{
    if (*mode == Draw::Calculation)
    {
        if (draftBlockCount() > 1)//don't need to update data if we have only one draft block
        {
            qCDebug(vXML, "Setting current data");
            qCDebug(vXML, "Current Draft block name %s", qUtf8Printable(activeDraftBlock));
            qCDebug(vXML, "Draftf block count %d", draftBlockCount());

            quint32 id = 0;
            if (history.size() == 0)
            {
                qCDebug(vXML, "History is empty!");
                return;
            }
            for (qint32 i = 0; i < history.size(); ++i)
            {
                const VToolRecord tool = history.at(i);
                if (tool.getDraftBlockName() == activeDraftBlock)
                {
                    id = tool.getId();
                }
            }
            qCDebug(vXML, "Resoring data from tool with id %u", id);
            if (id == NULL_ID)
            {
                qCDebug(vXML, "Could not find record for this current draft block %s",
                        qUtf8Printable(activeDraftBlock));

                const VToolRecord tool = history.at(history.size()-1);
                id = tool.getId();
                qCDebug(vXML, "Taking record with id %u from Draft block %s", id, qUtf8Printable(tool.getDraftBlockName()));
                if (id == NULL_ID)
                {
                    qCDebug(vXML, "Bad id for last record in history.");
                    return;
                }
            }
            if (tools.size() > 0)
            {
                try
                {
                    ToolExists(id);
                }
                catch (VExceptionBadId &error)
                {
                    Q_UNUSED(error)
                    qCDebug(vXML, "List of tools doesn't contain id= %u", id);
                    return;
                }

                const VDataTool *vTool = tools.value(id);
                *data = vTool->getData();
                //Delete special variables if exist
                data->RemoveVariable(currentLength);
                data->RemoveVariable(currentSeamAllowance);
                qCDebug(vXML, "Data successfully updated.");
            }
            else
            {
                qCDebug(vXML, "List of tools is empty!");
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateToolData update tool in list tools.
 * @param id tool id.
 * @param data container with variables.
 */
void VPattern::UpdateToolData(const quint32 &id, VContainer *data)
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0"); //-V712 //-V654
    SCASSERT(data != nullptr)
    ToolExists(id);
    VDataTool *tool = tools.value(id);
    SCASSERT(tool != nullptr)
    tool->VDataTool::setData(data);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getActiveBasePoint return id base point current draft block.
 * @return id base point.
 */
// cppcheck-suppress unusedFunction
quint32 VPattern::getActiveBasePoint()
{
    QDomElement calcElement;
    if (getActiveNodeElement(TagCalculation, calcElement))
    {
        const QDomNode domNode = calcElement.firstChild();
        if (domNode.isNull() == false && domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                if (domElement.tagName() == TagPoint && domElement.attribute(AttrType, "") == VToolBasePoint::ToolType)
                {
                    return getParameterId(domElement);
                }
            }
        }
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPattern::getActivePatternPieces() const
{
    QVector<quint32> pieces;
    QDomElement drawElement;
    if (getActiveDraftElement(drawElement))
    {
        const QDomElement elements = drawElement.firstChildElement(TagPieces);
        if (!elements.isNull())
        {
            QDomElement piece = elements.firstChildElement(TagPiece);
            while(!piece.isNull())
            {
                bool united = getParameterBool(piece, PatternPieceTool::AttrUnited, falseStr);
                if (!united)
                {
                    pieces.append(getParameterId(piece));
                }
                piece = piece.nextSiblingElement(TagPiece);
            }
        }
    }
    return pieces;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPattern::SaveDocument(const QString &fileName, QString &error)
{
    try
    {
        TestUniqueId();
    }
    catch (const VExceptionWrongId &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error not unique id.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        return false;
    }

    // Update comment with Seamly2D version
    QDomNode commentNode = documentElement().firstChild();
    if (commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    const bool saved = VAbstractPattern::SaveDocument(fileName, error);
    if (saved && QFileInfo(fileName).suffix() != QLatin1String("autosave"))
    {
        modified = false;
    }

    return saved;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::LiteParseVariables()
{
    try
    {
        emit setGuiEnabled(true);

        VContainer::clearUniqueVariableNames();
        data->ClearVariables(VarType::Variable);

        const QDomNodeList tags = elementsByTagName(TagVariables);
        if (not tags.isEmpty())
        {
            const QDomNode domElement = tags.at(0);
            if (not domElement.isNull())
            {
                parseVariablesElement(domElement);
            }
        }
    }
    catch (const VExceptionUndo &error)
    {
        Q_UNUSED(error)
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }
    catch (const VExceptionObjectError &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        return;
    }
    catch (const VExceptionConversionError &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        return;
    }
    catch (const VExceptionEmptyParameter &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        return;
    }
    catch (const VExceptionWrongId &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        return;
    }
    catch (VException &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vXML, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        emit setGuiEnabled(false);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LiteParseTree lite parse file.
 */
void VPattern::LiteParseTree(const Document &parse)
{
    // Save current draft block name
    QString draftBlockName = activeDraftBlock;

    try
    {
        emit setGuiEnabled(true);
        switch (parse)
        {
            case Document::LiteBlockParse:
                parseCurrentDraftBlock();
                break;
            case Document::LiteParse:
                Parse(parse);
                break;
            case Document::FullParse:
                qCWarning(vXML, "Lite parsing doesn't support full parsing");
                break;
            default:
                break;
        }
    }
    catch (const VExceptionUndo &error)
    {
        Q_UNUSED(error)
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }
    catch (const VExceptionObjectError &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionConversionError &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionEmptyParameter &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionWrongId &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (VException &error)
    {
        qCCritical(vXML, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vXML, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        emit setGuiEnabled(false);
        if (not Application2D::isGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    // Restore name current draft block
    activeDraftBlock = draftBlockName;
    qCDebug(vXML, "Current draft block %s", qUtf8Printable(activeDraftBlock));
    setCurrentData();
    emit FullUpdateFromFile();
    // Recalculate scene rect
    VMainGraphicsView::NewSceneRect(draftScene, qApp->getSceneView());
    VMainGraphicsView::NewSceneRect(pieceScene, qApp->getSceneView());
    qCDebug(vXML, "Scene size updated.");
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VPattern::customEvent(QEvent *event)
{
    if (event->type() == UNDO_EVENT)
    {
        qApp->getUndoStack()->undo();
    }
    else if (event->type() == LITE_PARSE_EVENT)
    {
        LiteParseTree(Document::LiteParse);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VNodeDetail VPattern::parsePieceNode(const QDomElement &domElement) const
{
    const quint32 id = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    const qreal mx = GetParametrDouble(domElement, AttrMx, "0.0");
    const qreal my = GetParametrDouble(domElement, AttrMy, "0.0");
    const bool reverse = GetParametrUInt(domElement, VAbstractPattern::AttrNodeReverse, "0");
    const NodeDetail nodeType = NodeDetail::Contour;

    const QString t = GetParametrString(domElement, AttrType, "NodePoint");
    Tool tool;

    QStringList types = QStringList() << VAbstractPattern::NodePoint
                                      << VAbstractPattern::NodeArc
                                      << VAbstractPattern::NodeSpline
                                      << VAbstractPattern::NodeSplinePath
                                      << VAbstractPattern::NodeElArc;
    switch (types.indexOf(t))
    {
        case 0: // NodePoint
            tool = Tool::NodePoint;
            break;
        case 1: // NodeArc
            tool = Tool::NodeArc;
            break;
        case 2: // NodeSpline
            tool = Tool::NodeSpline;
            break;
        case 3: // NodeSplinePath
            tool = Tool::NodeSplinePath;
            break;
        case 4: // NodeElArc
            tool = Tool::NodeElArc;
            break;
        default:
            VException e(tr("Wrong tag name '%1'.").arg(t));
            throw e;
    }
    return VNodeDetail(id, tool, nodeType, mx, my, reverse);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief parseDraftBlockElement parse draw tag.
 * @param node node.
 * @param parse parser file mode.
 */
void VPattern::parseDraftBlockElement(const QDomNode &node, const Document &parse)
{
    QStringList tags = QStringList() << TagCalculation << TagModeling << TagPieces << TagGroups << TagDraftImages;
    QDomNode domNode = node.firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                switch (tags.indexOf(domElement.tagName()))
                {
                    case 0: // TagCalculation
                        qCDebug(vXML, "Tag calculation.");
                        data->ClearCalculationGObjects();
                        ParseDrawMode(domElement, parse, Draw::Calculation);
                        break;
                    case 1: // TagModeling
                        qCDebug(vXML, "Tag modeling.");
                        ParseDrawMode(domElement, parse, Draw::Modeling);
                        break;
                    case 2: // TagPieces
                        qCDebug(vXML, "Tag pieces.");
                        parsePatternPieces(domElement, parse);
                        break;
                    case 3: // TagGroups
                        qCDebug(vXML, "Tag groups.");
                        parseGroups(domElement);
                        break;
                    case 4: // TagDraftImages
                        qCDebug(vXML, "Tag draft images.");
                        parseDraftImages(domElement, parse);
                        break;
                    default:
                        VException e(tr("Wrong tag name '%1'.").arg(domElement.tagName()));
                        throw e;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDrawMode parse draw tag with draw mode.
 * @param node node.
 * @param parse parser file mode.
 * @param mode draw mode.
 */
void VPattern::ParseDrawMode(const QDomNode &node, const Document &parse, const Draw &mode)
{
    SCASSERT(draftScene != nullptr)
    SCASSERT(pieceScene != nullptr)
    VMainGraphicsScene *scene = nullptr;
    if (mode == Draw::Calculation)
    {
        scene = draftScene;
    }
    else
    {
        scene = pieceScene;
    }
    const QStringList tags = QStringList() << TagPoint
                                           << TagLine
                                           << TagSpline
                                           << TagArc
                                           << TagTools
                                           << TagOperation
                                           << TagElArc
                                           << TagPath;
    const QDomNodeList nodeList = node.childNodes();
    const qint32 num = nodeList.size();
    for (qint32 i = 0; i < num; ++i)
    {
        QDomElement domElement = nodeList.at(i).toElement();
        if (domElement.isNull() == false)
        {
            switch (tags.indexOf(domElement.tagName()))
            {
                case 0: // TagPoint
                    qCDebug(vXML, "Tag point.");
                    ParsePointElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 1: // TagLine
                    qCDebug(vXML, "Tag line.");
                    ParseLineElement(scene, domElement, parse);
                    break;
                case 2: // TagSpline
                    qCDebug(vXML, "Tag spline.");
                    ParseSplineElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 3: // TagArc
                    qCDebug(vXML, "Tag arc.");
                    ParseArcElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 4: // TagTools
                    qCDebug(vXML, "Tag tools.");
                    ParseToolsElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 5: // TagOperation
                    qCDebug(vXML, "Tag operation.");
                    ParseOperationElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 6: // TagElArc
                    qCDebug(vXML, "Tag elliptical arc.");
                    ParseEllipticalArcElement(scene, domElement, parse, domElement.attribute(AttrType, ""));
                    break;
                case 7: // TagPath
                    qCDebug(vXML, "Tag path.");
                    ParsePathElement(scene, domElement, parse);
                    break;
                default:
                    VException e(tr("Wrong tag name '%1'.").arg(domElement.tagName()));
                    throw e;
            }
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseDraftImages parses draft images.
 * @param node node.
 */
void VPattern::parseDraftImages(const QDomNode &node, const Document &parse)
{
    SCASSERT(draftScene != nullptr)
    SCASSERT(pieceScene != nullptr)

    const QDomNodeList nodeList = node.childNodes();
    const qint32 num = nodeList.size();
    for (qint32 i = 0; i < num; ++i)
    {
        QDomElement domElement = nodeList.at(i).toElement();
        if (domElement.isNull() == false)
        {
            if (domElement.tagName()==TagDraftImage)
            {
                qCDebug(vXML, "Tag image.");
                parseImageElement(domElement, parse);
            }else{
                VException e(tr("Wrong tag name '%1'.").arg(domElement.tagName()));
                throw e;
            }
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief parsePieceElement parse piece tag.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::parsePieceElement(QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        VPiece piece;
        const quint32 id = getParameterId(domElement);
        piece.SetName(GetParametrString(domElement, AttrName, tr("Piece")));
        piece.setColor(GetParametrString(domElement, PatternPieceTool::AttrPieceColor, tr("white")));
        piece.setFill(GetParametrString(domElement, PatternPieceTool::AttrPieceFill, tr("nobrush")));
        piece.setIsLocked(getParameterBool(domElement, AttrPieceLocked, falseStr));

        piece.SetMx(qApp->toPixel(GetParametrDouble(domElement, AttrMx, "0.0")));
        piece.SetMy(qApp->toPixel(GetParametrDouble(domElement, AttrMy, "0.0")));
        piece.SetSeamAllowance(getParameterBool(domElement, PatternPieceTool::AttrSeamAllowance, falseStr));
        piece.setHideSeamLine(getParameterBool(domElement, PatternPieceTool::AttrHideSeamLine,
                                               QString().setNum(qApp->Seamly2DSettings()->isHideSeamLine())));
        piece.SetSeamAllowanceBuiltIn(getParameterBool(domElement, PatternPieceTool::AttrSeamAllowanceBuiltIn,
                                                       falseStr));
        piece.SetForbidFlipping(getParameterBool(domElement, PatternPieceTool::AttrForbidFlipping,
                                           QString().setNum(qApp->Seamly2DSettings()->getForbidPieceFlipping())));
        piece.SetInLayout(getParameterBool(domElement, AttrInLayout, trueStr));
        piece.SetUnited(getParameterBool(domElement, PatternPieceTool::AttrUnited, falseStr));

        const QString width = GetParametrString(domElement, AttrWidth, "0.0");
        QString w = width;//need for saving fixed formula;
        const uint version = GetParametrUInt(domElement, PatternPieceTool::AttrVersion, "1");

        const QStringList tags = QStringList() << TagNodes
                                               << TagData
                                               << TagPatternInfo
                                               << TagGrainline
                                               << PatternPieceTool::TagCSA
                                               << PatternPieceTool::TagIPaths
                                               << PatternPieceTool::TagAnchors;

        const QDomNodeList nodeList = domElement.childNodes();
        for (qint32 i = 0; i < nodeList.size(); ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (not element.isNull())
            {
                switch (tags.indexOf(element.tagName()))
                {
                    case 0:// TagNodes
                        if (version == 1)
                        {
                            // TODO. Delete if minimal supported version is 0.4.0
                            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                                              "Time to refactor the code.");
                            const bool closed = GetParametrUInt(domElement, AttrClosed, "1");
                            const qreal width = GetParametrDouble(domElement, AttrWidth, "0.0");
                            parsePieceNodes(element, piece, width, closed);
                        }
                        else
                        {
                            piece.SetPath(ParsePieceNodes(element));
                        }
                        break;
                    case 1:// TagData
                        ParsePieceDataTag(element, piece);
                        break;
                    case 2:// TagPatternInfo
                        ParsePiecePatternInfo(element, piece);
                        break;
                    case 3:// TagGrainline
                        ParsePieceGrainline(element, piece);
                        break;
                    case 4:// PatternPieceTool::TagCSA
                        piece.SetCustomSARecords(ParsePieceCSARecords(element));
                        break;
                    case 5:// PatternPieceTool::TagIPaths
                        piece.SetInternalPaths(ParsePieceInternalPaths(element));
                        break;
                    case 6:// PatternPieceTool::TagAnchors
                        piece.setAnchors(ParsePieceAnchors(element));
                        break;
                    default:
                        break;
                }
            }
        }
        PatternPieceTool::Create(id, piece, w, pieceScene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (w != width)
        {
            SetAttribute(domElement, AttrWidth, w);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating piece"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief parseImageElement parse image tag.
 * @param domElement tag in xml tree.
 */
void VPattern::parseImageElement(QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    DraftImage image;

    image.id = getParameterId(domElement);
    image.name = GetParametrString(domElement, AttrName, tr("Image_name"));
    image.filename = GetParametrString(domElement, AttrSource);
    image.width = GetParametrDouble(domElement, AttrWidth, "0.0");
    image.height = GetParametrDouble(domElement, AttrHeight, "0.0");
    image.xPos = GetParametrDouble(domElement, AttrXPos, "0.0");
    image.yPos = GetParametrDouble(domElement, AttrYPos, "0.0");
    image.rotation = GetParametrDouble(domElement, AttrRotation, "0.0");
    image.aspectLocked = getParameterBool(domElement, AttrAspectRatio, falseStr);
    image.locked = getParameterBool(domElement, AttrLocked, falseStr);
    image.units = StrToUnits(GetParametrString(domElement, AttrUnits, "px"));
    image.opacity = GetParametrDouble(domElement, AttrOpacity, "1.0");
    image.order = qint32(GetParametrDouble(domElement, AttrOrder, "0"));
    image.xOrigin = GetParametrDouble(domElement, AttrXOffset, "0.0");
    image.yOrigin = GetParametrDouble(domElement, AttrYOffset, "0.0");
    image.basepoint = GetParametrUInt(domElement, AttrBasepoint, 0);
    image.visible = getParameterBool(domElement, AttrVisible, trueStr);

    VContainer::UpdateId(image.id);

    if(parse == Document::FullParse)
    {
        ImageTool *image_tool = new ImageTool(this, this, draftScene, image);
        if(image_tool->creationWasSuccessful)
        {
            connect(image_tool, &ImageTool::setStatusMessage, this, [this](QString message) {emit setStatusMessage(message);});
        }
        else
        {
            image_tool->deleteLater();
            ParentNodeById(image.id).removeChild(domElement); //this way the broken image is not light-parsed in the future
        }
    }
    else
    {
        getBackgroundImage(image.id)->updateImageAndHandles(image);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPattern::parsePieceNodes(const QDomElement &domElement, VPiece &piece, qreal width, bool closed) const
{
    QVector<VNodeDetail> oldNodes;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull()
                && element.tagName() == VAbstractPattern::TagNode) // Old piece version need this check!
        {
            oldNodes.append(parsePieceNode(element));
        }
    }

    piece.GetPath().SetNodes(VNodeDetail::Convert(data, oldNodes, width, closed));
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePieceDataTag(const QDomElement &domElement, VPiece &piece) const
{
    VPieceLabelData &ppData = piece.GetPatternPieceData();
    ppData.SetVisible(getParameterBool(domElement, AttrVisible, trueStr));
    ppData.SetLetter(GetParametrEmptyString(domElement, AttrLetter));
    ppData.SetAnnotation(GetParametrEmptyString(domElement, AttrAnnotation));
    ppData.SetOrientation(GetParametrEmptyString(domElement, AttrOrientation));
    ppData.SetRotationWay(GetParametrEmptyString(domElement, AttrRotationWay));
    ppData.SetTilt(GetParametrEmptyString(domElement, AttrTilt));
    ppData.SetFoldPosition(GetParametrEmptyString(domElement, AttrFoldPosition));
    ppData.SetQuantity(static_cast<int>(GetParametrUInt(domElement, AttrQuantity, "1")));
    ppData.SetOnFold(getParameterBool(domElement, AttrOnFold, falseStr));
    ppData.SetPos(QPointF(GetParametrDouble(domElement, AttrMx, "0"), GetParametrDouble(domElement, AttrMy, "0")));
    ppData.SetLabelWidth(GetParametrString(domElement, AttrWidth, "1"));
    ppData.SetLabelHeight(GetParametrString(domElement, PatternPieceTool::AttrHeight, "1"));
    ppData.SetFontSize(static_cast<int>(GetParametrUInt(domElement, PatternPieceTool::AttrFont, "0")));
    ppData.SetRotation(GetParametrString(domElement, AttrRotation, "0"));
    ppData.setCenterAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrCenterAnchor, NULL_ID_STR));
    ppData.setTopLeftAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrTopLeftAnchor, NULL_ID_STR));
    ppData.setBottomRightAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrBottomRightAnchor, NULL_ID_STR));
    ppData.SetLabelTemplate(GetLabelTemplate(domElement));
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePiecePatternInfo(const QDomElement &domElement, VPiece &piece) const
{
    VPatternLabelData &patternInfo = piece.GetPatternInfo();
    patternInfo.SetVisible(getParameterBool(domElement, AttrVisible, trueStr));
    patternInfo.SetPos(QPointF(GetParametrDouble(domElement, AttrMx, "0"), GetParametrDouble(domElement, AttrMy, "0")));
    patternInfo.SetLabelWidth(GetParametrString(domElement, AttrWidth, "1"));
    patternInfo.SetLabelHeight(GetParametrString(domElement, PatternPieceTool::AttrHeight, "1"));
    patternInfo.SetFontSize(static_cast<int>(GetParametrUInt(domElement, PatternPieceTool::AttrFont, "0")));
    patternInfo.SetRotation(GetParametrString(domElement, AttrRotation, "0"));
    patternInfo.setCenterAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrCenterAnchor, NULL_ID_STR));
    patternInfo.setTopLeftAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrTopLeftAnchor, NULL_ID_STR));
    patternInfo.setBottomRightAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrBottomRightAnchor, NULL_ID_STR));
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePieceGrainline(const QDomElement &domElement, VPiece &piece) const
{
    VGrainlineData &gGeometry = piece.GetGrainlineGeometry();
    gGeometry.SetVisible(getParameterBool(domElement, AttrVisible, falseStr));
    gGeometry.SetPos(QPointF(GetParametrDouble(domElement, AttrMx, "0"), GetParametrDouble(domElement, AttrMy, "0")));
    gGeometry.SetLength(GetParametrString(domElement, AttrLength, "1"));
    gGeometry.SetRotation(GetParametrString(domElement, AttrRotation, "90"));
    gGeometry.SetArrowType(static_cast<ArrowType>(GetParametrUInt(domElement, AttrArrows, "0")));
    gGeometry.setCenterAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrCenterAnchor, NULL_ID_STR));
    gGeometry.setTopAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrTopAnchorPoint, NULL_ID_STR));
    gGeometry.setBottomAnchorPoint(GetParametrUInt(domElement, PatternPieceTool::AttrBottomAnchorPoint, NULL_ID_STR));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief parsePatternPieces parse pieces tag.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::parsePatternPieces(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    QDomNode domNode = domElement.firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                if (domElement.tagName() == TagPiece)
                {
                    parsePieceElement(domElement, parse);
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, quint32 &id, QString &name, qreal &mx, qreal &my,
                                      bool &isVisible, QString &lineType, QString &lineWeight, QString &lineColor)
{
    PointsCommonAttributes(domElement, id, name, mx, my, isVisible);
    lineType   = GetParametrString(domElement, AttrLineType,   LineTypeSolidLine);
    lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");
    lineColor  = GetParametrString(domElement, AttrLineColor,  qApp->Settings()->getPointNameColor());
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, quint32 &id, QString &name, qreal &mx, qreal &my,
                                      bool &isVisible, QString &lineColor)
{
    PointsCommonAttributes(domElement, id, name, mx, my, isVisible);
    lineColor  = GetParametrString(domElement, AttrLineColor,  qApp->Settings()->getPointNameColor());
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, quint32 &id, QString &name,
                                      qreal &mx, qreal &my, bool &isVisible)
{
    PointsCommonAttributes(domElement, id, mx, my);
    name      = GetParametrString(domElement, AttrName,          "A");
    isVisible = getParameterBool(domElement,  AttrShowPointName, trueStr);

}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PointsCommonAttributes(const QDomElement &domElement, quint32 &id, qreal &mx, qreal &my)
{
    ToolsCommonAttributes(domElement, id);
    mx = qApp->toPixel(GetParametrDouble(domElement, AttrMx, "10.0"));
    my = qApp->toPixel(GetParametrDouble(domElement, AttrMy, "15.0"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParsePointElement parse point tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of point.
 */
void VPattern::ParsePointElement(VMainGraphicsScene *scene, QDomElement &domElement,
                                 const Document &parse, const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of point is empty");

    QStringList points = QStringList() << VToolBasePoint::ToolType                  /*0*/
                                       << VToolEndLine::ToolType                    /*1*/
                                       << VToolAlongLine::ToolType                  /*2*/
                                       << VToolShoulderPoint::ToolType              /*3*/
                                       << VToolNormal::ToolType                     /*4*/
                                       << VToolBisector::ToolType                   /*5*/
                                       << VToolLineIntersect::ToolType              /*6*/
                                       << VToolPointOfContact::ToolType             /*7*/
                                       << VNodePoint::ToolType                      /*8*/
                                       << VToolHeight::ToolType                     /*9*/
                                       << VToolTriangle::ToolType                   /*10*/
                                       << PointIntersectXYTool::ToolType            /*11*/
                                       << VToolCutSpline::ToolType                  /*12*/
                                       << VToolCutSplinePath::ToolType              /*13*/
                                       << VToolCutArc::ToolType                     /*14*/
                                       << VToolLineIntersectAxis::ToolType          /*15*/
                                       << VToolCurveIntersectAxis::ToolType         /*16*/
                                       << VToolPointOfIntersectionArcs::ToolType    /*17*/
                                       << IntersectCirclesTool::ToolType            /*18*/
                                       << IntersectCircleTangentTool::ToolType      /*19*/
                                       << VToolPointFromArcAndTangent::ToolType     /*20*/
                                       << VToolTrueDarts::ToolType                  /*21*/
                                       << VToolPointOfIntersectionCurves::ToolType  /*22*/
                                       << AnchorPointTool::ToolType;                /*23*/
    switch (points.indexOf(type))
    {
        case 0: //VToolBasePoint::ToolType
            ParseToolBasePoint(scene, domElement, parse);
            break;
        case 1: //VToolEndLine::ToolType
            ParseToolEndLine(scene, domElement, parse);
            break;
        case 2: //VToolAlongLine::ToolType
            ParseToolAlongLine(scene, domElement, parse);
            break;
        case 3: //VToolShoulderPoint::ToolType
            ParseToolShoulderPoint(scene, domElement, parse);
            break;
        case 4: //VToolNormal::ToolType
            ParseToolNormal(scene, domElement, parse);
            break;
        case 5: //VToolBisector::ToolType
            ParseToolBisector(scene, domElement, parse);
            break;
        case 6: //VToolLineIntersect::ToolType
            ParseToolLineIntersect(scene, domElement, parse);
            break;
        case 7: //VToolPointOfContact::ToolType
            ParseToolPointOfContact(scene, domElement, parse);
            break;
        case 8: //VNodePoint::ToolType
            ParseNodePoint(domElement, parse);
            break;
        case 9: //VToolHeight::ToolType
            ParseToolHeight(scene, domElement, parse);
            break;
        case 10: //VToolTriangle::ToolType
            ParseToolTriangle(scene, domElement, parse);
            break;
        case 11: //PointIntersectXYTool::ToolType
            parseIntersectXYTool(scene, domElement, parse);
            break;
        case 12: //VToolCutSpline::ToolType
            ParseToolCutSpline(scene, domElement, parse);
            break;
        case 13: //VToolCutSplinePath::ToolType
            ParseToolCutSplinePath(scene, domElement, parse);
            break;
        case 14: //VToolCutArc::ToolType
            ParseToolCutArc(scene, domElement, parse);
            break;
        case 15: //VToolLineIntersectAxis::ToolType
            ParseToolLineIntersectAxis(scene, domElement, parse);
            break;
        case 16: //VToolCurveIntersectAxis::ToolType
            ParseToolCurveIntersectAxis(scene, domElement, parse);
            break;
        case 17: //VToolPointOfIntersectionArcs::ToolType
            ParseToolPointOfIntersectionArcs(scene, domElement, parse);
            break;
        case 18: //IntersectCirclesTool::ToolType
            ParseToolPointOfIntersectionCircles(scene, domElement, parse);
            break;
        case 19: //IntersectCircleTangentTool::ToolType
            ParseToolPointFromCircleAndTangent(scene, domElement, parse);
            break;
        case 20: //VToolPointFromArcAndTangent::ToolType
            ParseToolPointFromArcAndTangent(scene, domElement, parse);
            break;
        case 21: //VToolTrueDarts::ToolType
            ParseToolTrueDarts(scene, domElement, parse);
            break;
        case 22: //VToolPointOfIntersectionCurves::ToolType
            ParseToolPointOfIntersectionCurves(scene, domElement, parse);
            break;
        case 23: //AnchorPointTool::ToolType
            ParseAnchorPoint(domElement, parse);
            break;
        default:
            VException e(tr("Unknown point type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseLineElement parse line tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 */
void VPattern::ParseLineElement(VMainGraphicsScene *scene, const QDomElement &domElement,
                                const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        quint32 id = 0;
        ToolsCommonAttributes(domElement, id);
        const quint32 firstPoint  = GetParametrUInt(domElement,   AttrFirstPoint,  NULL_ID_STR);
        const quint32 secondPoint = GetParametrUInt(domElement,   AttrSecondPoint, NULL_ID_STR);
        const QString lineType    = GetParametrString(domElement, AttrLineType,    LineTypeSolidLine);
        const QString lineWeight  = GetParametrString(domElement, AttrLineWeight,  "0.35");
        const QString lineColor   = GetParametrString(domElement, AttrLineColor,   ColorBlack);

        VToolLine::Create(id, firstPoint, secondPoint, lineType, lineWeight, lineColor, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating line"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SplinesCommonAttributes(const QDomElement &domElement, quint32 &id, quint32 &idObject, quint32 &idTool)
{
    ToolsCommonAttributes(domElement, id);
    idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::parseCurrentDraftBlock()
{
    QDomElement domElement;
    if (getActiveDraftElement(domElement))
    {
        parseDraftBlockElement(domElement, Document::LiteParse);
    }
    emit patternParsed();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCurrentAlphabet returns the alphabet corresponding to the selected label language.
 */
QStringList VPattern::GetCurrentAlphabet() const
{
    const QStringList list = Application2D::pointNameLanguages();
    const QString def = QStringLiteral("A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z");
    QStringList alphabet;
    switch (list.indexOf(qApp->Seamly2DSettings()->getPointNameLanguage()))
    {
    case 0: // de
    {
        const QString al = QStringLiteral("A,Ä,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Ö,P,Q,R,S,ß,T,U,Ü,V,W,X,Y,Z");
        alphabet = al.split(",");
        break;
    }
    case 2: // fr
    {
        const QString al = QStringLiteral("A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z");
        alphabet = al.split(",");
        break;
    }
    case 3: // ru
    {
        const QString al = QStringLiteral("А,Б,В,Г,Д,Е,Ж,З,И,К,Л,М,Н,О,П,Р,С,Т,У,Ф,Х,Ц,Ч,Ш,Щ,Э,Ю,Я");
        alphabet = al.split(",");
        break;
    }
    case 4: // uk
    {
        const QString al = QStringLiteral("А,Б,В,Г,Д,Е,Ж,З,І,Ї,Й,К,Л,М,Н,О,П,Р,С,Т,У,Ф,Х,Ц,Ч,Ш,Щ,Є,Ю,Я");
        alphabet = al.split(",");
        break;
    }
    case 5: // hr
    case 7: // bs
    {
        const QString al = QStringLiteral("A,B,C,Č,Ć,D,Dž,Ð,E,F,G,H,I,J,K,L,Lj,M,N,Nj,O,P,R,S,Š,T,U,V,Z,Ž");
        alphabet = al.split(",");
        break;
    }
    case 6: // sr
    {
        const QString al = QStringLiteral("А,Б,В,Г,Д,Ђ,Е,Ж,З,И,Ј,К,Л,Љ,М,Н,Њ,О,П,Р,С,Т,Ћ,У,Ф,Х,Ц,Ч,Џ,Ш");
        alphabet = al.split(",");
        break;
    }
    case 1: // en
    default: // en
    {
        alphabet = def.split(",");
        break;
    }
    }

    return alphabet;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPattern::GetLabelBase(quint32 index) const
{
    QStringList alphabet = GetCurrentAlphabet();
    QString base;
    const int count = qFloor(index/static_cast<quint32>(alphabet.size()));
    const int number = static_cast<int>(index) - alphabet.size() * count;
    int i = 0;
    do
    {
        base.append(alphabet.at(number));
        ++i;
    } while (i < count);
    return base;
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolBasePoint(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    VToolBasePoint *spoint = nullptr;
    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const qreal x = qApp->toPixel(GetParametrDouble(domElement, AttrX, "10.0"));
        const qreal y = qApp->toPixel(GetParametrDouble(domElement, AttrY, "10.0"));

        VPointF *point = new VPointF(x, y, name, mx, my);
        point->setShowPointName(showPointName);
        spoint = VToolBasePoint::Create(id, activeDraftBlock, point, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating single point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        scene->removeItem(spoint);
        delete spoint;
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolEndLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);

        const QString formula = GetParametrString(domElement, AttrLength, "100.0");
        QString f = formula;//need for saving fixed formula;

        const quint32 basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);

        const QString angle = GetParametrString(domElement, AttrAngle, "0.0");
        QString angleFix = angle;

        VToolEndLine::Create(id, name, lineType, lineWeight, lineColor, f, angleFix, basePointId, mx, my, showPointName, scene, this, data,
                             parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula || angleFix != angle)
        {
            SetAttribute(domElement, AttrLength, f);
            SetAttribute(domElement, AttrAngle, angleFix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of end line"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of end line"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolAlongLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const QString formula = GetParametrString(domElement, AttrLength, "100.0");
        QString f = formula;//need for saving fixed formula;
        const quint32 firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolAlongLine::Create(id, name, lineType, lineWeight, lineColor, f, firstPointId, secondPointId, mx, my, showPointName, scene,
                               this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point along line"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point along line"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolShoulderPoint(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const QString formula = GetParametrString(domElement, AttrLength, "100.0");
        QString f = formula;//need for saving fixed formula;
        const quint32 p1Line = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        const quint32 p2Line = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
        const quint32 pShoulder = GetParametrUInt(domElement, AttrPShoulder, NULL_ID_STR);

        VToolShoulderPoint::Create(id, f, p1Line, p2Line, pShoulder, lineType, lineWeight, lineColor, name, mx, my,
                                   showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of shoulder"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of shoulder"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolNormal(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const QString formula = GetParametrString(domElement, AttrLength, "100.0");
        QString f = formula;//need for saving fixed formula;
        const quint32 firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
        const qreal angle = GetParametrDouble(domElement, AttrAngle, "0.0");

        VToolNormal::Create(id, f, firstPointId, secondPointId, lineType, lineWeight, lineColor, name, angle,
                            mx, my, showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of normal"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of normal"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolBisector(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const QString formula = GetParametrString(domElement, AttrLength, "100.0");
        QString f = formula;//need for saving fixed formula;
        const quint32 firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
        const quint32 thirdPointId = GetParametrUInt(domElement, AttrThirdPoint, NULL_ID_STR);

        VToolBisector::Create(id, f, firstPointId, secondPointId, thirdPointId,
                            lineType, lineWeight, lineColor, name, mx, my, showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of bisector"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of bisector"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolLineIntersect(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 p1Line1Id = GetParametrUInt(domElement, AttrP1Line1, NULL_ID_STR);
        const quint32 p2Line1Id = GetParametrUInt(domElement, AttrP2Line1, NULL_ID_STR);
        const quint32 p1Line2Id = GetParametrUInt(domElement, AttrP1Line2, NULL_ID_STR);
        const quint32 p2Line2Id = GetParametrUInt(domElement, AttrP2Line2, NULL_ID_STR);

        VToolLineIntersect::Create(id, p1Line1Id, p2Line1Id, p1Line2Id, p2Line2Id, name,
                                   mx, my, showPointName, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of line intersection"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfContact(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const QString radius = GetParametrString(domElement, AttrRadius, "0");
        QString f = radius;//need for saving fixed formula;
        const quint32 center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        const quint32 firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolPointOfContact::Create(id, f, center, firstPointId, secondPointId, name, mx, my, showPointName, scene, this,
                                    data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != radius)
        {
            SetAttribute(domElement, AttrRadius, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of contact"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of contact"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodePoint(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        qreal mx = 0;
        qreal my = 0;

        PointsCommonAttributes(domElement, id, mx, my);
        const quint32 idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        const quint32 idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        QSharedPointer<VPointF> point;
        try
        {
            point = data->GeometricObject<VPointF>(idObject);
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(error)
            return;// Just ignore
        }

        VPointF *nodePoint = new VPointF(static_cast<QPointF>(*point), point->name(), mx, my, idObject, Draw::Modeling);
        nodePoint->setShowPointName(getParameterBool(domElement, AttrShowPointName, trueStr));

        data->UpdateGObject(id, nodePoint);
        VNodePoint::Create(this, data, pieceScene, id, idObject, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseAnchorPoint(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        const quint32 idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        AnchorPointTool::Create(id, idObject, NULL_ID, this, data, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating anchor point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolHeight(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const quint32 basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        const quint32 p1LineId = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        const quint32 p2LineId = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);

        VToolHeight::Create(id, name, lineType, lineWeight, lineColor, basePointId, p1LineId, p2LineId,
                            mx, my, showPointName, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating height"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolTriangle(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 axisP1Id = GetParametrUInt(domElement, AttrAxisP1, NULL_ID_STR);
        const quint32 axisP2Id = GetParametrUInt(domElement, AttrAxisP2, NULL_ID_STR);
        const quint32 firstPointId = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        VToolTriangle::Create(id, name, axisP1Id, axisP2Id, firstPointId, secondPointId, mx, my, showPointName, scene, this,
                              data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating triangle"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::parseIntersectXYTool(VMainGraphicsScene *scene, const QDomElement &domElement,
                                            const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);
        const quint32 firstPointId  = GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);

        PointIntersectXYTool::Create(id, name, lineType, lineWeight, lineColor, firstPointId, secondPointId,
                                     mx, my, showPointName, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating Intersect XY tool"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutSpline(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineColor);
        QString direction       = GetParametrString(domElement, AttrDirection, "forward");
        const QString formula   = GetParametrString(domElement, AttrLength, "0");
        QString f               = formula;//need for saving fixed formula;
        const quint32 splineId  = GetParametrUInt(domElement, VToolCutSpline::AttrSpline, NULL_ID_STR);

        VToolCutSpline::Create(id, name, direction, f, lineColor, splineId, mx, my, showPointName, scene,
                               this, data, parse, Source::FromFile);

        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline point"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutSplinePath(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineColor);
        QString direction          = GetParametrString(domElement, AttrDirection, "forward");
        const QString formula      = GetParametrString(domElement, AttrLength, "0");
        QString f = formula; //need for saving fixed formula;
        const quint32 splinePathId = GetParametrUInt(domElement, VToolCutSplinePath::AttrSplinePath,
                                                     NULL_ID_STR);

        VToolCutSplinePath::Create(id, name, direction, f, lineColor, splinePathId, mx, my, showPointName, scene,
                                   this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline path point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut spline path point"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCutArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineColor);
        QString direction     = GetParametrString(domElement, AttrDirection, "forward");
        const QString formula = GetParametrString(domElement, AttrLength, "0");
        QString f = formula;//need for saving fixed formula;
        const quint32 arcId   = GetParametrUInt(domElement, AttrArc, NULL_ID_STR);

        VToolCutArc::Create(id, name, direction, f, lineColor, arcId, mx, my, showPointName, scene,
                            this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (f != formula)
        {
            SetAttribute(domElement, AttrLength, f);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut arc point"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cut arc point"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolLineIntersectAxis(VMainGraphicsScene *scene, QDomElement &domElement,
                                          const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);

        const quint32 basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        const quint32 firstPointId = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        const quint32 secondPointId = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);

        const QString angle = GetParametrString(domElement, AttrAngle, "0.0");
        QString angleFix = angle;

        VToolLineIntersectAxis::Create(id, name, lineType, lineWeight, lineColor, angleFix, basePointId, firstPointId,
                                       secondPointId, mx, my, showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (angleFix != angle)
        {
            SetAttribute(domElement, AttrAngle, angleFix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection line and axis"),
                                    domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection line and axis"),
                                    domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCurveIntersectAxis(VMainGraphicsScene *scene, QDomElement &domElement,
                                           const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        QString lineType;
        QString lineWeight;
        QString lineColor;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName, lineType, lineWeight, lineColor);

        const quint32 basePointId = GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
        const quint32 curveId = GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
        const QString angle = GetParametrString(domElement, AttrAngle, "0.0");
        QString angleFix = angle;

        VToolCurveIntersectAxis::Create(id, name, lineType, lineWeight, lineColor, angleFix, basePointId, curveId, mx, my,
                                        showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (angleFix != angle)
        {
            SetAttribute(domElement, AttrAngle, angleFix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curve and axis"),
                                    domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curve and axis"),
                                    domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionArcs(VMainGraphicsScene *scene, const QDomElement &domElement,
                                                const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 firstArcId = GetParametrUInt(domElement, AttrFirstArc, NULL_ID_STR);
        const quint32 secondArcId = GetParametrUInt(domElement, AttrSecondArc, NULL_ID_STR);
        const CrossCirclesPoint crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement,
                                                                                  AttrCrossPoint,
                                                                                  "1"));

        VToolPointOfIntersectionArcs::Create(id, name, firstArcId, secondArcId, crossPoint, mx, my, scene, this,
                                             data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection arcs"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionCircles(VMainGraphicsScene *scene, QDomElement &domElement,
                                                   const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 c1CenterId = GetParametrUInt(domElement, AttrC1Center, NULL_ID_STR);
        const quint32 c2CenterId = GetParametrUInt(domElement, AttrC2Center, NULL_ID_STR);
        const QString c1Radius = GetParametrString(domElement, AttrC1Radius);
        QString c1R = c1Radius;
        const QString c2Radius = GetParametrString(domElement, AttrC2Radius);
        QString c2R = c2Radius;
        const CrossCirclesPoint crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement,
                                                                                  AttrCrossPoint, "1"));

        IntersectCirclesTool::Create(id, name, c1CenterId, c2CenterId, c1R, c2R, crossPoint, mx, my,
                                                showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (c1R != c1Radius || c2R != c2Radius)
        {
            SetAttribute(domElement, AttrC1Center, c1R);
            SetAttribute(domElement, AttrC2Center, c2R);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection circles"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointOfIntersectionCurves(VMainGraphicsScene *scene, QDomElement &domElement,
                                                  const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const auto curve1Id = GetParametrUInt(domElement, AttrCurve1, NULL_ID_STR);
        const auto curve2Id = GetParametrUInt(domElement, AttrCurve2, NULL_ID_STR);
        const auto vCrossPoint = static_cast<VCrossCurvesPoint>(GetParametrUInt(domElement, AttrVCrossPoint, "1"));
        const auto hCrossPoint = static_cast<HCrossCurvesPoint>(GetParametrUInt(domElement, AttrHCrossPoint, "1"));

        VToolPointOfIntersectionCurves::Create(id, name, curve1Id, curve2Id, vCrossPoint, hCrossPoint, mx, my,
                                               showPointName, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point of intersection curves"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointFromCircleAndTangent(VMainGraphicsScene *scene, QDomElement &domElement,
                                                  const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 cCenterId = GetParametrUInt(domElement, AttrCCenter, NULL_ID_STR);
        const quint32 tangentId = GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
        const QString cRadius = GetParametrString(domElement, AttrCRadius);
        QString cR = cRadius;
        const CrossCirclesPoint crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement,
                                                                                  AttrCrossPoint,
                                                                                  "1"));

        IntersectCircleTangentTool::Create(id, name, cCenterId, cR, tangentId, crossPoint, mx, my,
                                               showPointName, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (cR != cRadius)
        {
            SetAttribute(domElement, AttrCCenter, cR);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point from circle and tangent"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolPointFromArcAndTangent(VMainGraphicsScene *scene, const QDomElement &domElement,
                                               const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        QString name;
        qreal mx = 0;
        qreal my = 0;
        bool showPointName = true;

        PointsCommonAttributes(domElement, id, name, mx, my, showPointName);
        const quint32 arcId = GetParametrUInt(domElement, AttrArc, NULL_ID_STR);
        const quint32 tangentId = GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
        const CrossCirclesPoint crossPoint = static_cast<CrossCirclesPoint>(GetParametrUInt(domElement,
                                                                                  AttrCrossPoint,
                                                                                  "1"));

        VToolPointFromArcAndTangent::Create(id, name, arcId, tangentId, crossPoint, mx, my,
                                            showPointName, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating point from arc and tangent"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolTrueDarts(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);

        const quint32 p1Id = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        const quint32 p2Id = GetParametrUInt(domElement, AttrPoint2, NULL_ID_STR);

        const quint32 baseLineP1Id = GetParametrUInt(domElement, AttrBaseLineP1, NULL_ID_STR);
        const quint32 baseLineP2Id = GetParametrUInt(domElement, AttrBaseLineP2, NULL_ID_STR);
        const quint32 dartP1Id = GetParametrUInt(domElement, AttrDartP1, NULL_ID_STR);
        const quint32 dartP2Id = GetParametrUInt(domElement, AttrDartP2, NULL_ID_STR);
        const quint32 dartP3Id = GetParametrUInt(domElement, AttrDartP3, NULL_ID_STR);

        const QString name1 = GetParametrString(domElement, AttrName1, "A");
        const qreal mx1 = qApp->toPixel(GetParametrDouble(domElement, AttrMx1, "10.0"));
        const qreal my1 = qApp->toPixel(GetParametrDouble(domElement, AttrMy1, "15.0"));
        const bool showPointName1 = getParameterBool(domElement, AttrShowPointName1, trueStr);

        const QString name2 = GetParametrString(domElement, AttrName2, "A");
        const qreal mx2 = qApp->toPixel(GetParametrDouble(domElement, AttrMx2, "10.0"));
        const qreal my2 = qApp->toPixel(GetParametrDouble(domElement, AttrMy2, "15.0"));
        const bool showPointName2 = getParameterBool(domElement, AttrShowPointName2, trueStr);


        VToolTrueDarts::Create(id, p1Id, p2Id,
                               baseLineP1Id, baseLineP2Id, dartP1Id, dartP2Id, dartP3Id,
                               name1, mx1, my1, showPointName1, name2, mx2, my2, showPointName2,
                               scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating true darts"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// TODO. Delete if minimal supported version is 0.2.7
void VPattern::ParseOldToolSpline(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        const quint32 point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);
        const qreal angle1 = GetParametrDouble(domElement, AttrAngle1, "270.0");
        const qreal angle2 = GetParametrDouble(domElement, AttrAngle2, "90.0");
        const qreal kAsm1 = GetParametrDouble(domElement, AttrKAsm1, "1.0");
        const qreal kAsm2 = GetParametrDouble(domElement, AttrKAsm2, "1.0");
        const qreal kCurve = GetParametrDouble(domElement, AttrKCurve, "1.0");
        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, "0");

        const auto p1 = data->GeometricObject<VPointF>(point1);
        const auto p4 = data->GeometricObject<VPointF>(point4);

        auto spline = new VSpline(*p1, *p4, angle1, angle2, kAsm1, kAsm2, kCurve);
        if (duplicate > 0)
        {
            spline->SetDuplicate(duplicate);
        }
        spline->setLineColor(color);

        VToolSpline::Create(id, spline, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple curve"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolSpline(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        const quint32 point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);

        const QString angle1 = GetParametrString(domElement, AttrAngle1, "0");
        QString a1 = angle1;//need for saving fixed formula;

        const QString angle2 = GetParametrString(domElement, AttrAngle2, "0");
        QString a2 = angle2;//need for saving fixed formula;

        const QString length1 = GetParametrString(domElement, AttrLength1, "0");
        QString l1 = length1;//need for saving fixed formula;

        const QString length2 = GetParametrString(domElement, AttrLength2, "0");
        QString l2 = length2;//need for saving fixed formula;

        const QString color      = GetParametrString(domElement, AttrColor,      ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle,   LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");
        const quint32 duplicate  = GetParametrUInt(domElement,   AttrDuplicate,  "0");

        VToolSpline *spl = VToolSpline::Create(id, point1, point4, a1, a2, l1, l2, duplicate, color, penStyle,
                                               lineWeight, scene, this, data, parse, Source::FromFile);

        if (spl != nullptr)
        {
            VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
            SCASSERT(window != nullptr)
            connect(spl, &VToolSpline::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
        }

        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (a1 != angle1 || a2 != angle2 || l1 != length1 || l2 != length2)
        {
            SetAttribute(domElement, AttrAngle1, a1);
            SetAttribute(domElement, AttrAngle2, a2);
            SetAttribute(domElement, AttrLength1, l1);
            SetAttribute(domElement, AttrLength2, l2);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple curve"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple interactive spline"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCubicBezier(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 point1 = GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
        const quint32 point2 = GetParametrUInt(domElement, AttrPoint2, NULL_ID_STR);
        const quint32 point3 = GetParametrUInt(domElement, AttrPoint3, NULL_ID_STR);
        const quint32 point4 = GetParametrUInt(domElement, AttrPoint4, NULL_ID_STR);

        const QString color      = GetParametrString(domElement, AttrColor,      ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle,   LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");
        const quint32 duplicate  = GetParametrUInt(domElement, AttrDuplicate,    "0");

        auto p1 = data->GeometricObject<VPointF>(point1);
        auto p2 = data->GeometricObject<VPointF>(point2);
        auto p3 = data->GeometricObject<VPointF>(point3);
        auto p4 = data->GeometricObject<VPointF>(point4);

        VCubicBezier *spline = new VCubicBezier(*p1, *p2, *p3, *p4);
        if (duplicate > 0)
        {
            spline->SetDuplicate(duplicate);
        }
        spline->setLineColor(color);
        spline->SetPenStyle(penStyle);
        spline->setLineWeight(lineWeight);

        VToolCubicBezier::Create(id, spline, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cubic bezier curve"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseOldToolSplinePath(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const qreal kCurve = GetParametrDouble(domElement, AttrKCurve, "1.0");
        const QString color = GetParametrString(domElement, AttrColor, ColorBlack);
        const quint32 duplicate = GetParametrUInt(domElement, AttrDuplicate, "0");

        QVector<VFSplinePoint> points;

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        for (qint32 i = 0; i < num; ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (element.isNull() == false)
            {
                if (element.tagName() == AttrPathPoint)
                {
                    const qreal kAsm1 = GetParametrDouble(element, AttrKAsm1, "1.0");
                    const qreal angle = GetParametrDouble(element, AttrAngle, "0");
                    const qreal kAsm2 = GetParametrDouble(element, AttrKAsm2, "1.0");
                    const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                    const VPointF p = *data->GeometricObject<VPointF>(pSpline);

                    QLineF line(0, 0, 100, 0);
                    line.setAngle(angle+180);

                    VFSplinePoint splPoint(p, kAsm1, line.angle(), kAsm2, angle);
                    points.append(splPoint);
                    if (parse == Document::FullParse)
                    {
                        IncrementReferens(p.getIdTool());
                    }
                }
            }
        }

        auto path = new VSplinePath(points, kCurve);
        if (duplicate > 0)
        {
            path->SetDuplicate(duplicate);
        }
        path->setLineColor(color);

        VToolSplinePath::Create(id, path, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating curve path"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolSplinePath(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const QString color      = GetParametrString(domElement, AttrColor,      ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle,   LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");
        const quint32 duplicate  = GetParametrUInt(domElement,   AttrDuplicate,  "0");

        QVector<quint32> points;
        QVector<QString> angle1, a1;
        QVector<QString> angle2, a2;
        QVector<QString> length1, l1;
        QVector<QString> length2, l2;

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        for (qint32 i = 0; i < num; ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (not element.isNull() && element.tagName() == AttrPathPoint)
            {
                angle1.append(GetParametrString(element, AttrAngle1, "0"));
                angle2.append(GetParametrString(element, AttrAngle2, "0"));
                length1.append(GetParametrString(element, AttrLength1, "0"));
                length2.append(GetParametrString(element, AttrLength2, "0"));
                const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                points.append(pSpline);

                if (parse == Document::FullParse)
                {
                    IncrementReferens(data->GeometricObject<VPointF>(pSpline)->getIdTool());
                }
            }
        }

        //need for saving fixed formula;
        a1 = angle1;
        a2 = angle2;
        l1 = length1;
        l2 = length2;

        VToolSplinePath *spl = VToolSplinePath::Create(id, points, a1, a2, l1, l2, color, penStyle, lineWeight,
                                                       duplicate, scene, this, data, parse, Source::FromFile);

        if (spl != nullptr)
        {
            VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
            SCASSERT(window != nullptr)
            connect(spl, &VToolSplinePath::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
        }

        //Rewrite attribute formula. Need for situation when we have wrong formula.
        int count = 0;
        for (qint32 i = 0; i < num; ++i)
        {
            QDomElement element = nodeList.at(i).toElement();
            if (not element.isNull() && element.tagName() == AttrPathPoint)
            {
                if (a1.at(count) != angle1.at(count) || a2.at(count) != angle2.at(count) ||
                    l1.at(count) != length1.at(count) || l2.at(count) != length2.at(count))
                {
                    SetAttribute(element, AttrAngle1, a1.at(count));
                    SetAttribute(element, AttrAngle2, a2.at(count));
                    SetAttribute(element, AttrLength1, l1.at(count));
                    SetAttribute(element, AttrLength2, l2.at(count));
                    modified = true;
                    haveLiteChange();
                }
                ++count;
            }
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating curve path"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating interactive spline path"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolCubicBezierPath(VMainGraphicsScene *scene, const QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const QString color      = GetParametrString(domElement, AttrColor,      ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle,   LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");
        const quint32 duplicate  = GetParametrUInt(domElement,   AttrDuplicate,  "0");

        QVector<VPointF> points;

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        for (qint32 i = 0; i < num; ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (element.isNull() == false)
            {
                if (element.tagName() == AttrPathPoint)
                {
                    const quint32 pSpline = GetParametrUInt(element, AttrPSpline, NULL_ID_STR);
                    const VPointF p = *data->GeometricObject<VPointF>(pSpline);
                    points.append(p);
                    if (parse == Document::FullParse)
                    {
                        IncrementReferens(p.getIdTool());
                    }
                }
            }
        }

        auto path = new VCubicBezierPath(points);
        if (duplicate > 0)
        {
            path->SetDuplicate(duplicate);
        }
        path->setLineColor(color);
        path->SetPenStyle(penStyle);
        path->setLineWeight(lineWeight);

        VToolCubicBezierPath::Create(id, path, scene, this, data, parse, Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating cubic bezier path curve"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeSpline(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        quint32 idObject = 0;
        quint32 idTool = 0;

        SplinesCommonAttributes(domElement, id, idObject, idTool);
        try
        {
            const auto obj = data->GetGObject(idObject);
            if (obj->getType() == GOType::Spline)
            {
                VSpline *spl = new VSpline(*data->GeometricObject<VSpline>(idObject));
                spl->setIdObject(idObject);
                spl->setMode(Draw::Modeling);
                data->UpdateGObject(id, spl);
            }
            else
            {
                VCubicBezier *spl = new VCubicBezier(*data->GeometricObject<VCubicBezier>(idObject));
                spl->setIdObject(idObject);
                spl->setMode(Draw::Modeling);
                data->UpdateGObject(id, spl);
            }
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(error)
            return;// Just ignore
        }

        VNodeSpline::Create(this, data, id, idObject, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling simple curve"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeSplinePath(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;
        quint32 idObject = 0;
        quint32 idTool = 0;

        SplinesCommonAttributes(domElement, id, idObject, idTool);
        try
        {
            const auto obj = data->GetGObject(idObject);
            if (obj->getType() == GOType::SplinePath)
            {
                VSplinePath *path = new VSplinePath(*data->GeometricObject<VSplinePath>(idObject));
                path->setIdObject(idObject);
                path->setMode(Draw::Modeling);
                data->UpdateGObject(id, path);
            }
            else
            {
                VCubicBezierPath *spl = new VCubicBezierPath(*data->GeometricObject<VCubicBezierPath>(idObject));
                spl->setIdObject(idObject);
                spl->setMode(Draw::Modeling);
                data->UpdateGObject(id, spl);
            }
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(error)
            return;// Just ignore
        }
        VNodeSplinePath::Create(this, data, id, idObject, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling curve path"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 center     = GetParametrUInt(domElement,   AttrCenter,     NULL_ID_STR);
        const QString radius     = GetParametrString(domElement, AttrRadius,     "10");
              QString r          = radius;//need for saving fixed formula;
        const QString f1         = GetParametrString(domElement, AttrAngle1,     "180");
              QString f1Fix      = f1;//need for saving fixed formula;
        const QString f2         = GetParametrString(domElement, AttrAngle2,     "270");
              QString f2Fix      = f2;//need for saving fixed formula;
        const QString color      = GetParametrString(domElement, AttrColor,      ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle,   LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");

        VToolArc::Create(id, center, r, f1Fix, f2Fix, color, penStyle, lineWeight, scene, this, data,
                         parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r != radius || f1Fix != f1 || f2Fix != f2)
        {
            SetAttribute(domElement, AttrRadius, r);
            SetAttribute(domElement, AttrAngle1, f1Fix);
            SetAttribute(domElement, AttrAngle2, f2Fix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolEllipticalArc(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 center     = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        const QString radius1    = GetParametrString(domElement, AttrRadius1, "10");
        const QString radius2    = GetParametrString(domElement, AttrRadius2, "10");
        QString r1               = radius1;//need for saving fixed formula;
        QString r2               = radius2;//need for saving fixed formula;
        const QString f1         = GetParametrString(domElement, AttrAngle1, "180");
        QString f1Fix            = f1;//need for saving fixed formula;
        const QString f2         = GetParametrString(domElement, AttrAngle2, "270");
        QString f2Fix            = f2;//need for saving fixed formula;
        const QString frotation  = GetParametrString(domElement, AttrRotationAngle, "0");
        QString frotationFix     = frotation;//need for saving fixed formula;
        const QString color      = GetParametrString(domElement, AttrColor, ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle, LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");

        VToolEllipticalArc::Create(id, center, r1, r2, f1Fix, f2Fix, frotationFix, color, penStyle, lineWeight,
                                   scene, this, data,
                                   parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r1 != radius1 || r2 != radius2 || f1Fix != f1 || f2Fix != f2 || frotationFix != frotation)
        {
            SetAttribute(domElement, AttrRadius1, r1);
            SetAttribute(domElement, AttrRadius2, r2);
            SetAttribute(domElement, AttrAngle1, f1Fix);
            SetAttribute(domElement, AttrAngle2, f2Fix);
            SetAttribute(domElement, AttrRotationAngle, frotationFix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple elliptical arc"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple elliptical arc"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeEllipticalArc(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        const quint32 idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        VEllipticalArc *arc = nullptr;
        try
        {
            arc = new VEllipticalArc(*data->GeometricObject<VEllipticalArc>(idObject));
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(error)
            return;// Just ignore
        }
        arc->setIdObject(idObject);
        arc->setMode(Draw::Modeling);
        data->UpdateGObject(id, arc);
        VNodeEllipticalArc::Create(this, data, id, idObject, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling elliptical arc"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseNodeArc(const QDomElement &domElement, const Document &parse)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 idObject = GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
        const quint32 idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        VArc *arc = nullptr;
        try
        {
            arc = new VArc(*data->GeometricObject<VArc>(idObject));
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(error)
            return;// Just ignore
        }
        arc->setIdObject(idObject);
        arc->setMode(Draw::Modeling);
        data->UpdateGObject(id, arc);
        VNodeArc::Create(this, data, id, idObject, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating modeling arc"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolArcWithLength(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = 0;

        ToolsCommonAttributes(domElement, id);
        const quint32 center     = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        const QString radius     = GetParametrString(domElement, AttrRadius, "10");
        QString r                = radius;//need for saving fixed formula;
        const QString f1         = GetParametrString(domElement, AttrAngle1, "180");
        QString f1Fix            = f1;//need for saving fixed formula;
        const QString length     = GetParametrString(domElement, AttrLength, "10");
        QString lengthFix        = length;//need for saving fixed length;
        const QString color      = GetParametrString(domElement, AttrColor, ColorBlack);
        const QString penStyle   = GetParametrString(domElement, AttrPenStyle, LineTypeSolidLine);
        const QString lineWeight = GetParametrString(domElement, AttrLineWeight, "0.35");

        VToolArcWithLength::Create(id, center, r, f1Fix, lengthFix, color, penStyle, lineWeight, scene, this,
                                   data, parse,
                                   Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (r != radius || f1Fix != f1 || lengthFix != length)
        {
            SetAttribute(domElement, AttrRadius, r);
            SetAttribute(domElement, AttrAngle1, f1Fix);
            SetAttribute(domElement, AttrLength, lengthFix);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating simple arc"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolRotation(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = NULL_ID;

        ToolsCommonAttributes(domElement, id);
        const quint32 center = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        const QString angle = GetParametrString(domElement, AttrAngle, "10");
        QString a = angle;//need for saving fixed formula;
        const QString suffix = GetParametrString(domElement, AttrSuffix, "");

        QVector<SourceItem> source;
        QVector<DestinationItem> destination;
        VAbstractOperation::ExtractData(domElement, source, destination);

        VToolRotation::Create(id, center, a, suffix, source, destination, scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (a != angle)
        {
            SetAttribute(domElement, AttrAngle, a);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of rotation"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of rotation"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolMirrorByLine(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = NULL_ID;

        ToolsCommonAttributes(domElement, id);
        const quint32 p1 = GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
        const quint32 p2 = GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
        const QString suffix = GetParametrString(domElement, AttrSuffix, "");

        QVector<SourceItem> source;;
        QVector<DestinationItem> destination;
        VAbstractOperation::ExtractData(domElement, source, destination);

        VToolMirrorByLine::Create(id, p1, p2, suffix, source, destination, scene, this, data, parse,
                                    Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of mirror by line"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolMirrorByAxis(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = NULL_ID;

        ToolsCommonAttributes(domElement, id);
        const quint32 origin = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
        const auto axisType = static_cast<AxisType>(GetParametrUInt(domElement, AttrAxisType, "1"));
        const QString suffix = GetParametrString(domElement, AttrSuffix, "");

        QVector<SourceItem> source;;
        QVector<DestinationItem> destination;
        VAbstractOperation::ExtractData(domElement, source, destination);

        VToolMirrorByAxis::Create(id, origin, axisType, suffix, source, destination, scene, this, data, parse,
                                    Source::FromFile);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of mirror by axis"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseToolMove(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");

    try
    {
        quint32 id = NULL_ID;

        ToolsCommonAttributes(domElement, id);
        const QString angle = GetParametrString(domElement, AttrAngle, "0");
        QString a = angle;//need for saving fixed formula;
        const QString length = GetParametrString(domElement, AttrLength, "0");
        QString len = length;//need for saving fixed formula;

        const QString rotation = GetParametrString(domElement, AttrRotationAngle, "0");
        QString rot = rotation;//need for saving fixed formula;
        quint32 originPointId = GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);

        const QString suffix = GetParametrString(domElement, AttrSuffix, "");

        QVector<SourceItem> source;;
        QVector<DestinationItem> destination;
        VAbstractOperation::ExtractData(domElement, source, destination);

        VToolMove::Create(id, a, len, rot, originPointId, suffix, source, destination,
                          scene, this, data, parse, Source::FromFile);
        //Rewrite attribute formula. Need for situation when we have wrong formula.
        if (a != angle || len != length || rot != rotation)
        {
            SetAttribute(domElement, AttrAngle, angle);
            SetAttribute(domElement, AttrLength, length);
            SetAttribute(domElement, AttrRotationAngle, rotation);
            modified = true;
            haveLiteChange();
        }
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of moving"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
    catch (qmu::QmuParserError &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating operation of moving"), domElement);
        excep.AddMoreInformation(QString("Message:     " + error.GetMsg() + "\n"+ "Expression:  " + error.GetExpr()));
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPattern::EvalFormula(VContainer *data, const QString &formula, bool *ok) const
{
    if (formula.isEmpty())
    {
        *ok = true;
        return 0;
    }
    else
    {
        try
        {
            // Replace line return character with spaces for calc if exist
            QString f = formula;
            f.replace("\n", " ");
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(data->DataVariables(), f);

            (qIsInf(result) || qIsNaN(result)) ? *ok = false : *ok = true;
            return result;
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            *ok = false;
            return 0;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPattern::createEmptyVariable(const QString &name)
{
    QDomElement element = createElement(TagVariable);
    SetAttribute(element, VariableName, name);
    SetAttribute(element, VariableFormula, QString("0"));
    SetAttribute(element, VariableDescription, QString(""));
    return element;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VPattern::findVariable(const QString &name) const
{
    QDomNodeList list = elementsByTagName(TagVariable);

    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            const QString parameter = domElement.attribute(VariableName);
            if (parameter == name)
            {
                return domElement;
            }
        }
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::GarbageCollector()
{
    QDomNodeList modelingList = elementsByTagName(TagModeling);
    for (int i=0; i < modelingList.size(); ++i)
    {
        QDomElement modElement = modelingList.at(i).toElement();
        if (not modElement.isNull())
        {
            QDomElement modNode = modElement.firstChild().toElement();
            while (not modNode.isNull())
            {
                // First get next sibling because later will not have chance to get it
                QDomElement nextSibling = modNode.nextSibling().toElement();
                if (modNode.hasAttribute(VAbstractNode::AttrInUse))
                {
                    const NodeUsage inUse = GetParametrUsage(modNode, VAbstractNode::AttrInUse);
                    if (inUse == NodeUsage::InUse)
                    { // It is dangerous to leave object with attribute 'inUse'
                      // Each parse should confirm this status.
                        SetParametrUsage(modNode, VAbstractNode::AttrInUse, NodeUsage::NotInUse);
                    }
                    else
                    { // Parent was deleted. We do not need this object anymore
                        modElement.removeChild(modNode);
                    }
                }
                else
                { // Each parse should confirm his status.
                    SetParametrUsage(modNode, VAbstractNode::AttrInUse, NodeUsage::NotInUse);
                }

                modNode = nextSibling;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseSplineElement parse spline tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseSplineElement(VMainGraphicsScene *scene, QDomElement &domElement,
                                  const Document &parse, const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(type.isEmpty() == false, Q_FUNC_INFO, "type of spline is empty");

    QStringList splines = QStringList() << VToolSpline::OldToolType        /*0*/
                                        << VToolSpline::ToolType           /*1*/
                                        << VToolSplinePath::OldToolType    /*2*/
                                        << VToolSplinePath::ToolType       /*3*/
                                        << VNodeSpline::ToolType           /*4*/
                                        << VNodeSplinePath::ToolType       /*5*/
                                        << VToolCubicBezier::ToolType      /*6*/
                                        << VToolCubicBezierPath::ToolType; /*7*/
    switch (splines.indexOf(type))
    {
        case 0: //VToolSpline::OldToolType
            qCDebug(vXML, "VOldToolSpline.");
            ParseOldToolSpline(scene, domElement, parse);// TODO. Delete if minimal supported version is 0.2.7
            break;
        case 1: //VToolSpline::ToolType
            qCDebug(vXML, "VToolSpline.");
            ParseToolSpline(scene, domElement, parse);
            break;
        case 2: //VToolSplinePath::OldToolType
            qCDebug(vXML, "VOldToolSplinePath.");
            ParseOldToolSplinePath(scene, domElement, parse);// TODO. Delete if minimal supported version is 0.2.7
            break;
        case 3: //VToolSplinePath::ToolType
            qCDebug(vXML, "VToolSplinePath.");
            ParseToolSplinePath(scene, domElement, parse);
            break;
        case 4: //VNodeSpline::ToolType
            qCDebug(vXML, "VNodeSpline.");
            ParseNodeSpline(domElement, parse);
            break;
        case 5: //VNodeSplinePath::ToolType
            qCDebug(vXML, "VNodeSplinePath.");
            ParseNodeSplinePath(domElement, parse);
            break;
        case 6: //VToolCubicBezier::ToolType
            qCDebug(vXML, "VToolCubicBezier.");
            ParseToolCubicBezier(scene, domElement, parse);
            break;
        case 7: //VToolCubicBezierPath::ToolType
            qCDebug(vXML, "VToolCubicBezierPath.");
            ParseToolCubicBezierPath(scene, domElement, parse);
            break;
        default:
            VException e(tr("Unknown spline type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseArcElement parse arc tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseArcElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                               const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of arc is empty");

    QStringList arcs = QStringList() << VToolArc::ToolType            /*0*/
                                     << VNodeArc::ToolType            /*1*/
                                     << VToolArcWithLength::ToolType; /*2*/

    switch (arcs.indexOf(type))
    {
        case 0: //VToolArc::ToolType
            ParseToolArc(scene, domElement, parse);
            break;
        case 1: //VNodeArc::ToolType
            ParseNodeArc(domElement, parse);
            break;
        case 2: //VToolArcWithLength::ToolType
            ParseToolArcWithLength(scene, domElement, parse);
            break;
        default:
            VException e(tr("Unknown arc type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseEllipticalArcElement parse elliptical arc tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseEllipticalArcElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                               const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of elliptical arc is empty");

    const QStringList arcs = QStringList() << VToolEllipticalArc::ToolType  /*0*/
                                           << VNodeEllipticalArc::ToolType; /*1*/

    switch (arcs.indexOf(type))
    {
        case 0: //VToolArc::ToolType
            ParseToolEllipticalArc(scene, domElement, parse);
            break;
        case 1: //VNodeEllipticalArc::ToolType
            ParseNodeEllipticalArc(domElement, parse);
            break;
        default:
            VException e(tr("Unknown elliptical arc type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ParseToolsElement parse tools tag.
 * @param scene scene.
 * @param domElement tag in xml tree.
 * @param parse parser file mode.
 * @param type type of spline.
 */
void VPattern::ParseToolsElement(VMainGraphicsScene *scene, const QDomElement &domElement,
                                 const Document &parse, const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(domElement.isNull() == false, Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(type.isEmpty() == false, Q_FUNC_INFO, "type of spline is empty");

    const QStringList tools = QStringList() << UnionTool::ToolType;
    switch (tools.indexOf(type))
    {
        case 0: //UnionTool::ToolType
            try
            {
                quint32 id = 0;
                ToolsCommonAttributes(domElement, id);

                UnionToolInitData initData;
                initData.piece1_Index = GetParametrUInt(domElement, UnionTool::AttrIndexD1, "-1");
                initData.piece2_Index = GetParametrUInt(domElement, UnionTool::AttrIndexD2, "-1");
                initData.scene = scene;
                initData.doc = this;
                initData.data = data;
                initData.parse = parse;
                initData.typeCreation = Source::FromFile;

                UnionTool::Create(id, initData);
            }
            catch (const VExceptionBadId &error)
            {
                VExceptionObjectError excep(tr("Error creating or updating union pieces"), domElement);
                excep.AddMoreInformation(error.ErrorMessage());
                throw excep;
            }
            break;
        default:
            VException e(tr("Unknown tools type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParseOperationElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse,
                                     const QString &type)
{
    SCASSERT(scene != nullptr)
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    Q_ASSERT_X(not type.isEmpty(), Q_FUNC_INFO, "type of operation is empty");

    const QStringList opers = QStringList() << VToolRotation::ToolType        /*0*/
                                            << VToolMirrorByLine::ToolType  /*1*/
                                            << VToolMirrorByAxis::ToolType  /*2*/
                                            << VToolMove::ToolType;           /*3*/

    switch (opers.indexOf(type))
    {
        case 0: //VToolRotation::ToolType
            ParseToolRotation(scene, domElement, parse);
            break;
        case 1: //VToolMirrorByLine::ToolType
            ParseToolMirrorByLine(scene, domElement, parse);
            break;
        case 2: //VToolMirrorByAxis::ToolType
            ParseToolMirrorByAxis(scene, domElement, parse);
            break;
        case 3: //VToolMove::ToolType
            ParseToolMove(scene, domElement, parse);
            break;
        default:
            VException e(tr("Unknown operation type '%1'.").arg(type));
            throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ParsePathElement(VMainGraphicsScene *scene, QDomElement &domElement, const Document &parse)
{
    SCASSERT(scene != nullptr);
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");
    try
    {
        quint32 id = 0;
        ToolsCommonAttributes(domElement, id);
        const QString name = GetParametrString(domElement, AttrName, tr("Unnamed path"));
        const QString defType = QString().setNum(static_cast<int>(PiecePathType::CustomSeamAllowance));
        const PiecePathType type = static_cast<PiecePathType>(GetParametrUInt(domElement, AttrType, defType));
        const quint32 idTool = GetParametrUInt(domElement, VAbstractNode::AttrIdTool, NULL_ID_STR);
        const QString penType = GetParametrString(domElement, AttrLineType, LineTypeSolidLine);
        const bool cut = getParameterBool(domElement, AttrCut, falseStr);

        VPiecePath path;
        const QDomElement element = domElement.firstChildElement(VAbstractPattern::TagNodes);
        if (not element.isNull())
        {
            path = ParsePathNodes(element);
        }

        path.SetType(type);
        path.SetName(name);
        path.SetPenType(lineTypeToPenStyle(penType));
        path.SetCutPath(cut);

        VToolInternalPath::Create(id, path, 0, scene, this, data, parse, Source::FromFile, "", idTool);
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating a piece path"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief parseVariablesElement parse variables tag.
 * @param node tag in xml tree.
 */
void VPattern::parseVariablesElement(const QDomNode &node)
{
    int index = 0;
    QDomNode domNode = node.firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                if (domElement.tagName() == TagVariable)
                {
                    const QString name = GetParametrString(domElement, VariableName, "");

                    QString desc;
                    try
                    {
                        desc = GetParametrString(domElement, VariableDescription);
                    }
                    catch (VExceptionEmptyParameter &error)
                    {
                        Q_UNUSED(error)
                    }

                    const QString formula = GetParametrString(domElement, VariableFormula, "0");
                    bool ok = false;
                    const qreal value = EvalFormula(data, formula, &ok);

                    data->AddVariable(name, new CustomVariable(data, name, static_cast<quint32>(index), value, formula, ok,
                                                           desc));
                    ++index;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::addEmptyCustomVariable(const QString &name)
{
    const QDomElement element = createEmptyVariable(name);

    const QDomNodeList list = elementsByTagName(TagVariables);
    list.at(0).appendChild(element);
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::addEmptyCustomVariableAfter(const QString &after, const QString &name)
{
    const QDomElement element = createEmptyVariable(name);
    const QDomElement sibling = findVariable(after);

    const QDomNodeList list = elementsByTagName(TagVariables);

    if (sibling.isNull())
    {
        list.at(0).appendChild(element);
    }
    else
    {
        list.at(0).insertAfter(element, sibling);
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::removeCustomVariable(const QString &name)
{
    const QDomNodeList list = elementsByTagName(TagVariables);
    list.at(0).removeChild(findVariable(name));
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::moveVariableUp(const QString &name)
{
    const QDomElement node = findVariable(name);
    if (not node.isNull())
    {
        const QDomElement prSibling = node.previousSiblingElement(TagVariable);
        if (not prSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagVariables);
            list.at(0).insertBefore(node, prSibling);
        }
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::moveVariableDown(const QString &name)
{
    const QDomElement node = findVariable(name);
    if (not node.isNull())
    {
        const QDomElement nextSibling = node.nextSiblingElement(TagVariable);
        if (not nextSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagVariables);
            list.at(0).insertAfter(node, nextSibling);
        }
    }
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::setVariableName(const QString &name, const QString &text)
{
    QDomElement node = findVariable(name);
    if (not node.isNull())
    {
        SetAttribute(node, VariableName, text);
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::setVariableFormula(const QString &name, const QString &text)
{
    QDomElement node = findVariable(name);
    if (not node.isNull())
    {
        SetAttribute(node, VariableFormula, text);
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::setVariableDescription(const QString &name, const QString &text)
{
    QDomElement node = findVariable(name);
    if (not node.isNull())
    {
        SetAttribute(node, VariableDescription, text);
        emit patternChanged(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::replaceNameInFormula(QVector<VFormulaField> &expressions, const QString &name, const QString &newName)
{
    const int bias = name.length() - newName.length();

    for(int i = 0; i < expressions.size(); ++i)
    {
        if (expressions.at(i).expression.indexOf(name) != -1)
        {
            QMap<int, QString> tokens;

            // Eval formula
            try
            {
                QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(expressions.at(i).expression, false,
                                                                                false));
                tokens = cal->GetTokens();// Tokens (variables, measurements)

            }
            catch (const qmu::QmuParserError &)
            {
                continue;// Because we not sure if used. A formula is broken.
            }

            QList<QString> tValues = tokens.values();
            if (not tValues.contains(name))
            {
                continue;
            }

            QList<int> tKeys = tokens.keys();// Take all tokens positions
            QString newFormula = expressions.at(i).expression;

            for (int i = 0; i < tKeys.size(); ++i)
            {
                if (tValues.at(i) != name)
                {
                    continue;
                }

                newFormula.replace(tKeys.at(i), name.length(), newName);

                if (bias != 0)
                {// Translated token has different length than original. Position next tokens need to be corrected.
                    VTranslateVars::BiasTokens(tKeys.at(i), bias, tokens);
                    tKeys = tokens.keys();
                    tValues = tokens.values();
                }
            }

            expressions[i].expression = newFormula;
            expressions[i].element.setAttribute(expressions.at(i).attribute, newFormula);
            emit patternChanged(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GenerateLabel create name for draft block basepoints.
 * @param type type of the label.
 * @param reservedName reversed point name. Use when need reserve name, but point is not in data base yet.
 * @return unique name for current draft block.
 */
QString VPattern::GenerateLabel(const LabelType &type, const QString &reservedName) const
{
    if (type == LabelType::NewPatternPiece)
    {
        const QDomNodeList blockList = elementsByTagName(TagDraftBlock);
        QString name;
        int i = 0;
        for (;;)
        {
            name = GetLabelBase(static_cast<quint32>(blockList.size() + i));
            if (data->IsUnique(name))
            {
                return name;
            }
            if (i == INT_MAX)
            {
                break;
            }
            ++i;
        }
        qCDebug(vXML, "Point name: %s", qUtf8Printable(name));
        return name;
    }
    else if (type == LabelType::NewLabel)
    {
        QString labelBase = defaultBasePoint;
        if (defaultBasePoint.isEmpty())
        {
            labelBase = GetLabelBase(static_cast<quint32>(getActiveDraftBlockIndex()));
        }

        qint32 num = 1;
        QString name;
        do
        {
            name = QString("%1%2").arg(labelBase).arg(num);
            num++;
            if (num == INT_MAX)
            {
                break;
            }
        } while (data->IsUnique(name) == false || name == reservedName);
        qCDebug(vXML, "Point name: %s", qUtf8Printable(name));
        return name;
    }
    qCDebug(vXML, "Got unknown type %d", static_cast<int>(type));
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
QString VPattern::GenerateSuffix(const QString &type) const
{
    const QString suffixBase = GetLabelBase(static_cast<quint32>(getActiveDraftBlockIndex())).toLower();
    const QStringList uniqueNames = VContainer::AllUniqueNames();
    qint32 num = 1;
    QString suffix;
    for (;;)
    {
        suffix = QString("_%1%2%3").arg(suffixBase).arg(num).arg(type);

        for (int i=0; i < uniqueNames.size(); ++i)
        {
            if (not data->IsUnique(uniqueNames.at(i) + suffix))
            {
                break;
            }

            if (i == uniqueNames.size()-1)
            {
                qCDebug(vXML, "Suffix is: %s", qUtf8Printable(suffix));
                return suffix;
            }
        }

        if (num == INT_MAX)
        {
            break;
        }
        ++num;
    }
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPattern::IsDefCustom() const
{
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.size() == 0)
    {
        return false;
    }

    const QDomNode domNode = tags.at(0);
    const QDomElement domElement = domNode.toElement();
    if (domElement.isNull() == false)
    {
        return getParameterBool(domElement, AttrCustom, falseStr);
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetDefCustom(bool value)
{
    CheckTagExists(TagGradation);
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.isEmpty())
    {
        qWarning() << "Can't save attribute " << AttrCustom << Q_FUNC_INFO;
        return;
    }

    QDomNode domNode = tags.at(0);
    QDomElement domElement = domNode.toElement();
    if (domElement.isNull() == false)
    {
        if (value == false)
        {
            domElement.removeAttribute(AttrDefHeight);
            SetDefCustomHeight(0);
            SetDefCustomSize(0);
        }
        else
        {
            SetAttribute(domElement, AttrCustom, value);
        }
        modified = true;
    }
    else
    {
        qWarning() << "Can't save attribute " << AttrCustom << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VPattern::GetDefCustomHeight() const
{
    if (IsDefCustom())
    {
        QDomNodeList tags = elementsByTagName(TagGradation);
        if (tags.size() == 0)
        {
            return 0;
        }

        const QDomNode domNode = tags.at(0);
        const QDomElement domElement = domNode.toElement();
        if (domElement.isNull() == false)
        {
            return static_cast<int>(GetParametrUInt(domElement, AttrDefHeight, QStringLiteral("0")));
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetDefCustomHeight(int value)
{
    CheckTagExists(TagGradation);
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.isEmpty())
    {
        qWarning() << "Can't save attribute " << AttrDefHeight << Q_FUNC_INFO;
        return;
    }

    QDomNode domNode = tags.at(0);
    QDomElement domElement = domNode.toElement();
    if (domElement.isNull() == false)
    {
        if (value == 0)
        {
            domElement.removeAttribute(AttrDefHeight);
        }
        else
        {
            SetAttribute(domElement, AttrDefHeight, value);
        }
        modified = true;
    }
    else
    {
        qWarning() << "Can't save attribute " << AttrDefHeight << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VPattern::GetDefCustomSize() const
{
    if (IsDefCustom())
    {
        QDomNodeList tags = elementsByTagName(TagGradation);
        if (tags.size() == 0)
        {
            return 0;
        }

        const QDomNode domNode = tags.at(0);
        const QDomElement domElement = domNode.toElement();
        if (domElement.isNull() == false)
        {
            return static_cast<int>(GetParametrUInt(domElement, AttrDefSize, QStringLiteral("0")));
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetDefCustomSize(int value)
{
    CheckTagExists(TagGradation);
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.isEmpty())
    {
        qWarning() << "Can't save attribute " << AttrDefSize << Q_FUNC_INFO;
        return;
    }

    QDomNode domNode = tags.at(0);
    QDomElement domElement = domNode.toElement();
    if (domElement.isNull() == false)
    {
        if (value == 0)
        {
            domElement.removeAttribute(AttrDefSize);
        }
        else
        {
            SetAttribute(domElement, AttrDefSize, value);
        }
        modified = true;
    }
    else
    {
        qWarning() << "Can't save attribute " << AttrDefSize << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPattern::isReadOnly() const
{
    const QDomElement pattern = documentElement();

    if (pattern.isNull())
    {
        return false;
    }

    return getParameterBool(pattern, AttrReadOnly, falseStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::SetReadOnly(bool rOnly)
{
    QDomElement pattern = documentElement();

    if (not pattern.isNull())
    {
        if (rOnly)
        {
            SetAttribute(pattern, AttrReadOnly, rOnly);
        }
        else
        {// For better backward compatibility
            pattern.removeAttribute(AttrReadOnly);
        }
        modified = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::PrepareForParse(const Document &parse)
{
    SCASSERT(draftScene != nullptr)
    SCASSERT(pieceScene != nullptr)
    if (parse == Document::FullParse)
    {
        TestUniqueId();
        draftScene->clear();
        draftScene->initializeOrigins();
        pieceScene->clear();
        pieceScene->initializeOrigins();
        data->ClearForFullParse();
        activeDraftBlock.clear();
        patternPieces.clear();
        clearBackgroundImageMap();

        qDeleteAll(toolsOnRemove);//Remove all invisible on a scene objects.
        toolsOnRemove.clear();

        tools.clear();
        cursor = 0;
        history.clear();
    }
    else if (parse == Document::LiteParse)
    {
        VContainer::ClearUniqueNames();
        data->ClearVariables(VarType::Variable);
        data->ClearVariables(VarType::LineAngle);
        data->ClearVariables(VarType::LineLength);
        data->ClearVariables(VarType::CurveLength);
        data->ClearVariables(VarType::CurveCLength);
        data->ClearVariables(VarType::ArcRadius);
        data->ClearVariables(VarType::CurveAngle);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPattern::ToolsCommonAttributes(const QDomElement &domElement, quint32 &id)
{
    id = getParameterId(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

QRectF VPattern::ActiveDrawBoundingRect() const
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used.");

    QRectF rect;

    for (qint32 i = 0; i< history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        if (tool.getDraftBlockName() == activeDraftBlock)
        {
            switch ( tool.getTypeTool() )
            {
                case Tool::Arrow:
                case Tool::SinglePoint:
                case Tool::DoublePoint:
                case Tool::LinePoint:
                case Tool::AbstractSpline:
                case Tool::Cut:
                case Tool::Midpoint:// Same as Tool::AlongLine, but tool will never has such type
                case Tool::ArcIntersectAxis:// Same as Tool::CurveIntersectAxis, but tool will never has such type
                case Tool::BackgroundImage:
                case Tool::LAST_ONE_DO_NOT_USE:
                    Q_UNREACHABLE();
                    break;
                case Tool::BasePoint:
                case Tool::LineIntersect:
                case Tool::PointOfContact:
                case Tool::Triangle:
                case Tool::PointOfIntersection:
                case Tool::CutArc:
                case Tool::CutSpline:
                case Tool::CutSplinePath:
                case Tool::PointOfIntersectionArcs:
                case Tool::PointOfIntersectionCircles:
                case Tool::PointOfIntersectionCurves:
                case Tool::PointFromCircleAndTangent:
                case Tool::PointFromArcAndTangent:
                    rect = ToolBoundingRect<VToolSinglePoint>(rect, tool.getId());
                    break;
                case Tool::EndLine:
                case Tool::AlongLine:
                case Tool::ShoulderPoint:
                case Tool::Normal:
                case Tool::Bisector:
                case Tool::Height:
                case Tool::LineIntersectAxis:
                case Tool::CurveIntersectAxis:
                    rect = ToolBoundingRect<VToolLinePoint>(rect, tool.getId());
                    break;
                case Tool::Line:
                    rect = ToolBoundingRect<VToolLine>(rect, tool.getId());
                    break;
                case Tool::Spline:
                case Tool::CubicBezier:
                case Tool::Arc:
                case Tool::SplinePath:
                case Tool::CubicBezierPath:
                case Tool::ArcWithLength:
                case Tool::EllipticalArc:
                    rect = ToolBoundingRect<VAbstractSpline>(rect, tool.getId());
                    break;
                case Tool::TrueDarts:
                    rect = ToolBoundingRect<VToolDoublePoint>(rect, tool.getId());
                    break;
                case Tool::Rotation:
                case Tool::MirrorByLine:
                case Tool::MirrorByAxis:
                case Tool::Move:
                    rect = ToolBoundingRect<VAbstractOperation>(rect, tool.getId());
                    break;
                //These tools are not accesseble in Draw mode, but still 'history' contains them.
                case Tool::Piece:
                case Tool::Union:
                case Tool::NodeArc:
                case Tool::NodeElArc:
                case Tool::NodePoint:
                case Tool::NodeSpline:
                case Tool::NodeSplinePath:
                case Tool::Group:
                case Tool::InternalPath:
                case Tool::AnchorPoint:
                case Tool::InsertNodes:
                    break;
            }
        }
    }
    return rect;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
QRectF VPattern::ToolBoundingRect(const QRectF &rect, const quint32 &id) const
{
    QRectF toolRect = rect;
    if (tools.contains(id))
    {
        const T *vTool = qobject_cast<T *>(tools.value(id));
        SCASSERT(vTool != nullptr)

        QRectF childrenRect = vTool->childrenBoundingRect();
        //map to scene coordinate.
        childrenRect.translate(vTool->scenePos());

        toolRect = toolRect.united(vTool->sceneBoundingRect());
        toolRect = toolRect.united(childrenRect);
    }
    else
    {
        qWarning() << "Can't find tool with id=" << id;
    }
    return toolRect;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IncrementReferens increment reference parent objects.
 * @param id parent object id.
 */
void VPattern::IncrementReferens(quint32 id) const
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0");
    ToolExists(id);
    VDataTool *tool = tools.value(id);
    SCASSERT(tool != nullptr)
    tool->incrementReferens();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DecrementReferens decrement reference parent objects.
 * @param id parent object id.
 */
void VPattern::DecrementReferens(quint32 id) const
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0");
    ToolExists(id);
    VDataTool *tool = tools.value(id);
    SCASSERT(tool != nullptr)
    tool->decrementReferens();
}

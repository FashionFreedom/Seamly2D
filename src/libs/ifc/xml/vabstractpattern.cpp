/***************************************************************************
 **  @file   vabstractpattern.cpp
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
 **
 **  @file   vabstractpattern.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vabstractpattern.h"

#include <QDomNode>
#include <QDomNodeList>
#include <QLatin1String>
#include <QList>
#include <QMessageBox>
#include <QMessageLogger>
#include <QSet>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QtDebug>

#include "vdomdocument.h"
#include "vpatternconverter.h"
#include "vtoolrecord.h"
#include "../exception/vexceptionemptyparameter.h"
#include "../exception/vexceptionobjecterror.h"
#include "../exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vtools/tools/vdatatool.h"

class QDomElement;

const QString VAbstractPattern::TagPattern              = QStringLiteral("pattern");
const QString VAbstractPattern::TagCalculation          = QStringLiteral("calculation");
const QString VAbstractPattern::TagModeling             = QStringLiteral("modeling");
const QString VAbstractPattern::TagPieces               = QStringLiteral("pieces");
const QString VAbstractPattern::TagPiece                = QStringLiteral("piece");
const QString VAbstractPattern::TagDescription          = QStringLiteral("description");
const QString VAbstractPattern::TagNotes                = QStringLiteral("notes");
const QString VAbstractPattern::TagImage                = QStringLiteral("image");
const QString VAbstractPattern::TagMeasurements         = QStringLiteral("measurements");
const QString VAbstractPattern::TagIncrements           = QStringLiteral("increments");
const QString VAbstractPattern::TagIncrement            = QStringLiteral("increment");
const QString VAbstractPattern::TagDraftBlock           = QStringLiteral("draftBlock");
const QString VAbstractPattern::TagGroups               = QStringLiteral("groups");
const QString VAbstractPattern::TagGroup                = QStringLiteral("group");
const QString VAbstractPattern::TagGroupItem            = QStringLiteral("item");
const QString VAbstractPattern::TagPoint                = QStringLiteral("point");
const QString VAbstractPattern::TagSpline               = QStringLiteral("spline");
const QString VAbstractPattern::TagArc                  = QStringLiteral("arc");
const QString VAbstractPattern::TagElArc                = QStringLiteral("elArc");
const QString VAbstractPattern::TagTools                = QStringLiteral("tools");
const QString VAbstractPattern::TagOperation            = QStringLiteral("operation");
const QString VAbstractPattern::TagGradation            = QStringLiteral("gradation");
const QString VAbstractPattern::TagHeights              = QStringLiteral("heights");
const QString VAbstractPattern::TagSizes                = QStringLiteral("sizes");
const QString VAbstractPattern::TagData                 = QStringLiteral("data");
const QString VAbstractPattern::TagPatternInfo          = QStringLiteral("patternInfo");
const QString VAbstractPattern::TagPatternName          = QStringLiteral("patternName");
const QString VAbstractPattern::TagPatternNum           = QStringLiteral("patternNumber");
const QString VAbstractPattern::TagCustomerName         = QStringLiteral("customer");
const QString VAbstractPattern::TagCompanyName          = QStringLiteral("company");
const QString VAbstractPattern::TagPatternLabel         = QStringLiteral("patternLabel");
const QString VAbstractPattern::TagGrainline            = QStringLiteral("grainline");
const QString VAbstractPattern::TagPath                 = QStringLiteral("path");
const QString VAbstractPattern::TagNodes                = QStringLiteral("nodes");
const QString VAbstractPattern::TagNode                 = QStringLiteral("node");
const QString VAbstractPattern::TagLine                 = QStringLiteral("line");

const QString VAbstractPattern::AttrName                = QStringLiteral("name");
const QString VAbstractPattern::AttrVisible             = QStringLiteral("visible");
const QString VAbstractPattern::AttrGroupLocked         = QStringLiteral("locked");
const QString VAbstractPattern::AttrGroupColor          = QStringLiteral("groupColor");
const QString VAbstractPattern::AttrObject              = QStringLiteral("object");
const QString VAbstractPattern::AttrTool                = QStringLiteral("tool");
const QString VAbstractPattern::AttrType                = QStringLiteral("type");
const QString VAbstractPattern::AttrLetter              = QStringLiteral("letter");
const QString VAbstractPattern::AttrAnnotation          = QStringLiteral("annotation");
const QString VAbstractPattern::AttrOrientation         = QStringLiteral("orientation");
const QString VAbstractPattern::AttrRotationWay         = QStringLiteral("rotationWay");
const QString VAbstractPattern::AttrTilt                = QStringLiteral("tilt");
const QString VAbstractPattern::AttrFoldPosition        = QStringLiteral("foldPosition");
const QString VAbstractPattern::AttrQuantity            = QStringLiteral("quantity");
const QString VAbstractPattern::AttrOnFold              = QStringLiteral("onFold");
const QString VAbstractPattern::AttrDateFormat          = QStringLiteral("dateFormat");
const QString VAbstractPattern::AttrTimeFormat          = QStringLiteral("timeFormat");
const QString VAbstractPattern::AttrArrows              = QStringLiteral("arrows");
const QString VAbstractPattern::AttrNodeReverse         = QStringLiteral("reverse");
const QString VAbstractPattern::AttrNodeExcluded        = QStringLiteral("excluded");
const QString VAbstractPattern::AttrNodeIsNotch         = QStringLiteral("notch");
const QString VAbstractPattern::AttrNodeNotchType       = QStringLiteral("notchType");
const QString VAbstractPattern::AttrNodeNotchSubType    = QStringLiteral("notchSubtype");
const QString VAbstractPattern::AttrNodeShowNotch       = QStringLiteral("showNotch");
const QString VAbstractPattern::AttrNodeShowSecondNotch = QStringLiteral("showSecondNotch");
const QString VAbstractPattern::AttrNodeNotchLength     = QStringLiteral("notchLength");
const QString VAbstractPattern::AttrNodeNotchWidth      = QStringLiteral("notchWidth");
const QString VAbstractPattern::AttrNodeNotchAngle      = QStringLiteral("notchAngle");
const QString VAbstractPattern::AttrNodeNotchCount      = QStringLiteral("notchCount");
const QString VAbstractPattern::AttrSABefore            = QStringLiteral("before");
const QString VAbstractPattern::AttrSAAfter             = QStringLiteral("after");
const QString VAbstractPattern::AttrStart               = QStringLiteral("start");
const QString VAbstractPattern::AttrPath                = QStringLiteral("path");
const QString VAbstractPattern::AttrEnd                 = QStringLiteral("end");
const QString VAbstractPattern::AttrIncludeAs           = QStringLiteral("includeAs");
const QString VAbstractPattern::AttrWidth               = QStringLiteral("width");
const QString VAbstractPattern::AttrRotation            = QStringLiteral("rotation");

const QString VAbstractPattern::AttrAll                 = QStringLiteral("all");

const QString VAbstractPattern::AttrH50                 = QStringLiteral("h50");
const QString VAbstractPattern::AttrH56                 = QStringLiteral("h56");
const QString VAbstractPattern::AttrH62                 = QStringLiteral("h62");
const QString VAbstractPattern::AttrH68                 = QStringLiteral("h68");
const QString VAbstractPattern::AttrH74                 = QStringLiteral("h74");
const QString VAbstractPattern::AttrH80                 = QStringLiteral("h80");
const QString VAbstractPattern::AttrH86                 = QStringLiteral("h86");
const QString VAbstractPattern::AttrH92                 = QStringLiteral("h92");
const QString VAbstractPattern::AttrH98                 = QStringLiteral("h98");
const QString VAbstractPattern::AttrH104                = QStringLiteral("h104");
const QString VAbstractPattern::AttrH110                = QStringLiteral("h110");
const QString VAbstractPattern::AttrH116                = QStringLiteral("h116");
const QString VAbstractPattern::AttrH122                = QStringLiteral("h122");
const QString VAbstractPattern::AttrH128                = QStringLiteral("h128");
const QString VAbstractPattern::AttrH134                = QStringLiteral("h134");
const QString VAbstractPattern::AttrH140                = QStringLiteral("h140");
const QString VAbstractPattern::AttrH146                = QStringLiteral("h146");
const QString VAbstractPattern::AttrH152                = QStringLiteral("h152");
const QString VAbstractPattern::AttrH158                = QStringLiteral("h158");
const QString VAbstractPattern::AttrH164                = QStringLiteral("h164");
const QString VAbstractPattern::AttrH170                = QStringLiteral("h170");
const QString VAbstractPattern::AttrH176                = QStringLiteral("h176");
const QString VAbstractPattern::AttrH182                = QStringLiteral("h182");
const QString VAbstractPattern::AttrH188                = QStringLiteral("h188");
const QString VAbstractPattern::AttrH194                = QStringLiteral("h194");
const QString VAbstractPattern::AttrH200                = QStringLiteral("h200");

const QString VAbstractPattern::AttrS22                 = QStringLiteral("s22");
const QString VAbstractPattern::AttrS24                 = QStringLiteral("s24");
const QString VAbstractPattern::AttrS26                 = QStringLiteral("s26");
const QString VAbstractPattern::AttrS28                 = QStringLiteral("s28");
const QString VAbstractPattern::AttrS30                 = QStringLiteral("s30");
const QString VAbstractPattern::AttrS32                 = QStringLiteral("s32");
const QString VAbstractPattern::AttrS34                 = QStringLiteral("s34");
const QString VAbstractPattern::AttrS36                 = QStringLiteral("s36");
const QString VAbstractPattern::AttrS38                 = QStringLiteral("s38");
const QString VAbstractPattern::AttrS40                 = QStringLiteral("s40");
const QString VAbstractPattern::AttrS42                 = QStringLiteral("s42");
const QString VAbstractPattern::AttrS44                 = QStringLiteral("s44");
const QString VAbstractPattern::AttrS46                 = QStringLiteral("s46");
const QString VAbstractPattern::AttrS48                 = QStringLiteral("s48");
const QString VAbstractPattern::AttrS50                 = QStringLiteral("s50");
const QString VAbstractPattern::AttrS52                 = QStringLiteral("s52");
const QString VAbstractPattern::AttrS54                 = QStringLiteral("s54");
const QString VAbstractPattern::AttrS56                 = QStringLiteral("s56");
const QString VAbstractPattern::AttrS58                 = QStringLiteral("s58");
const QString VAbstractPattern::AttrS60                 = QStringLiteral("s60");
const QString VAbstractPattern::AttrS62                 = QStringLiteral("s62");
const QString VAbstractPattern::AttrS64                 = QStringLiteral("s64");
const QString VAbstractPattern::AttrS66                 = QStringLiteral("s66");
const QString VAbstractPattern::AttrS68                 = QStringLiteral("s68");
const QString VAbstractPattern::AttrS70                 = QStringLiteral("s70");
const QString VAbstractPattern::AttrS72                 = QStringLiteral("s72");

const QString VAbstractPattern::AttrCustom              = QStringLiteral("custom");
const QString VAbstractPattern::AttrDefHeight           = QStringLiteral("defHeight");
const QString VAbstractPattern::AttrDefSize             = QStringLiteral("defSize");
const QString VAbstractPattern::AttrExtension           = QStringLiteral("extension");

const QString VAbstractPattern::IncrementName           = QStringLiteral("name");
const QString VAbstractPattern::IncrementFormula        = QStringLiteral("formula");
const QString VAbstractPattern::IncrementDescription    = QStringLiteral("description");

const QString VAbstractPattern::NodeArc                 = QStringLiteral("NodeArc");
const QString VAbstractPattern::NodeElArc               = QStringLiteral("NodeElArc");
const QString VAbstractPattern::NodePoint               = QStringLiteral("NodePoint");
const QString VAbstractPattern::NodeSpline              = QStringLiteral("NodeSpline");
const QString VAbstractPattern::NodeSplinePath          = QStringLiteral("NodeSplinePath");

QHash<quint32, VDataTool*> VAbstractPattern::tools = QHash<quint32, VDataTool*>();
QVector<VLabelTemplateLine> VAbstractPattern::patternLabelLines = QVector<VLabelTemplateLine>();
bool VAbstractPattern::patternLabelWasChanged = false;

namespace
{
void ReadExpressionAttribute(QVector<VFormulaField> &expressions, const QDomElement &element, const QString &attribute)
{
    VFormulaField formula;
    try
    {
        formula.expression = VDomDocument::GetParametrString(element, attribute);
    }
    catch (VExceptionEmptyParameter &error)
    {
        Q_UNUSED(error)
        return;
    }

    formula.element = element;
    formula.attribute = attribute;

    expressions.append(formula);
}
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractPattern::VAbstractPattern(QObject *parent)
    : QObject(parent)
    , VDomDocument()
    , activeDraftBlock(QString())
    , m_DefaultLineColor(qApp->Settings()->getDefaultLineColor())
    , m_DefaultLineWeight(qApp->Settings()->getDefaultLineWeight())
    , m_DefaultLineType(qApp->Settings()->getDefaultLineType())
    , lastSavedExportFormat(QString())
    , cursor(0)
    , toolsOnRemove(QVector<VDataTool*>())
    , history(QVector<VToolRecord>())
    , patternPieces(QStringList())
    , modified(false)
{}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::ListMeasurements() const
{
    QSet<QString> measurements;
    QSet<QString> others;

    const QStringList increments = ListIncrements();
    for (int i=0; i < increments.size(); ++i)
    {
        others.insert(increments.at(i));
    }

    const QVector<VFormulaField> expressions = ListExpressions();
    for (int i=0; i < expressions.size(); ++i)
    {
        // Eval formula
        QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(expressions.at(i).expression, false, false));
        const QMap<int, QString> tokens = cal->GetTokens();// Tokens (variables, measurements)
        delete cal.take();

        const QList<QString> tValues = tokens.values();
        for (int j = 0; j < tValues.size(); ++j)
        {
            if (tValues.at(j) == QChar('-'))
            {
                continue;
            }

            if (measurements.contains(tValues.at(j)))
            {
                continue;
            }

            if (others.contains(tValues.at(j)))
            {
                continue;
            }

            if (IsVariable(tValues.at(j)) || IsPostfixOperator(tValues.at(j)) || IsFunction(tValues.at(j)))
            {
                others.insert(tValues.at(j));
            }
            else
            {
                measurements.insert(tValues.at(j));
            }
        }
    }

	return QStringList(measurements.values());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief changeActiveDraftBlock set new active draft block name.
 * @param name new name.
 * @param parse parser file mode.
 */
void VAbstractPattern::changeActiveDraftBlock(const QString &name, const Document &parse)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name draft block is empty");
    if (draftBlockNameExists(name))
    {
        this->activeDraftBlock = name;
        if (parse == Document::FullParse)
        {
            emit activeDraftBlockChanged(name);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getActiveDraftBlockName return current draft block name.
 * @return draft block name.
 */
QString VAbstractPattern::getActiveDraftBlockName() const
{
    return activeDraftBlock;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getActiveDraftElement return draftBlock element for current draft block.
 * @param element draftBlock element.
 * @return true if found.
 */
bool VAbstractPattern::getActiveDraftElement(QDomElement &element) const
{
    if (activeDraftBlock.isEmpty() == false)
    {
        const QDomNodeList elements = this->documentElement().elementsByTagName(TagDraftBlock);
        if (elements.size() == 0)
        {
            return false;
        }
        for ( qint32 i = 0; i < elements.count(); i++ )
        {
            element = elements.at( i ).toElement();
            if (element.isNull() == false)
            {
                const QString fieldName = element.attribute( AttrName );
                if ( fieldName == activeDraftBlock )
                {
                    return true;
                }
            }
        }
        element = QDomElement();
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief draftBlockNameExists check if draft block with this name exists.
 * @param name draft block name.
 * @return true if exist.
 */
bool VAbstractPattern::draftBlockNameExists(const QString &name) const
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "draft block name is empty");
    const QDomNodeList elements = this->documentElement().elementsByTagName(TagDraftBlock);
    if (elements.size() == 0)
    {
        return false;
    }
    for ( qint32 i = 0; i < elements.count(); i++ )
    {
        const QDomElement elem = elements.at( i ).toElement();
        if (elem.isNull() == false)
        {
            if ( GetParametrString(elem, AttrName) == name )
            {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getActiveNodeElement find element in current draft block by name.
 * @param name name tag.
 * @param element element.
 * @return true if found.
 */
bool VAbstractPattern::getActiveNodeElement(const QString &name, QDomElement &element) const
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "draft block name is empty");
    QDomElement draftBlockElement;
    if (getActiveDraftElement(draftBlockElement))
    {
        const QDomNodeList listElement = draftBlockElement.elementsByTagName(name);
        if (listElement.size() != 1)
        {
            return false;
        }
        element = listElement.at( 0 ).toElement();
        if (element.isNull() == false)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::parseGroups(const QDomElement &domElement)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    QMap<quint32, quint32> itemTool;
    QMap<quint32, bool> itemVisibility;

    QDomNode domNode = domElement.firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                if (domElement.tagName() == TagGroup)
                {
                    VContainer::UpdateId(GetParametrUInt(domElement, AttrId, NULL_ID_STR));

                    const QPair<bool, QMap<quint32, quint32> > groupData = parseItemElement(domElement);
                    const QMap<quint32, quint32> group = groupData.second;
                    auto i = group.constBegin();
                    while (i != group.constEnd())
                    {
                        if (not itemTool.contains(i.key()))
                        {
                            itemTool.insert(i.key(), i.value());
                        }

                        const bool previous = itemVisibility.value(i.key(), false);
                        itemVisibility.insert(i.key(), previous || groupData.first);
                        ++i;
                    }
                }
            }
        }
        domNode = domNode.nextSibling();
    }

    auto i = itemTool.constBegin();
    while (i != itemTool.constEnd())
    {
        if (tools.contains(i.value()))
        {
            VDataTool* tool = tools.value(i.value());
            tool->GroupVisibility(i.key(), itemVisibility.value(i.key(), true));
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractPattern::draftBlockCount() const
{
    const QDomElement rootElement = this->documentElement();
    if (rootElement.isNull())
    {
        return 0;
    }

    return rootElement.elementsByTagName(TagDraftBlock).count();
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::getDraftBlockElement(const QString &name)
{
    if (name.isEmpty() == false)
    {
        const QDomNodeList elements = this->documentElement().elementsByTagName(TagDraftBlock);
        if (elements.size() == 0)
        {
            return QDomElement();
        }
        for ( qint32 i = 0; i < elements.count(); i++ )
        {
            QDomElement element = elements.at( i ).toElement();
            if (element.isNull() == false)
            {
                if ( element.attribute( AttrName ) == name )
                {
                    return element;
                }
            }
        }
    }
    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief renameDraftBlock change draft block name.
 * @param oldName old draft block name.
 * @param newName new draft block name.
 * @return true if success.
 */
bool VAbstractPattern::renameDraftBlock(const QString &oldName, const QString &newName)
{
    Q_ASSERT_X(!newName.isEmpty(), Q_FUNC_INFO, "new name draft block is empty");
    Q_ASSERT_X(!oldName.isEmpty(), Q_FUNC_INFO, "old name draft block is empty");

    if (draftBlockNameExists(oldName) == false)
    {
        qDebug() << "Draft block does not exist with name" << oldName;
        return false;
    }

    if (draftBlockNameExists(newName))
    {
        qDebug() << "Draft block already exists with name" << newName;
        return false;
    }

    QDomElement ppElement = getDraftBlockElement(oldName);
    if (ppElement.isElement())
    {
        if (activeDraftBlock == oldName)
        {
            activeDraftBlock = newName;
        }
        ppElement.setAttribute(AttrName, newName);
        emit patternChanged(false);//For situation when we change name directly, without undocommands.
        emit draftBlockNameChanged(oldName, newName);
        return true;
    }
    else
    {
        qDebug() << "Can't find draft block node with name" << oldName << Q_FUNC_INFO;
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief appendDraftBlock add new draft block.
 *
 * Method check if not exist draft block with the same name and change name active draft block name, send signal
 * about change draft block. Doen't add draft block to file structure. This task make SPoint tool.
 * @param name draft block name.
 * @return true if success.
 */
bool VAbstractPattern::appendDraftBlock(const QString &name)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name draft block is empty");
    if (name.isEmpty())
    {
        return false;
    }
    if (draftBlockNameExists(name) == false)
    {
        setActiveDraftBlock(name);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VAbstractPattern::getCursor() const
{
    return cursor;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setCursor(const quint32 &value)
{
    if (cursor != value)
    {
        cursor = value;
        emit ChangedCursor(cursor);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setDefaultPen(Pen pen)
{
  m_DefaultLineColor  = pen.color;
  m_DefaultLineWeight = pen.lineWeight;
  m_DefaultLineType   = pen.lineType;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::getDefaultLineColor() const
{
  return m_DefaultLineColor;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VAbstractPattern::getDefaultLineWeight() const
{
  return m_DefaultLineWeight;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::getDefaultLineType() const
{
  return m_DefaultLineType;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getTool return tool from tool list.
 * @param id tool id.
 * @return tool.
 */
VDataTool *VAbstractPattern::getTool(quint32 id)
{
    ToolExists(id);
    return tools.value(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddTool add tool to list tools.
 * @param id tool id.
 * @param tool tool.
 */
void VAbstractPattern::AddTool(quint32 id, VDataTool *tool)
{
    Q_ASSERT_X(id != 0, Q_FUNC_INFO, "id == 0");
    SCASSERT(tool != nullptr)
            tools.insert(id, tool);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::RemoveTool(quint32 id)
{
    tools.remove(id);
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VAbstractPattern::ParsePieceNodes(const QDomElement &domElement)
{
    VPiecePath path;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            path.Append(ParseSANode(element));
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> VAbstractPattern::ParsePieceCSARecords(const QDomElement &domElement)
{
    QVector<CustomSARecord> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            CustomSARecord record;
            record.startPoint = GetParametrUInt(element, VAbstractPattern::AttrStart, NULL_ID_STR);
            record.path = GetParametrUInt(element, VAbstractPattern::AttrPath, NULL_ID_STR);
            record.endPoint = GetParametrUInt(element, VAbstractPattern::AttrEnd, NULL_ID_STR);
            record.reverse = getParameterBool(element, VAbstractPattern::AttrNodeReverse, falseStr);
            record.includeType = static_cast<PiecePathIncludeType>(GetParametrUInt(element,
                                                                                   VAbstractPattern::AttrIncludeAs,
                                                                                   "1"));
            records.append(record);
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VAbstractPattern::ParsePieceInternalPaths(const QDomElement &domElement)
{
    QVector<quint32> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            const quint32 path = GetParametrUInt(element, VAbstractPattern::AttrPath, NULL_ID_STR);
            if (path > NULL_ID)
            {
                records.append(path);
            }
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VAbstractPattern::ParsePieceAnchors(const QDomElement &domElement)
{
    QVector<quint32> records;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull())
        {
            const quint32 path = element.text().toUInt();
            if (path > NULL_ID)
            {
                records.append(path);
            }
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode VAbstractPattern::ParseSANode(const QDomElement &domElement)
{
    const quint32 id = VDomDocument::GetParametrUInt(domElement, AttrIdObject, NULL_ID_STR);
    const bool reverse = VDomDocument::GetParametrUInt(domElement, VAbstractPattern::AttrNodeReverse, "0");
    const bool excluded = VDomDocument::getParameterBool(domElement, VAbstractPattern::AttrNodeExcluded, falseStr);
    const QString saBefore = VDomDocument::GetParametrString(domElement, VAbstractPattern::AttrSABefore,
                                                             currentSeamAllowance);
    const QString saAfter = VDomDocument::GetParametrString(domElement, VAbstractPattern::AttrSAAfter,
                                                            currentSeamAllowance);
    const PieceNodeAngle angle = static_cast<PieceNodeAngle>(VDomDocument::GetParametrUInt(domElement, AttrAngle, "0"));

    const bool notch = VDomDocument::getParameterBool(domElement, VAbstractPattern::AttrNodeIsNotch, falseStr);
    const NotchType notchType = stringToNotchType(VDomDocument::GetParametrString(domElement,
                                                          VAbstractPattern::AttrNodeNotchType,strSlit));
    const NotchSubType notchSubType = stringToNotchSubType(VDomDocument::GetParametrString(domElement,
                                                          VAbstractPattern::AttrNodeNotchSubType, strStraightforward));

    const bool showNotch = VDomDocument::getParameterBool(domElement, VAbstractPattern::AttrNodeShowNotch,
                                                          trueStr);
    const bool showSecond = VDomDocument::getParameterBool(domElement, VAbstractPattern::AttrNodeShowSecondNotch,
                                                          trueStr);
    const qreal  notchLength = VDomDocument::GetParametrDouble(domElement, VAbstractPattern::AttrNodeNotchLength,
                                                               QString::number(qApp->Settings()->getDefaultNotchLength()));
    const qreal   notchWidth = VDomDocument::GetParametrDouble(domElement, VAbstractPattern::AttrNodeNotchWidth,
                                                               QString::number(qApp->Settings()->getDefaultNotchWidth()));
    const qreal   notchAngle = VDomDocument::GetParametrDouble(domElement, VAbstractPattern::AttrNodeNotchAngle, ".00");
    const quint32 notchCount = VDomDocument::GetParametrUInt(domElement,   VAbstractPattern::AttrNodeNotchCount, "1");


    const QString t = VDomDocument::GetParametrString(domElement, AttrType, VAbstractPattern::NodePoint);
    Tool tool;

    const QStringList types = QStringList() << VAbstractPattern::NodePoint
                                            << VAbstractPattern::NodeArc
                                            << VAbstractPattern::NodeSpline
                                            << VAbstractPattern::NodeSplinePath
                                            << VAbstractPattern::NodeElArc;

    switch (types.indexOf(t))
    {
        case 0: // VAbstractPattern::NodePoint
            tool = Tool::NodePoint;
            break;
        case 1: // VAbstractPattern::NodeArc
            tool = Tool::NodeArc;
            break;
        case 2: // VAbstractPattern::NodeSpline
            tool = Tool::NodeSpline;
            break;
        case 3: // VAbstractPattern::NodeSplinePath
            tool = Tool::NodeSplinePath;
            break;
        case 4: // NodeElArc
            tool = Tool::NodeElArc;
            break;
        default:
            VException e(QObject::tr("Wrong tag name '%1'.").arg(t));
            throw e;
    }
    VPieceNode node(id, tool, reverse);
    node.setBeforeSAFormula(saBefore);
    node.setAfterSAFormula(saAfter);
    node.SetAngleType(angle);
    node.SetExcluded(excluded);
    node.setNotch(notch);
    node.setNotchType(notchType);
    node.setNotchSubType(notchSubType);
    node.setShowNotch(showNotch);
    node.setShowSeamlineNotch(showSecond);
    node.setNotchLength(notchLength);
    node.setNotchWidth(notchWidth);
    node.setNotchAngle(notchAngle);
    node.setNotchCount(notchCount);
    return node;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::AddToolOnRemove(VDataTool *tool)
{
    SCASSERT(tool != nullptr)
    toolsOnRemove.append(tool);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getHistory return list with list of history records.
 * @return list of history records.
 */
QVector<VToolRecord> *VAbstractPattern::getHistory()
{
    return &history;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VToolRecord> VAbstractPattern::getBlockHistory() const
{
    QVector<VToolRecord> draftBlockHistory;
    for (qint32 i = 0; i< history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        if (tool.getDraftBlockName() != getActiveDraftBlockName())
        {
            continue;
        }
        draftBlockHistory.append(tool);
    }
    return draftBlockHistory;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, Tool> VAbstractPattern::getGroupObjHistory() const
{
    QMap<quint32, Tool> draftBlockHistory;
    for (qint32 i = 0; i< history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        if (tool.getDraftBlockName() != getActiveDraftBlockName())
        {
            continue;
        }
        draftBlockHistory.insert(tool.getId(), tool.getTypeTool());
    }
    return draftBlockHistory;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::MPath() const
{
    return UniqueTagText(TagMeasurements);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetMPath(const QString &path)
{
    if (setTagText(TagMeasurements, path))
    {
        emit patternChanged(false);
        patternLabelWasChanged = true;
    }
    else
    {
        qWarning() << "Can't save path to measurements" << Q_FUNC_INFO;
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VAbstractPattern::SiblingNodeId(const quint32 &nodeId) const
{
    quint32 siblingId = NULL_ID;

    const QVector<VToolRecord> history = getBlockHistory();
    for (qint32 i = 0; i < history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        if (nodeId == tool.getId())
        {
            if (i == 0)
            {
                siblingId = NULL_ID;
            }
            else
            {
                for (qint32 j = i; j > 0; --j)
                {
                    const VToolRecord tool = history.at(j-1);
                    switch ( tool.getTypeTool() )
                    {
                        case Tool::Piece:
                        case Tool::Union:
                        case Tool::NodeArc:
                        case Tool::NodeElArc:
                        case Tool::NodePoint:
                        case Tool::NodeSpline:
                        case Tool::NodeSplinePath:
                            continue;
                        default:
                            siblingId = tool.getId();
                            j = 0;// break loop
                            break;
                    }
                }
            }
        }
    }
    return siblingId;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::getPatternPieces() const
{
    return patternPieces;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<GHeights, bool> VAbstractPattern::GetGradationHeights() const
{
    QMap<GHeights, bool> map;

    map.insert(GHeights::ALL, true);
    map.insert(GHeights::H50, true);
    map.insert(GHeights::H56, true);
    map.insert(GHeights::H62, true);
    map.insert(GHeights::H68, true);
    map.insert(GHeights::H74, true);
    map.insert(GHeights::H80, true);
    map.insert(GHeights::H86, true);
    map.insert(GHeights::H92, true);
    map.insert(GHeights::H98, true);
    map.insert(GHeights::H104, true);
    map.insert(GHeights::H110, true);
    map.insert(GHeights::H116, true);
    map.insert(GHeights::H122, true);
    map.insert(GHeights::H128, true);
    map.insert(GHeights::H134, true);
    map.insert(GHeights::H140, true);
    map.insert(GHeights::H146, true);
    map.insert(GHeights::H152, true);
    map.insert(GHeights::H158, true);
    map.insert(GHeights::H164, true);
    map.insert(GHeights::H170, true);
    map.insert(GHeights::H176, true);
    map.insert(GHeights::H182, true);
    map.insert(GHeights::H188, true);
    map.insert(GHeights::H194, true);
    map.insert(GHeights::H200, true);

    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.size() == 0)
    {
        return map;
    }

    QStringList gTags = QStringList() << TagHeights << TagSizes;
    QDomNode domNode = tags.at(0).firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                const QString defValue = trueStr;
                switch (gTags.indexOf(domElement.tagName()))
                {
                    case 0: // TagHeights
                        if (getParameterBool(domElement, AttrAll, defValue))
                        {
                            return map;
                        }
                        else
                        {
                            map.insert(GHeights::ALL, false);
                        }

                        map.insert(GHeights::H50, getParameterBool(domElement, AttrH50, defValue));
                        map.insert(GHeights::H56, getParameterBool(domElement, AttrH56, defValue));
                        map.insert(GHeights::H62, getParameterBool(domElement, AttrH62, defValue));
                        map.insert(GHeights::H68, getParameterBool(domElement, AttrH68, defValue));
                        map.insert(GHeights::H74, getParameterBool(domElement, AttrH74, defValue));
                        map.insert(GHeights::H80, getParameterBool(domElement, AttrH80, defValue));
                        map.insert(GHeights::H86, getParameterBool(domElement, AttrH86, defValue));
                        map.insert(GHeights::H92, getParameterBool(domElement, AttrH92, defValue));
                        map.insert(GHeights::H98, getParameterBool(domElement, AttrH98, defValue));
                        map.insert(GHeights::H104, getParameterBool(domElement, AttrH104, defValue));
                        map.insert(GHeights::H110, getParameterBool(domElement, AttrH110, defValue));
                        map.insert(GHeights::H116, getParameterBool(domElement, AttrH116, defValue));
                        map.insert(GHeights::H122, getParameterBool(domElement, AttrH122, defValue));
                        map.insert(GHeights::H128, getParameterBool(domElement, AttrH128, defValue));
                        map.insert(GHeights::H134, getParameterBool(domElement, AttrH134, defValue));
                        map.insert(GHeights::H140, getParameterBool(domElement, AttrH140, defValue));
                        map.insert(GHeights::H146, getParameterBool(domElement, AttrH146, defValue));
                        map.insert(GHeights::H152, getParameterBool(domElement, AttrH152, defValue));
                        map.insert(GHeights::H158, getParameterBool(domElement, AttrH158, defValue));
                        map.insert(GHeights::H164, getParameterBool(domElement, AttrH164, defValue));
                        map.insert(GHeights::H170, getParameterBool(domElement, AttrH170, defValue));
                        map.insert(GHeights::H176, getParameterBool(domElement, AttrH176, defValue));
                        map.insert(GHeights::H182, getParameterBool(domElement, AttrH182, defValue));
                        map.insert(GHeights::H188, getParameterBool(domElement, AttrH188, defValue));
                        map.insert(GHeights::H194, getParameterBool(domElement, AttrH194, defValue));
                        map.insert(GHeights::H200, getParameterBool(domElement, AttrH200, defValue));
                        return map;
                    case 1: // TagSizes
                    default:
                        break;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetGradationHeights(const QMap<GHeights, bool> &options)
{
    CheckTagExists(TagGradation);
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.isEmpty())
    {
        qWarning() << "Can't save tag " << TagGradation << Q_FUNC_INFO;
        return;
    }

    QStringList gTags = QStringList() << TagHeights << TagSizes;
    QDomNode domNode = tags.at(0).firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                switch (gTags.indexOf(domElement.tagName()))
                {
                    case 0: // TagHeights
                        SetAttribute(domElement, AttrAll, options.value(GHeights::ALL));
                        if (options.value(GHeights::ALL))
                        {
                            domElement.removeAttribute(AttrH50);
                            domElement.removeAttribute(AttrH56);
                            domElement.removeAttribute(AttrH62);
                            domElement.removeAttribute(AttrH68);
                            domElement.removeAttribute(AttrH74);
                            domElement.removeAttribute(AttrH80);
                            domElement.removeAttribute(AttrH86);
                            domElement.removeAttribute(AttrH92);
                            domElement.removeAttribute(AttrH98);
                            domElement.removeAttribute(AttrH104);
                            domElement.removeAttribute(AttrH110);
                            domElement.removeAttribute(AttrH116);
                            domElement.removeAttribute(AttrH122);
                            domElement.removeAttribute(AttrH128);
                            domElement.removeAttribute(AttrH134);
                            domElement.removeAttribute(AttrH140);
                            domElement.removeAttribute(AttrH146);
                            domElement.removeAttribute(AttrH152);
                            domElement.removeAttribute(AttrH158);
                            domElement.removeAttribute(AttrH164);
                            domElement.removeAttribute(AttrH170);
                            domElement.removeAttribute(AttrH176);
                            domElement.removeAttribute(AttrH182);
                            domElement.removeAttribute(AttrH188);
                            domElement.removeAttribute(AttrH194);
                            domElement.removeAttribute(AttrH200);
                        }
                        else
                        {
                            SetAttribute(domElement, AttrH50, options.value(GHeights::H50));
                            SetAttribute(domElement, AttrH56, options.value(GHeights::H56));
                            SetAttribute(domElement, AttrH62, options.value(GHeights::H62));
                            SetAttribute(domElement, AttrH68, options.value(GHeights::H68));
                            SetAttribute(domElement, AttrH74, options.value(GHeights::H74));
                            SetAttribute(domElement, AttrH80, options.value(GHeights::H80));
                            SetAttribute(domElement, AttrH86, options.value(GHeights::H86));
                            SetAttribute(domElement, AttrH92, options.value(GHeights::H92));
                            SetAttribute(domElement, AttrH98, options.value(GHeights::H98));
                            SetAttribute(domElement, AttrH104, options.value(GHeights::H104));
                            SetAttribute(domElement, AttrH110, options.value(GHeights::H110));
                            SetAttribute(domElement, AttrH116, options.value(GHeights::H116));
                            SetAttribute(domElement, AttrH122, options.value(GHeights::H122));
                            SetAttribute(domElement, AttrH128, options.value(GHeights::H128));
                            SetAttribute(domElement, AttrH134, options.value(GHeights::H134));
                            SetAttribute(domElement, AttrH140, options.value(GHeights::H140));
                            SetAttribute(domElement, AttrH146, options.value(GHeights::H146));
                            SetAttribute(domElement, AttrH152, options.value(GHeights::H152));
                            SetAttribute(domElement, AttrH158, options.value(GHeights::H158));
                            SetAttribute(domElement, AttrH164, options.value(GHeights::H164));
                            SetAttribute(domElement, AttrH170, options.value(GHeights::H170));
                            SetAttribute(domElement, AttrH176, options.value(GHeights::H176));
                            SetAttribute(domElement, AttrH182, options.value(GHeights::H182));
                            SetAttribute(domElement, AttrH188, options.value(GHeights::H188));
                            SetAttribute(domElement, AttrH194, options.value(GHeights::H194));
                            SetAttribute(domElement, AttrH200, options.value(GHeights::H200));
                        }

                        modified = true;
                        emit patternChanged(false);
                        return;
                    case 1: // TagSizes
                    default:
                        break;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<GSizes, bool> VAbstractPattern::GetGradationSizes() const
{
    QMap<GSizes, bool> map;
    map.insert(GSizes::ALL, true);
    map.insert(GSizes::S22, true);
    map.insert(GSizes::S24, true);
    map.insert(GSizes::S26, true);
    map.insert(GSizes::S28, true);
    map.insert(GSizes::S30, true);
    map.insert(GSizes::S32, true);
    map.insert(GSizes::S34, true);
    map.insert(GSizes::S36, true);
    map.insert(GSizes::S38, true);
    map.insert(GSizes::S40, true);
    map.insert(GSizes::S42, true);
    map.insert(GSizes::S44, true);
    map.insert(GSizes::S46, true);
    map.insert(GSizes::S48, true);
    map.insert(GSizes::S50, true);
    map.insert(GSizes::S52, true);
    map.insert(GSizes::S54, true);
    map.insert(GSizes::S56, true);
    map.insert(GSizes::S58, true);
    map.insert(GSizes::S60, true);
    map.insert(GSizes::S62, true);
    map.insert(GSizes::S64, true);
    map.insert(GSizes::S66, true);
    map.insert(GSizes::S68, true);
    map.insert(GSizes::S70, true);
    map.insert(GSizes::S72, true);

    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.size() == 0)
    {
        return map;
    }

    QStringList gTags = QStringList() << TagHeights << TagSizes;
    QDomNode domNode = tags.at(0).firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                const QString defValue = trueStr;
                switch (gTags.indexOf(domElement.tagName()))
                {
                    case 1: // TagSizes
                        if (getParameterBool(domElement, AttrAll, defValue))
                        {
                            return map;
                        }
                        else
                        {
                            map.insert(GSizes::ALL, false);
                        }

                        map.insert(GSizes::S22, getParameterBool(domElement, AttrS22, defValue));
                        map.insert(GSizes::S24, getParameterBool(domElement, AttrS24, defValue));
                        map.insert(GSizes::S26, getParameterBool(domElement, AttrS26, defValue));
                        map.insert(GSizes::S28, getParameterBool(domElement, AttrS28, defValue));
                        map.insert(GSizes::S30, getParameterBool(domElement, AttrS30, defValue));
                        map.insert(GSizes::S32, getParameterBool(domElement, AttrS32, defValue));
                        map.insert(GSizes::S34, getParameterBool(domElement, AttrS34, defValue));
                        map.insert(GSizes::S36, getParameterBool(domElement, AttrS36, defValue));
                        map.insert(GSizes::S38, getParameterBool(domElement, AttrS38, defValue));
                        map.insert(GSizes::S40, getParameterBool(domElement, AttrS40, defValue));
                        map.insert(GSizes::S42, getParameterBool(domElement, AttrS42, defValue));
                        map.insert(GSizes::S44, getParameterBool(domElement, AttrS44, defValue));
                        map.insert(GSizes::S46, getParameterBool(domElement, AttrS46, defValue));
                        map.insert(GSizes::S48, getParameterBool(domElement, AttrS48, defValue));
                        map.insert(GSizes::S50, getParameterBool(domElement, AttrS50, defValue));
                        map.insert(GSizes::S52, getParameterBool(domElement, AttrS52, defValue));
                        map.insert(GSizes::S54, getParameterBool(domElement, AttrS54, defValue));
                        map.insert(GSizes::S56, getParameterBool(domElement, AttrS56, defValue));
                        map.insert(GSizes::S58, getParameterBool(domElement, AttrS58, defValue));
                        map.insert(GSizes::S60, getParameterBool(domElement, AttrS60, defValue));
                        map.insert(GSizes::S62, getParameterBool(domElement, AttrS62, defValue));
                        map.insert(GSizes::S64, getParameterBool(domElement, AttrS64, defValue));
                        map.insert(GSizes::S66, getParameterBool(domElement, AttrS66, defValue));
                        map.insert(GSizes::S68, getParameterBool(domElement, AttrS68, defValue));
                        map.insert(GSizes::S70, getParameterBool(domElement, AttrS70, defValue));
                        map.insert(GSizes::S72, getParameterBool(domElement, AttrS72, defValue));
                        return map;
                    case 0: // TagHeights
                    default:
                        break;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetGradationSizes(const QMap<GSizes, bool> &options)
{
    CheckTagExists(TagGradation);
    QDomNodeList tags = elementsByTagName(TagGradation);
    if (tags.isEmpty())
    {
        qWarning() << "Can't save tag " << TagGradation << Q_FUNC_INFO;
        return;
    }

    QStringList gTags = QStringList() << TagHeights << TagSizes;
    QDomNode domNode = tags.at(0).firstChild();
    while (domNode.isNull() == false)
    {
        if (domNode.isElement())
        {
            QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                switch (gTags.indexOf(domElement.tagName()))
                {
                    case 1: // TagSizes
                        SetAttribute(domElement, AttrAll, options.value(GSizes::ALL));
                        if (options.value(GSizes::ALL))
                        {
                            domElement.removeAttribute(AttrS22);
                            domElement.removeAttribute(AttrS24);
                            domElement.removeAttribute(AttrS26);
                            domElement.removeAttribute(AttrS28);
                            domElement.removeAttribute(AttrS30);
                            domElement.removeAttribute(AttrS32);
                            domElement.removeAttribute(AttrS34);
                            domElement.removeAttribute(AttrS36);
                            domElement.removeAttribute(AttrS38);
                            domElement.removeAttribute(AttrS40);
                            domElement.removeAttribute(AttrS42);
                            domElement.removeAttribute(AttrS44);
                            domElement.removeAttribute(AttrS46);
                            domElement.removeAttribute(AttrS48);
                            domElement.removeAttribute(AttrS50);
                            domElement.removeAttribute(AttrS52);
                            domElement.removeAttribute(AttrS54);
                            domElement.removeAttribute(AttrS56);
                            domElement.removeAttribute(AttrS58);
                            domElement.removeAttribute(AttrS60);
                            domElement.removeAttribute(AttrS62);
                            domElement.removeAttribute(AttrS64);
                            domElement.removeAttribute(AttrS66);
                            domElement.removeAttribute(AttrS68);
                            domElement.removeAttribute(AttrS70);
                            domElement.removeAttribute(AttrS72);
                        }
                        else
                        {
                            SetAttribute(domElement, AttrS22, options.value(GSizes::S22));
                            SetAttribute(domElement, AttrS24, options.value(GSizes::S24));
                            SetAttribute(domElement, AttrS26, options.value(GSizes::S26));
                            SetAttribute(domElement, AttrS28, options.value(GSizes::S28));
                            SetAttribute(domElement, AttrS30, options.value(GSizes::S30));
                            SetAttribute(domElement, AttrS32, options.value(GSizes::S32));
                            SetAttribute(domElement, AttrS34, options.value(GSizes::S34));
                            SetAttribute(domElement, AttrS36, options.value(GSizes::S36));
                            SetAttribute(domElement, AttrS38, options.value(GSizes::S38));
                            SetAttribute(domElement, AttrS40, options.value(GSizes::S40));
                            SetAttribute(domElement, AttrS42, options.value(GSizes::S42));
                            SetAttribute(domElement, AttrS44, options.value(GSizes::S44));
                            SetAttribute(domElement, AttrS46, options.value(GSizes::S46));
                            SetAttribute(domElement, AttrS48, options.value(GSizes::S48));
                            SetAttribute(domElement, AttrS50, options.value(GSizes::S50));
                            SetAttribute(domElement, AttrS52, options.value(GSizes::S52));
                            SetAttribute(domElement, AttrS54, options.value(GSizes::S54));
                            SetAttribute(domElement, AttrS56, options.value(GSizes::S56));
                            SetAttribute(domElement, AttrS58, options.value(GSizes::S58));
                            SetAttribute(domElement, AttrS60, options.value(GSizes::S60));
                            SetAttribute(domElement, AttrS62, options.value(GSizes::S62));
                            SetAttribute(domElement, AttrS64, options.value(GSizes::S64));
                            SetAttribute(domElement, AttrS66, options.value(GSizes::S66));
                            SetAttribute(domElement, AttrS68, options.value(GSizes::S68));
                            SetAttribute(domElement, AttrS70, options.value(GSizes::S70));
                            SetAttribute(domElement, AttrS72, options.value(GSizes::S72));
                        }

                        modified = true;
                        emit patternChanged(false);
                        return;
                    case 0: // TagHeights
                    default:
                        break;
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetDescription() const
{
    return UniqueTagText(TagDescription);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetDescription(const QString &text)
{
    CheckTagExists(TagDescription);
    setTagText(TagDescription, text);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetNotes() const
{
    return UniqueTagText(TagNotes);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetNotes(const QString &text)
{
    CheckTagExists(TagNotes);
    setTagText(TagNotes, text);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetPatternName() const
{
    return UniqueTagText(TagPatternName);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternName(const QString &qsName)
{
    CheckTagExists(TagPatternName);
    setTagText(TagPatternName, qsName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetCompanyName() const
{
    return UniqueTagText(TagCompanyName);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCompanyName(const QString &qsName)
{
    CheckTagExists(TagCompanyName);
    setTagText(TagCompanyName, qsName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetPatternNumber() const
{
    return UniqueTagText(TagPatternNum);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternNumber(const QString &qsNum)
{
    CheckTagExists(TagPatternNum);
    setTagText(TagPatternNum, qsNum);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetCustomerName() const
{
    return UniqueTagText(TagCustomerName);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetCustomerName(const QString &qsName)
{
    CheckTagExists(TagCustomerName);
    setTagText(TagCustomerName, qsName);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetLabelDateFormat() const
{
    QString globalLabelDateFormat = qApp->Settings()->GetLabelDateFormat();

    const QDomNodeList list = elementsByTagName(TagPatternLabel);
    if (list.isEmpty())
    {
        return globalLabelDateFormat;
    }

    QDomElement tag = list.at(0).toElement();
    return GetParametrString(tag, AttrDateFormat, globalLabelDateFormat);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetLabelDateFormat(const QString &format)
{
    QDomElement tag = CheckTagExists(TagPatternLabel);
    SetAttribute(tag, AttrDateFormat, format);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetLabelTimeFormat() const
{
    QString globalLabelTimeFormat = qApp->Settings()->GetLabelTimeFormat();

    const QDomNodeList list = elementsByTagName(TagPatternLabel);
    if (list.isEmpty())
    {
        return globalLabelTimeFormat;
    }

    QDomElement tag = list.at(0).toElement();
    return GetParametrString(tag, AttrTimeFormat, globalLabelTimeFormat);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetLabelTimeFormat(const QString &format)
{
    QDomElement tag = CheckTagExists(TagPatternLabel);
    SetAttribute(tag, AttrTimeFormat, format);
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setPatternLabelTemplate(const QVector<VLabelTemplateLine> &lines)
{
    QDomElement tag = CheckTagExists(TagPatternLabel);
    RemoveAllChildren(tag);
    SetLabelTemplate(tag, lines);
    patternLabelLines = lines;
    patternLabelWasChanged = true;
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VLabelTemplateLine> VAbstractPattern::getPatternLabelTemplate() const
{
    if (patternLabelLines.isEmpty())
    {
        const QDomNodeList list = elementsByTagName(TagPatternLabel);
        if (list.isEmpty())
        {
            return QVector<VLabelTemplateLine>();
        }

        patternLabelLines = GetLabelTemplate(list.at(0).toElement());
    }

    return patternLabelLines;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetPatternWasChanged(bool changed)
{
    patternLabelWasChanged = changed;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::GetPatternWasChanged() const
{
    return patternLabelWasChanged;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetImage() const
{
    return UniqueTagText(TagImage);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetImageExtension() const
{
    const QString defExt =  QStringLiteral("PNG");
    const QDomNodeList nodeList = this->elementsByTagName(TagImage);
    if (nodeList.isEmpty())
    {
        return defExt;
    }
    else
    {
        const QDomNode domNode = nodeList.at(0);
        if (domNode.isNull() == false && domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                const QString ext = domElement.attribute(AttrExtension, defExt);
                return ext;
            }
        }
    }
    return defExt;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetImage(const QString &text, const QString &extension)
{
    QDomElement imageElement = CheckTagExists(TagImage);
    setTagText(imageElement, text);
    CheckTagExists(TagImage).setAttribute(AttrExtension, extension);
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::DeleteImage()
{
    QDomElement pattern = documentElement();
    pattern.removeChild(CheckTagExists(TagImage));
    modified = true;
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::GetVersion() const
{
    return UniqueTagText(TagVersion, VPatternConverter::PatternMaxVerStr);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setVersion()
{
    setTagText(TagVersion, VPatternConverter::PatternMaxVerStr);
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getOpItems get vector of operation tool obects.
 * @param toolId operation tool id.
 * @param itemType  type of item - either source or destination.
 * @return vector of item element object ids.
 */
QVector<quint32> VAbstractPattern::getOpItems(const quint32 &toolId, const QString &itemType)
{
    QVector<quint32> items;
    quint32 objId;
    const QDomElement domElement = elementById(toolId);
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement dataElement = nodeList.at(i).toElement();
        if (!dataElement.isNull() && dataElement.tagName() == itemType)
        {
            const QDomNodeList srcList = dataElement.childNodes();
            for (qint32 j = 0; j < srcList.size(); ++j)
            {
                const QDomElement element = srcList.at(j).toElement();
                if (!element.isNull())
                {
                    objId = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR);
                    items.append(objId);
                }
            }
        }
    }

    return items;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VAbstractPattern::getDartItems(const quint32 &toolId)
{
    QVector<quint32> items;
    quint32 objId;
    const QDomElement domElement = elementById(toolId);

    objId = VDomDocument::GetParametrUInt(domElement, AttrPoint1, NULL_ID_STR);
    items.append(objId);

    objId = VDomDocument::GetParametrUInt(domElement, AttrPoint2, NULL_ID_STR);
    items.append(objId);

    return items;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief haveLiteChange we have unsaved change.
 */
void VAbstractPattern::haveLiteChange()
{
    emit patternChanged(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::NeedFullParsing()
{
    emit UndoCommand();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::ClearScene()
{
    emit ClearMainWindow();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::updatePieceList(quint32 id)
{
    emit UpdateInLayoutList(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::selectedPiece(quint32 id)
{
    emit showPiece(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::ToolExists(const quint32 &id)
{
    if (tools.contains(id) == false)
    {
        throw VExceptionBadId(tr("Can't find tool in table."), id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VAbstractPattern::ParsePathNodes(const QDomElement &domElement)
{
    VPiecePath path;
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull() && element.tagName() == VAbstractPattern::TagNode)
        {
            path.Append(ParseSANode(element));
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setActiveDraftBlock set current draft block.
 * @param name draft block name.
 */
void VAbstractPattern::setActiveDraftBlock(const QString &name)
{
    Q_ASSERT_X(not name.isEmpty(), Q_FUNC_INFO, "name draft block is empty");
    this->activeDraftBlock = name;
    emit activeDraftBlockChanged(name);
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::CheckTagExists(const QString &tag)
{
    const QDomNodeList list = elementsByTagName(tag);
    QDomElement element;
    if (list.isEmpty())
    {
        const QStringList tags = QStringList() << TagUnit << TagImage << TagDescription << TagNotes
                                         << TagGradation << TagPatternName << TagPatternNum << TagCompanyName
                                         << TagCustomerName << TagPatternLabel;
        switch (tags.indexOf(tag))
        {
            case 1: //TagImage
                element = createElement(TagImage);
                break;
            case 2: //TagDescription
                element = createElement(TagDescription);
                break;
            case 3: //TagNotes
                element = createElement(TagNotes);
                break;
            case 4: //TagGradation
            {
                element = createElement(TagGradation);

                QDomElement heights = createElement(TagHeights);
                heights.setAttribute(AttrAll, QLatin1String("true"));
                element.appendChild(heights);

                QDomElement sizes = createElement(TagSizes);
                sizes.setAttribute(AttrAll, QLatin1String("true"));
                element.appendChild(sizes);
                break;
            }
            case 5: // TagPatternName
                element = createElement(TagPatternName);
                break;
            case 6: // TagPatternNum
                element = createElement(TagPatternNum);
                break;
            case 7: // TagCompanyName
                element = createElement(TagCompanyName);
                break;
            case 8: // TagCustomerName
                element = createElement(TagCustomerName);
                break;
            case 9: // TagPatternLabel
                element = createElement(TagPatternLabel);
                break;
            case 0: //TagUnit (Mandatory tag)
            default:
                return QDomElement();
        }
        InsertTag(tags, element);
        return element;
    }
    return list.at(0).toElement();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::InsertTag(const QStringList &tags, const QDomElement &element)
{
    QDomElement pattern = documentElement();
    for (int i = tags.indexOf(element.tagName())-1; i >= 0; --i)
    {
        const QDomNodeList list = elementsByTagName(tags.at(i));
        if (not list.isEmpty())
        {
            pattern.insertAfter(element, list.at(0));
            break;
        }
    }
    setVersion();
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractPattern::getActiveDraftBlockIndex() const
{
    const QDomNodeList blockList = elementsByTagName(TagDraftBlock);

    int index = 0;
    if (not blockList.isEmpty())
    {
        for (int i = 0; i < blockList.size(); ++i)
        {
            QDomElement node = blockList.at(i).toElement();
            if (node.attribute(AttrName) == activeDraftBlock)
            {
                index = i;
                break;
            }
        }
    }

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstractPattern::ListIncrements() const
{
    QStringList increments;
    const QDomNodeList list = elementsByTagName(TagIncrement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        try
        {
            increments.append(GetParametrString(dom, IncrementName));
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }
    }

    return increments;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListExpressions() const
{
    QVector<VFormulaField> list;

    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    // Note. Tool Union Details also contains formulas, but we don't use them for union and keep only to simplifying
    // working with nodes. Same code for saving reading.
    list << ListPointExpressions();
    list << ListArcExpressions();
    list << ListElArcExpressions();
    list << ListSplineExpressions();
    list << ListIncrementExpressions();
    list << ListOperationExpressions();
    list << ListPathExpressions();
    list << ListPieceExpressions();

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPointExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment a number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagPoint);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrLength);
        ReadExpressionAttribute(expressions, dom, AttrAngle);
        ReadExpressionAttribute(expressions, dom, AttrC1Radius);
        ReadExpressionAttribute(expressions, dom, AttrC2Radius);
        ReadExpressionAttribute(expressions, dom, AttrCRadius);
        ReadExpressionAttribute(expressions, dom, AttrRadius);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListArcExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagArc);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrAngle1);
        ReadExpressionAttribute(expressions, dom, AttrAngle2);
        ReadExpressionAttribute(expressions, dom, AttrRadius);
        ReadExpressionAttribute(expressions, dom, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListElArcExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagElArc);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrRadius1);
        ReadExpressionAttribute(expressions, dom, AttrRadius2);
        ReadExpressionAttribute(expressions, dom, AttrAngle1);
        ReadExpressionAttribute(expressions, dom, AttrAngle2);
        ReadExpressionAttribute(expressions, dom, AttrRotationAngle);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListSplineExpressions() const
{
    QVector<VFormulaField> expressions;
    expressions << ListPathPointExpressions();
    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPathPointExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(AttrPathPoint);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrKAsm1);
        ReadExpressionAttribute(expressions, dom, AttrKAsm2);
        ReadExpressionAttribute(expressions, dom, AttrAngle);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListIncrementExpressions() const
{
    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagIncrement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        ReadExpressionAttribute(expressions, dom, IncrementFormula);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListOperationExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagOperation);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrAngle);
        ReadExpressionAttribute(expressions, dom, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListNodesExpressions(const QDomElement &nodes) const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;

    const QDomNodeList nodeList = nodes.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement element = nodeList.at(i).toElement();
        if (not element.isNull() && element.tagName() == VAbstractPattern::TagNode)
        {
            ReadExpressionAttribute(expressions, element, VAbstractPattern::AttrSABefore);
            ReadExpressionAttribute(expressions, element, VAbstractPattern::AttrSAAfter);
        }
    }
    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPathExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagPath);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        if (dom.isNull())
        {
            continue;
        }

        expressions << ListNodesExpressions(dom.firstChildElement(TagNodes));
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListGrainlineExpressions(const QDomElement &element) const
{
    QVector<VFormulaField> expressions;
    if (not element.isNull())
    {
        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, element, AttrRotation);
        ReadExpressionAttribute(expressions, element, AttrLength);
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VFormulaField> VAbstractPattern::ListPieceExpressions() const
{
    // Check if new tool doesn't bring new attribute with a formula.
    // If no just increment number.
    // If new tool bring absolutely new type and has formula(s) create new method to cover it.
    Q_STATIC_ASSERT(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53);

    QVector<VFormulaField> expressions;
    const QDomNodeList list = elementsByTagName(TagPiece);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();
        if (dom.isNull())
        {
            continue;
        }

        // Each tag can contains several attributes.
        ReadExpressionAttribute(expressions, dom, AttrWidth);

        expressions << ListNodesExpressions(dom.firstChildElement(TagNodes));
        expressions << ListGrainlineExpressions(dom.firstChildElement(TagGrainline));
    }

    return expressions;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::IsVariable(const QString &token) const
{
    for (int i = 0; i < builInVariables.size(); ++i)
    {
        if (token.indexOf( builInVariables.at(i) ) == 0)
        {
            if (builInVariables.at(i) == currentLength || builInVariables.at(i) == currentSeamAllowance)
            {
                return token == builInVariables.at(i);
            }
            else
            {
                return true;
            }
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::IsPostfixOperator(const QString &token) const
{
    for (int i = 0; i < builInPostfixOperators.size(); ++i)
    {
        if (token.indexOf( builInPostfixOperators.at(i) ) == 0)
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::IsFunction(const QString &token) const
{
    for (int i = 0; i < builInFunctions.size(); ++i)
    {
        if (token.indexOf( builInFunctions.at(i) ) == 0)
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
QPair<bool, QMap<quint32, quint32> > VAbstractPattern::parseItemElement(const QDomElement &domElement)
{
    Q_ASSERT_X(not domElement.isNull(), Q_FUNC_INFO, "domElement is null");

    try
    {
        const bool visible = getParameterBool(domElement, AttrVisible, trueStr);

        QMap<quint32, quint32> items;

        const QDomNodeList nodeList = domElement.childNodes();
        const qint32 num = nodeList.size();
        for (qint32 i = 0; i < num; ++i)
        {
            const QDomElement element = nodeList.at(i).toElement();
            if (not element.isNull() && element.tagName() == TagGroupItem)
            {
                const quint32 object = GetParametrUInt(element, AttrObject, NULL_ID_STR);
                const quint32 tool = GetParametrUInt(element, AttrTool, NULL_ID_STR);
                items.insert(object, tool);
            }
        }

        QPair<bool, QMap<quint32, quint32> > group;
        group.first = visible;
        group.second = items;

        return group;
    }
    catch (const VExceptionBadId &error)
    {
        VExceptionObjectError excep(tr("Error creating or updating group"), domElement);
        excep.AddMoreInformation(error.ErrorMessage());
        throw excep;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief IsModified state of the document for cases that do not cover QUndoStack.
 * @return true if the document was modified without using QUndoStack.
 */
bool VAbstractPattern::IsModified() const
{
    return modified;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::SetModified(bool modified)
{
    this->modified = modified;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::createGroups()
{
    QDomElement draftBlock;
    if (getActiveDraftElement(draftBlock))
    {
        QDomElement groups = draftBlock.firstChildElement(TagGroups);

        if (groups.isNull())
        {
            groups = createElement(TagGroups);
            draftBlock.appendChild(groups);
        }

        return groups;
    }
    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement VAbstractPattern::createGroup(quint32 groupId, const QString &name, const QString &color, const QString &type,
                                          const QString &weight, const QMap<quint32, quint32> &groupData)
{
    if (groupId == NULL_ID)
    {
        return QDomElement();
    }

    //Create new empty group
    QDomElement group = createElement(TagGroup);
    SetAttribute(group, AttrId, groupId);
    SetAttribute(group, AttrName, name);
    SetAttribute(group, AttrVisible, true);
    SetAttribute(group, AttrGroupLocked, false);
    SetAttribute(group, AttrGroupColor, color);
    SetAttribute(group, AttrLineType, type);
    SetAttribute(group, AttrLineWeight, weight);

    //Add objects to group
    if (!groupData.isEmpty())
    {
        auto i = groupData.constBegin();
        while (i != groupData.constEnd())
        {
            QDomElement item = createElement(TagGroupItem);
            item.setAttribute(AttrObject, i.key());
            item.setAttribute(AttrTool, i.value());
            group.appendChild(item);
            ++i;
        }
        return group;
    }
    else
    {
    return group;
    }
}

QDomElement VAbstractPattern::addGroupItems(const QString &name, const QMap<quint32, quint32> &groupData)
{
    quint32 groupId = getGroupIdByName(name);
    const bool locked = getGroupLock(groupId);
    if (locked)
    {
      return QDomElement();
    }

    QDomElement group = getGroupByName(name);

    if (! getParameterBool(group, AttrGroupLocked, trueStr))
    {
        if (!groupData.isEmpty())
        {
            auto i = groupData.constBegin();
            while (i != groupData.constEnd())
            {
                QDomElement item = createElement(TagGroupItem);
                item.setAttribute(AttrObject, i.key());
                item.setAttribute(AttrTool, i.value());
                group.appendChild(item);
                ++i;
            }
            modified = true;
            emit patternChanged(false);

            emit updateGroups();
        }
    }
    return group;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractPattern::getGroupName(quint32 id)
{
    QString name = tr("New group");
    QDomElement groups = createGroups();
    if (not groups.isNull())
    {
        QDomElement group = elementById(id, TagGroup);
        if (group.isElement())
        {
            name = GetParametrString(group, AttrName, name);
            return name;
        }
        else
        {
            if (groups.childNodes().isEmpty())
            {
                QDomNode parent = groups.parentNode();
                parent.removeChild(groups);
            }

            qDebug("Can't get group by id = %u.", id);
            return name;
        }
    }
    else
    {
        qDebug("Can't get tag Groups.");
        return name;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupName(quint32 id, const QString &name)
{
    QDomElement groups = createGroups();
    if (not groups.isNull())
    {
        QDomElement group = elementById(id, TagGroup);
        if (group.isElement())
        {
            group.setAttribute(AttrName, name);
            modified = true;
            emit patternChanged(false);
        }
        else
        {
            if (groups.childNodes().isEmpty())
            {
                QDomNode parent = groups.parentNode();
                parent.removeChild(groups);
            }

            qDebug("Can't get group by id = %u.", id);
        }
    }
    else
    {
        qDebug("Can't get tag Groups.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, GroupAttributes> VAbstractPattern::getGroups()
{
    GroupAttributes groupData;
    QMap<quint32, GroupAttributes> data;

    try
    {
        QDomElement groups = createGroups();
        if (not groups.isNull())
        {
            QDomNode domNode = groups.firstChild();
            while (domNode.isNull() == false)
            {
                if (domNode.isElement())
                {
                    const QDomElement group = domNode.toElement();
                    if (group.isNull() == false)
                    {
                        if (group.tagName() == TagGroup)
                        {
                            const quint32 id = GetParametrUInt(group, AttrId, "0");
                            const QString name = GetParametrString(group, AttrName, tr("New group 2"));
                            const bool visible = getParameterBool(group, AttrVisible, trueStr);
                            const bool locked = getParameterBool(group, AttrGroupLocked, trueStr);
                            const QString color = GetParametrString(group, AttrGroupColor, ColorBlack);
                            const QString linetype = GetParametrString(group, AttrLineType, LineTypeSolidLine);
                            const QString lineweight = GetParametrString(group, AttrLineWeight, "0.35");

                            groupData.name = name;
                            groupData.visible = visible;
                            groupData.locked = locked;
                            groupData.color = color;
                            groupData.linetype = linetype;
                            groupData.lineweight = lineweight;
                            data.insert(id, groupData);
                        }
                    }
                }
                domNode = domNode.nextSibling();
            }
            emit patternHasGroups(true);
        }
        else
        {
            emit patternHasGroups(false);
            qDebug("Can't get tag Groups.");
        }
    }
    catch (const VExceptionConversionError &)
    {
        return QMap<quint32, GroupAttributes>();
    }

    return data;
}


/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets List of Groups for the current Draft Block
 *
 * @return  AttrName - Group Name.
 * @throw   VExceptionConversionError if group error.
 */

QStringList VAbstractPattern::groupListByName()
{
    QStringList groupList;
    try
    {
        QDomElement groups = createGroups();
        if (not groups.isNull())
        {
            QDomNode domNode = groups.firstChild();
            if (domNode.isNull() == false)
            {
                while (domNode.isNull() == false)
                {
                    if (domNode.isElement())
                    {
                        const QDomElement group = domNode.toElement();
                        if (group.isNull() == false)
                        {
                            if (group.tagName() == TagGroup)
                            {

                                groupList <<  GetParametrString(group, AttrName);
                            }
                        }
                    }
                    domNode = domNode.nextSibling();
                }
                emit patternHasGroups(true);
            }
            else
            {
                emit patternHasGroups(false);
                qDebug("Can't get tag Groups.");
            }
        }

    }
    catch (const VExceptionConversionError &)
    {
        return groupList;
    }

    return groupList;
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets Dom Element for a given group name.
 *
 * @return  group - group Dom Element.
 */
QDomElement VAbstractPattern::getGroupByName(const QString &name)
{

    QDomElement groups = createGroups();
    if (not groups.isNull())
    {
        QDomNode domNode = groups.firstChild();
        if (domNode.isNull() == false)
        {
            while (domNode.isNull() == false)
            {
                if (domNode.isElement())
                {
                    const QDomElement group = domNode.toElement();
                    if (group.isNull() == false)
                    {
                        if (group.tagName() == TagGroup)
                        {
                            const QString groupName = GetParametrString(group, AttrName);
                            if (groupName == name)
                            {
                                return group;
                            }
                        }
                    }
                }
                domNode = domNode.nextSibling();
            }
            emit patternHasGroups(true);
        }
        else
        {
            emit patternHasGroups(false);
            qDebug("Can't get tag Groups.");
        }
    }
    return groups;
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets groupId for a given group name.
 *
 * @return  groupId - group Id.
 */
quint32 VAbstractPattern::getGroupIdByName(const QString &name)
{

    QDomElement groups = createGroups();
    if (not groups.isNull())
    {
        QDomNode domNode = groups.firstChild();
        if (domNode.isNull() == false)
        {
            while (domNode.isNull() == false)
            {
                if (domNode.isElement())
                {
                    const QDomElement group = domNode.toElement();
                    if (group.isNull() == false)
                    {
                        if (group.tagName() == TagGroup)
                        {
                            const QString groupName = GetParametrString(group, AttrName);
                            if (groupName == name)
                            {
                                const quint32 groupId = GetParametrUInt(group, AttrId, "0");
                                return groupId;
                            }
                        }
                    }
                }
                domNode = domNode.nextSibling();
            }
        }
        else
        {
            qDebug("Can't get tag Groups.");
        }
    }
    return quint32();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the groups that contain or do not contain the item identified by the toolid and the objectid
 * @param toolId
 * @param objectId
 * @param containsItem |true if the groups contain the given item, false if they don't contain the item
 * @return
 */
QMap<quint32, QString> VAbstractPattern::getGroupsContainingItem(quint32 toolId, quint32 objectId, bool containsItem)
{
    QMap<quint32, QString> data;

    if(objectId == 0)
    {
        objectId = toolId;
    }

    QDomElement groups = createGroups();
    if (not groups.isNull())
    {
        QDomNode domNode = groups.firstChild();
        while (domNode.isNull() == false) // iterate through the groups
        {
            if (domNode.isElement())
            {
                const QDomElement group = domNode.toElement();
                if (group.isNull() == false)
                {
                    if (group.tagName() == TagGroup)
                    {
                        bool groupHasItem = hasGroupItem(group, toolId, objectId);
                        if((containsItem && groupHasItem) || (not containsItem && not groupHasItem))
                        {
                            const quint32 groupId = GetParametrUInt(group, AttrId, "0");
                            const QString name = GetParametrString(group, AttrName, tr("New group"));
                            data.insert(groupId, name);
                        }
                    }
                }
            }
            domNode = domNode.nextSibling();
        }
    }
    else
    {
        qDebug("Can't get tag Groups.");
    }

    return data;
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Checks if the given group has the item with the given toolId and objectId
 * @param groupDomElement
 * @param toolId
 * @param objectId
 * @return
 */
bool VAbstractPattern::hasGroupItem(const QDomElement &groupDomElement, quint32 toolId, quint32 objectId)
{
    bool result = false;

    QDomNode itemNode = groupDomElement.firstChild();
    while (itemNode.isNull() == false) // iterate through the items of the group
    {
        if (itemNode.isElement())
        {
            const QDomElement item = itemNode.toElement();
            if (item.isNull() == false)
            {
                quint32 toolIdIterate= GetParametrUInt(item, AttrTool, "0");
                quint32 objectIdIterate= GetParametrUInt(item, AttrObject, "0");

                if(toolIdIterate == toolId && objectIdIterate == objectId)
                {
                    result = true;
                    break;
                }
            }
        }
        itemNode = itemNode.nextSibling();
    }
    return result;
}

/**
 * @brief Deletes an item from the group containing the toolId
 * @param toolId
 */
void VAbstractPattern::deleteToolFromGroup(quint32 toolId)
{
    QMap<quint32,QString> groupsContainingItem = getGroupsContainingItem(toolId, 0, true);
    QStringList list = QStringList(groupsContainingItem.values());
    QString  listGroupName = list.value(0);
    quint32 groupId = groupsContainingItem.key(listGroupName);

    const bool locked = getGroupLock(groupId);
    if (locked)
    {
      return;
    }

    QDomElement group = removeGroupItem(toolId, 0, groupId);
}

/**
 * @brief Adds an item to the given group with the given toolId and objectId
 * @param toolId
 */
void VAbstractPattern::addToolToGroup(quint32 toolId, quint32 objectId, const QString &groupName)
{
    quint32 groupId = getGroupIdByName(groupName);
    const bool locked = getGroupLock(groupId);
    if (locked)
    {
        return;
    }

    //Delete tool if contained in an existing group
    QMap<quint32,QString> groupsContainingItem = getGroupsContainingItem(toolId, objectId, true);
    QStringList list = QStringList(groupsContainingItem.values());
    QString  listGroupName = list.value(0);
    groupId = groupsContainingItem.key(listGroupName);

    QDomElement group = removeGroupItem(toolId, objectId, groupId);

    //Add to new Group.
    QMap<quint32,QString> groupsNotContainingItem = getGroupsContainingItem(toolId, objectId, false);
    list = QStringList(groupsNotContainingItem.values());

    for(int i=0; i<list.count(); ++i)
    {
        const QString  listGroupName = list.value(i);
        if (groupName == listGroupName)
        {
            const quint32 groupId = groupsNotContainingItem.key(list[i]);
            QDomElement group = addGroupItem(toolId, objectId, groupId);
            modified = true;
            emit patternChanged(false);
            emit updateGroups();
            QDomElement groups = createGroups();
            if (not groups.isNull())
            {
                parseGroups(groups);
            }
            break;
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Adds an item to the given group with the given toolId and objectId
 * @param toolId
 * @param objectId
 * @param groupId
 */
QDomElement VAbstractPattern::addGroupItem(quint32 toolId, quint32 objectId, quint32 groupId)
{

    const bool locked = getGroupLock(groupId);
    if (locked)
    {
      return QDomElement();
    }

    QDomElement group = elementById(groupId, TagGroup);

    if (!group.isNull())
    {
        if(objectId == 0)
        {
            objectId = toolId;
        }

        QDomElement item = createElement(TagGroupItem);
        item.setAttribute(AttrTool, toolId);
        item.setAttribute(AttrObject, objectId);
        group.appendChild(item);

        modified = true;
        emit patternChanged(false);

        emit updateGroups();

        QDomElement groups = createGroups();
        if (not groups.isNull())
        {
            parseGroups(groups);
        }

        return item;
    }
    else
    {
        qDebug() << "The group of id " << groupId << " doesn't exist";
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Removes the item of given toolId and objectId from the group of given groupId
 * @param toolId
 * @param objectId
 * @param groupId
 */
QDomElement VAbstractPattern::removeGroupItem(quint32 toolId, quint32 objectId, quint32 groupId)
{
    QDomElement group = elementById(groupId, TagGroup);

    const bool locked = getGroupLock(groupId);
    if (locked)
    {
      return QDomElement();
    }

    if (!group.isNull())
    {
        if(objectId == 0)
        {
            objectId = toolId;
        }

        QDomNode itemNode = group.firstChild();
        while (itemNode.isNull() == false) // iterate through the items of the group
        {
            if (itemNode.isElement())
            {
                const QDomElement item = itemNode.toElement();
                if (item.isNull() == false)
                {
                    quint32 toolIdIterate= GetParametrUInt(item, AttrTool, "0");
                    quint32 objectIdIterate= GetParametrUInt(item, AttrObject, "0");

                    if(toolIdIterate == toolId && objectIdIterate == objectId)
                    {
                        group.removeChild(itemNode);

                        // to signalised that the pattern was changed and need to be saved
                        modified = true;
                        emit patternChanged(false);

                        // to update the group table of the gui
                        emit updateGroups();

                        // parse the groups to update the drawing, in case the item was removed from an invisible group
                        QDomElement groups = createGroups();
                        if (not groups.isNull())
                        {
                            parseGroups(groups);
                        }

                        return item;
                    }
                }
            }
            itemNode = itemNode.nextSibling();
        }
    }
    else
    {
        qDebug() << "The group of id " << groupId << " doesn't exist";
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns true if the given group is empty
 * @param id
 * @return
 */
bool VAbstractPattern::isGroupEmpty(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);

    if (group.isNull() == false)
    {
        return not group.hasChildNodes();
    }
    else
    {
        qDebug() << "The group of id " << id << " doesn't exist";
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractPattern::getGroupVisibility(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        return getParameterBool(group, AttrVisible, trueStr);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupVisibility(quint32 id, bool visible)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttribute(group, AttrVisible, visible);
        modified = true;
        emit patternChanged(false);

        QDomElement groups = createGroups();
        if (not groups.isNull())
        {
            parseGroups(groups);
        }
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}


/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets whether a Group is locked or not
 * @param  id - Tool Id.
 * @return AttrGroupLocked - True if locked False if unlocked.
 * @return True if can't find group by id.
*/

bool VAbstractPattern::getGroupLock(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        return getParameterBool(group, AttrGroupLocked, trueStr);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupLock(quint32 id, bool locked)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttribute(group, AttrGroupLocked, locked);
        modified = true;
        emit patternChanged(false);
        //qDebug("VAbstractPattern::setGroupLock - Group %u is locked.", id);

        QDomElement groups = createGroups();
        if (not groups.isNull())
        {
            parseGroups(groups);
        }
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets the Group color
 * @param  id - Tool Id.
 * @return AttrGroupColor - Color of the group.
 * @return color  - group color.
*/

QString VAbstractPattern::getGroupColor(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    QString color;
    if (group.isElement())
    {

        color = GetParametrString(group, AttrGroupColor, ColorBlack);
        return color;
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return ColorBlack;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupColor(quint32 id, QString color)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttribute(group, AttrGroupColor, color);
        modified = true;
        emit patternChanged(false);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets the Group Line Type
 * @param  id - Tool Id.
 * @return AttrLineType - Line type of the group.
 * @return type  - group line type.
*/

QString VAbstractPattern::getGroupLineType(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    QString type;
    if (group.isElement())
    {

        type = GetParametrString(group, AttrLineType, LineTypeSolidLine);
        return type;
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return LineTypeSolidLine;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupLineType(quint32 id, QString type)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttribute(group, AttrLineType, type);
        modified = true;
        emit patternChanged(false);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets the Group Line Weight
 * @param  id - Tool Id.
 * @return AttrLineWeight - Linw weight of the group.
 * @return weight  - group line weight.
*/

QString VAbstractPattern::getGroupLineWeight(quint32 id)
{
    QDomElement group = elementById(id, TagGroup);
    QString weight;
    if (group.isElement())
    {

        weight = GetParametrString(group, AttrLineWeight, "0.35");
        return weight;
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
        return ColorBlack;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPattern::setGroupLineWeight(quint32 id, QString weight)
{
    QDomElement group = elementById(id, TagGroup);
    if (group.isElement())
    {
        SetAttribute(group, AttrLineWeight, weight);
        modified = true;
        emit patternChanged(false);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}

/*---------------------------------------------------------------------------------------------------------------------
 * @brief  Gets whether a group name already exists
 * @param  groupName - group name.
 * @return exists - True id name exists False if it does not.
*/
bool VAbstractPattern::groupNameExists(const QString &groupName)
{
    QStringList groupList = groupListByName();
    bool exists = groupList.contains(groupName, Qt::CaseInsensitive);
    return exists;
}

QString VAbstractPattern::useGroupColor(quint32 toolId, QString color)
{
    QMap<quint32,QString> groupsContainingItem = getGroupsContainingItem(toolId, 0, true);
    QStringList list = QStringList(groupsContainingItem.values());
    QString  groupName = list.value(0);

    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupColor - Group Name = %s",qUtf8Printable(groupName));
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupColor - Tool ID = %d",toolId);
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupColor - Tool Color = %s",qUtf8Printable(color));
    quint32 objectId = toolId;
    bool groupHasItem = hasGroupItem(getGroupByName(groupName), toolId, objectId);
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupColor - Group has item = %d",groupHasItem);
    if ((color == ColorByGroup) && groupHasItem)
    {
        QString groupColor = getGroupColor(getGroupIdByName(groupName));
        //qCDebug(vbstractPattern,"VAbstractPattern::useGroupColor - Group Color = %s",qUtf8Printable(groupColor));
        return groupColor;
    }
    else
    {
        return color;
    }
}

QString VAbstractPattern::useGroupLineType(quint32 toolId, QString type)
{
    QMap<quint32,QString> groupsContainingItem = getGroupsContainingItem(toolId, 0, true);
    QStringList list = QStringList(groupsContainingItem.values());
    QString  groupName = list.value(0);

    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupLineType - Group Name = %s",qUtf8Printable(groupName));
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupLineType - Tool ID = %d",toolId);
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupLineType - Tool Color = %s",qUtf8Printable(type));
    quint32 objectId = toolId;
    bool groupHasItem = hasGroupItem(getGroupByName(groupName), toolId, objectId);
    //qCDebug(vbstractPattern,"VAbstractPattern::useGroupLineType - Group has item = %d",groupHasItem);
    if ((type == LineTypeByGroup) && groupHasItem)
    {
        QString groupLineType = getGroupLineType(getGroupIdByName(groupName));
        //qCDebug(vbstractPattern,"VAbstractPattern::useGroupLineType - Group Linetpe = %s",qUtf8Printable(groupLineType));
        return groupLineType;
    }
    else
    {
        return type;
    }
}

QString VAbstractPattern::useGroupLineWeight(quint32 toolId, QString weight)
{
    QMap<quint32,QString> groupsContainingItem = getGroupsContainingItem(toolId, 0, true);
    QStringList list = QStringList(groupsContainingItem.values());
    QString  groupName = list.value(0);

    quint32 objectId = toolId;
    bool groupHasItem = hasGroupItem(getGroupByName(groupName), toolId, objectId);
    if ((weight == LineWeightByGroup) && groupHasItem)
    {
        QString groupLineWeight = getGroupLineWeight(getGroupIdByName(groupName));
        return groupLineWeight;
    }
    else
    {
        return weight;
    }
}

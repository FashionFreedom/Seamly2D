/******************************************************************************
 **  @file   svg_generator.cpp
 **  @author Evans PERRET
 **  @date   September 21, 2024
 **
 **  @brief
 **  Custom SVG generator to handle groups in SVGs
 **
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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
 *****************************************************************************/

#include "svg_generator.h"
#include "../vlayout/vlayoutdef.h"

#include <QFile>
#include <QDebug>
#include <QSvgGenerator>
#include <QGraphicsItem>
#include <QPainter>
#include <QBuffer>

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SvgGenerator constructor.
 * @param paper       paper rectangle defining the SVG size and view box.
 * @param name        output file path.
 * @param patternName pattern name, written as the data-name attribute of the pattern group.
 * @param description pattern description (currently unused in the output).
 * @param resolution  render resolution in DPI.
 */
SvgGenerator::SvgGenerator(QGraphicsRectItem *paper, QString name, QString patternName, QString description,
                           int resolution):
    m_paper(paper),
    m_filepath(name),
    m_patternName(patternName),
    m_description(description),
    m_resolution(resolution),
    m_pieceCount(0)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Merge all the SVGs in the m_domList list into a single SVG
 * @return The merged SVG as a DOM document
 * @details m_domList contains DOM representations of multiple SVGs
            Assuming each svg contains a main group containing every graphical item of the svg,
            this function adds to the first svg of the list all the main groups of the other svgs,
            thus creating a single svg with each svg of the list in it, every svg being in its own group.
            This function is used in order to create svgs containing groups
 */
QDomDocument SvgGenerator::mergeSvgDoms()
{
    if (m_domList.isEmpty()) {
        qDebug() << "Error : the SVG list is empty";
        return QDomDocument();
    }

    QDomDocument mergedSvg = m_domList.at(0).cloneNode().toDocument();

    QDomElement mergedSvgRoot = mergedSvg.documentElement();
    if (mergedSvgRoot.tagName() != "svg") {
        qDebug() << "Error : the first SVG does not contain a <svg> tag.";
        return QDomDocument();
    }

    QDomNodeList mergedSvgGroups = mergedSvgRoot.elementsByTagName("g");
    if (mergedSvgGroups.isEmpty()) {
        qDebug() << "Error : the SVG does not contain a <g> tag.";
        return QDomDocument();
    }
    // Drop the clone's own main group; it is re-added from the list below.
    mergedSvgGroups.at(0).parentNode().removeChild(mergedSvgGroups.at(0));

    // Piece-based exports get one pattern group wrapping all piece groups, tagged
    // with the SVG data-* attributes so downstream tools can identify the pattern.
    // Whole-scene exports (draft blocks) keep the legacy flat structure.
    QDomElement parentElement = mergedSvgRoot;
    if (m_pieceCount > 0)
    {
        QDomElement patternGroup = mergedSvg.createElement("g");
        patternGroup.setAttribute("id", "pattern-1");
        patternGroup.setAttribute("data-type", "pattern");
        patternGroup.setAttribute("data-type-number", "1");
        setAttribute(patternGroup, "data-name", m_patternName);
        mergedSvgRoot.appendChild(patternGroup);
        parentElement = patternGroup;
    }

    for (int i = 0; i < m_domList.size(); ++i) {
        QDomDocument domSvg = m_domList.at(i);
        QDomElement svgRoot = domSvg.documentElement();
        if (svgRoot.tagName() != "svg") {
            qDebug() << "Error : the SVG does not contain a <svg> tag.";
            return QDomDocument();
        }
        QDomNodeList svgGroups = svgRoot.elementsByTagName("g");
        if (svgGroups.isEmpty()) {
            qDebug() << "Error : the SVG does not contain a <g> tag.";
            return QDomDocument();
        }
        QDomElement mainGroup = svgGroups.at(0).toElement();
        cleanSvg(mainGroup);
        // importNode: nodes must be cloned into the target document before appending;
        // appending a node owned by another document is silently ignored by QDom.
        parentElement.appendChild(mergedSvg.importNode(mainGroup, true));
    }

    return mergedSvg;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Remove empty groups from the SVG
 * @return void
 * @details This function removes empty unuseful groups from the exported SVG.
 *          Those empty groups were generated by Qt svg generator.
 */
void SvgGenerator::removeEmptyGroups(QDomElement &mainGroup)
{
    bool svgCleaned = false;

    //We must remove the empty groups one by one until no more empty group is found.
    //Since removing a group modifies the QDomNodeList indexation, we start over
    //the search from the beginning each time a group is removed.
    while(!svgCleaned)
    {
        svgCleaned = true;
        QDomNodeList groups = mainGroup.elementsByTagName("g");

        for (int i = 0; i < groups.size(); ++i) {
            QDomElement group = groups.at(i).toElement();
            if (group.childNodes().isEmpty()) {
                // Remove from the group's real parent: empty groups can be nested at
                // any depth, not only directly below mainGroup.
                if (group.parentNode().removeChild(group).isNull()) {
                    qDebug() << "Error : could not remove empty group";
                }
                svgCleaned = false;
                break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Remove the M0,0 origin paths from the SVG
 * @return void
 * @details This function removes every empty "M0,0" path from the SVG. Qt's SVG
 *          generator emits such a path for each QGraphicsPathItem with an empty
 *          path (e.g. the invisible piece container and label group items). If
 *          these paths are not removed, the bounding box of the exported pattern
 *          piece when opened in a svg editing software can be wrong.
 */
void SvgGenerator::removeEmptyOriginPath(QDomElement &mainGroup)
{
    QDomNodeList paths = mainGroup.elementsByTagName("path");
    // Iterate backwards: QDomNodeList is live, removing a node reindexes the list.
    for (int i = paths.size() - 1; i >= 0; --i) {
        QDomElement path = paths.at(i).toElement();
        const QString d = path.attribute("d");
        if (d.isEmpty() || d == "M0,0") {
            QDomElement parentGroup = path.parentNode().toElement();
            parentGroup.removeChild(path);
            // Drop the wrapping group as well when the path was its only content.
            if (parentGroup != mainGroup && parentGroup.childNodes().isEmpty()) {
                parentGroup.parentNode().removeChild(parentGroup);
            }
        }
    }
}

//-----------------------------------------------------------------------------
/// @brief Clean the SVG
/// @return void
/// @details This function cleans the SVG by removing the empty origin "M0,0" paths
///          first (which may leave their wrapping groups empty) and then removing
///          all remaining empty groups.
//-----------------------------------------------------------------------------
void SvgGenerator::cleanSvg(QDomElement &mainGroup)
{
    removeEmptyOriginPath(mainGroup);
    removeEmptyGroups(mainGroup);
}

//-----------------------------------------------------------------------------
/// @brief setAttribute set an attribute in the svg
///
/// This method sets an attibute for the 1st elemnent by tag in the svg document.
///
/// @param element : the dom document element.
/// @param attr : attribute name.
/// @param value : value of the attibute.
/// @return void
//-----------------------------------------------------------------------------
void SvgGenerator::setAttribute(QDomElement element, const QString &attr, const QString &value)
{
    if (!element.isNull() && !value.isEmpty())
    {
        element.setAttribute(attr, value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Render a graphics scene into an SVG DOM document.
 *
 * The scene is painted through Qt's QSvgGenerator into an in-memory buffer and
 * the resulting SVG markup is parsed into a QDomDocument for post-processing.
 *
 * @param scene the scene to render.
 * @return the rendered SVG as a DOM document; a null document when parsing failed.
 */
QDomDocument SvgGenerator::renderSceneToDom(QGraphicsScene *scene)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QSvgGenerator svgGenerator;
    svgGenerator.setOutputDevice(&buffer);
    svgGenerator.setSize(m_paper->rect().size().toSize());
    svgGenerator.setViewBox(m_paper->rect());
    svgGenerator.setTitle(QString());
    svgGenerator.setDescription(QString());
    svgGenerator.setResolution(m_resolution);

    QPainter painter;
    painter.begin(&svgGenerator);
    painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter, m_paper->rect(), m_paper->rect(), Qt::IgnoreAspectRatio);
    painter.end();

    QDomDocument domDoc;
    if (!domDoc.setContent(byteArray))
    {
        qDebug() << "Error : Impossible to load the SVG content in the QDomDocument.";
        return QDomDocument();
    }

    buffer.close();
    return domDoc;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Render each component of a piece separately and append the tagged groups to the piece group.
 *
 * Qt's QSvgGenerator creates <g> elements from painter state changes, not from
 * graphics items, so a single render of the whole piece cannot be mapped back to
 * its components. Instead each direct child of the piece root (seamline, cutline,
 * notches, internal paths, labels, grainline) is rendered in its own pass with
 * every sibling hidden. Each pass therefore produces exactly one <g> that belongs
 * to one known component, which is tagged with the SVG data-* attributes:
 * data-type, data-type-number (per-type counter within the piece), data-parent
 * (the piece id) and a structured unique id ("<pieceId>-<type>-<n>").
 *
 * @param scene      scene containing (only) this piece's item tree.
 * @param item       piece root item whose children are the components.
 * @param pieceDoc   DOM document of the piece; receives the component groups.
 * @param pieceGroup piece <g> element the component groups are appended to.
 * @param pieceId    SVG id of the piece group, used to build data-parent and component ids.
 */
void SvgGenerator::addComponentGroups(QGraphicsScene *scene, QGraphicsItem *item, QDomDocument &pieceDoc,
                                      QDomElement &pieceGroup, const QString &pieceId)
{
    const QList<QGraphicsItem *> components = item->childItems();
    QHash<QString, int> typeCounters; // per data-type counter within this piece

    for (int i = 0; i < components.size(); ++i)
    {
        // Show only the current component; hide every sibling.
        for (int j = 0; j < components.size(); ++j)
        {
            components.at(j)->setVisible(i == j);
        }

        QDomDocument componentDoc = renderSceneToDom(scene);
        if (componentDoc.isNull())
        {
            continue;
        }

        QDomNodeList groups = componentDoc.documentElement().elementsByTagName("g");
        if (groups.isEmpty())
        {
            continue;
        }

        QDomElement componentGroup = groups.at(0).toElement();
        cleanSvg(componentGroup);
        if (componentGroup.childNodes().isEmpty())
        {
            // Component painted nothing (e.g. a piece without notches) — skip it.
            continue;
        }

        // Tag the group so downstream tools (SeamlyLayout) can identify the component.
        QString type = components.at(i)->data(PieceItemData::ItemType).toString();
        if (type.isEmpty())
        {
            type = QStringLiteral("unknown");
        }
        const int typeNumber = ++typeCounters[type];
        componentGroup.setAttribute("id", QString("%1-%2-%3").arg(pieceId, type).arg(typeNumber));
        componentGroup.setAttribute("data-type", type);
        componentGroup.setAttribute("data-type-number", QString::number(typeNumber));
        componentGroup.setAttribute("data-parent", pieceId);

        pieceGroup.appendChild(pieceDoc.importNode(componentGroup, true));
    }

    // Restore visibility so later exports (PDF, DXF, previews) see the whole piece.
    for (int j = 0; j < components.size(); ++j)
    {
        components.at(j)->setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add a new SVG to the list of SVGs to be merged into a single SVG.
 *
 * When a piece item is given, the piece becomes a <g> element carrying the SVG
 * data-* attributes (data-type="piece", data-type-number, data-parent,
 * data-name and data-letter) and each of the piece's components is rendered
 * and tagged individually (see addComponentGroups()). Without an item the
 * scene is rendered as-is in a single untagged group (whole-scene exports,
 * e.g. draft blocks).
 *
 * @param scene the scene that must be converted to SVG.
 * @param item  piece root item the scene contains; nullptr for whole-scene exports.
 */
void SvgGenerator::addSvgFromScene(QGraphicsScene *scene, QGraphicsItem *item)
{
    if (item == nullptr)
    {
        // Whole-scene export: keep the legacy single-group behavior.
        QDomDocument domDoc = renderSceneToDom(scene);
        if (!domDoc.isNull())
        {
            m_domList.append(domDoc);
        }
        return;
    }

    // One piece: build "<g id='piece-<n>' data-type='piece' ...>" holding one tagged group per component.
    ++m_pieceCount;
    const QString pieceId = QString("piece-%1").arg(m_pieceCount);

    // Full render only to obtain a document with the correct <svg> root attributes.
    QDomDocument pieceDoc = renderSceneToDom(scene);
    if (pieceDoc.isNull())
    {
        return;
    }

    QDomElement svgRoot = pieceDoc.documentElement();
    QDomNodeList rootGroups = svgRoot.elementsByTagName("g");
    if (!rootGroups.isEmpty())
    {
        // Drop the untagged full-piece group; it is replaced by tagged component groups.
        rootGroups.at(0).parentNode().removeChild(rootGroups.at(0));
    }

    QDomElement pieceGroup = pieceDoc.createElement("g");
    pieceGroup.setAttribute("id", pieceId);
    pieceGroup.setAttribute("data-type", "piece");
    pieceGroup.setAttribute("data-type-number", QString::number(m_pieceCount));
    pieceGroup.setAttribute("data-parent", "pattern-1");
    setAttribute(pieceGroup, "data-name", item->data(PieceItemData::ObjectName).toString());
    setAttribute(pieceGroup, "data-letter", item->data(PieceItemData::PieceLetter).toString());
    svgRoot.appendChild(pieceGroup);

    addComponentGroups(scene, item, pieceDoc, pieceGroup, pieceId);

    m_domList.append(pieceDoc);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Generate the merged SVG where each previously given scene is grouped separately.
 * @return void
 * @details This function merges the SVGs of the m_domList list and writes the result
            in a file at the path given in the constructor.
*/
void SvgGenerator::generate()
{
    QDomDocument mergedSvg = mergeSvgDoms();

    QFile outputFile(m_filepath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error : Couldn't write the output file.";
        return;
    }

    QTextStream stream(&outputFile);
    stream << mergedSvg.toString();
    outputFile.close();

    qDebug() << "Merged SVG Generated!";
}

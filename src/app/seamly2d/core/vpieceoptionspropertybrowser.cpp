/******************************************************************************
*   @file   vpieceoptionspropertybrowser.cpp
**  @author Douglas S Caskey
**  @date   27 Jun, 2026
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2026 Seamly2D project
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

#include "vpieceoptionspropertybrowser.h"

#include "../core/application_2d.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpropertyexplorer/vproperties.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/undocommands/savepatternfabricsettings.h"
#include "../vtools/undocommands/savepieceoptions.h"

#include <QDockWidget>
#include <QScrollArea>
#include <QDebug>

namespace
{
const QString IdName               = QStringLiteral("name");
const QString IdColor              = QStringLiteral("color");
const QString IdFill               = QStringLiteral("fill");
const QString IdSeamAllowance      = QStringLiteral("seamAllowance");
const QString IdSABuiltIn          = QStringLiteral("seamAllowanceBuiltIn");
const QString IdHideSeamLine       = QStringLiteral("hideSeamLine");
const QString IdLabelSelection     = QStringLiteral("labelSelection");
const QString IdLabelProperties    = QStringLiteral("labelProperties");
const QString IdLabelSeamAllowance = QStringLiteral("labelSeamAllowance");
const QString IdLabelFabric        = QStringLiteral("labelFabric");
const QString IdLabelSADefaults    = QStringLiteral("labelSADefaults");
const QString IdFabricWidth        = QStringLiteral("fabricWidth");
const QString IdHeightRepeat        = QStringLiteral("heightRepeat");
const QString IdLengthRepeat       = QStringLiteral("lengthRepeat");
const QString IdShrinkagePercent   = QStringLiteral("shrinkagePercent");
const QString IdStretchPercent     = QStringLiteral("stretchPercent");
const QString IdDefaultSAWidth     = QStringLiteral("defaultSAWidth");
}

//---------------------------------------------------------------------------------------------------------------------
VPieceOptionsPropertyBrowser::VPieceOptionsPropertyBrowser(VAbstractPattern *doc, QDockWidget *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_propertyModel(nullptr)
    , m_formView(nullptr)
    , m_scrollArea(nullptr)
    , m_currentItem(nullptr)
    , m_propertyToId(QMap<VPE::VProperty *, QString>())
    , m_idToProperty(QMap<QString, VPE::VProperty *>())
    , m_showingPatternSettings(false)
{
    m_propertyModel = new VPE::VPropertyModel(this);
    m_formView = new VPE::VPropertyFormView(m_propertyModel, parent);
    m_formView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_scrollArea = new QScrollArea(parent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_formView);

    parent->setWidget(m_scrollArea);

    connect(m_propertyModel, &VPE::VPropertyModel::onDataChangedByEditor, this,
            &VPieceOptionsPropertyBrowser::userChangedData);
}

//---------------------------------------------------------------------------------------------------------------------
QScrollArea *VPieceOptionsPropertyBrowser::scrollArea() const
{
    return m_scrollArea;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::clearPropertyBrowser()
{
    m_propertyModel->clear();
    m_propertyToId.clear();
    m_idToProperty.clear();
    m_currentItem = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::itemClicked(QGraphicsItem *item)
{
    if (item == nullptr)
    {
        showPatternSettings();
        return;
    }

    if (item == m_currentItem && !m_idToProperty.isEmpty())
    {
        updateOptions();
        return;
    }

    clearPropertyBrowser();

    QGraphicsItem *pieceItem = item;
    while (pieceItem != nullptr && pieceItem->type() != PatternPieceTool::Type)
    {
        pieceItem = pieceItem->parentItem();
    }

    if (pieceItem != nullptr)
    {
        m_currentItem = pieceItem;
        showPieceOptions(pieceItem);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::updateOptions()
{
    if (m_showingPatternSettings)
    {
        updatePatternSettings();
        return;
    }

    if (m_currentItem == nullptr || m_idToProperty.isEmpty())
    {
        return;
    }

    if (m_currentItem->type() == PatternPieceTool::Type)
    {
        updatePieceOptions();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::userChangedData(VPE::VProperty *property)
{
    if (property == nullptr)
    {
        return;
    }

    if (m_showingPatternSettings)
    {
        changePatternSettingsData(property);
    }
    else if (m_currentItem != nullptr && m_currentItem->type() == PatternPieceTool::Type)
    {
        changePieceData(property);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::addProperty(VPE::VProperty *property, const QString &id)
{
    m_propertyToId[property] = id;
    m_idToProperty[id] = property;
    m_propertyModel->addProperty(property, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::showPieceOptions(QGraphicsItem *item)
{
    PatternPieceTool *tool = qgraphicsitem_cast<PatternPieceTool *>(item);
    if (tool == nullptr)
    {
        return;
    }

    m_showingPatternSettings = false;
    m_formView->setTitle(tr("Pattern Piece"));

    const VPiece piece = tool->getData()->GetPiece(tool->getId());
    const QStringList yesNo = QStringList() << tr("No") << tr("Yes");

    // --- Selection ---
    auto *labelSelection = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Selection") + QStringLiteral("</b>"));
    labelSelection->setValue(QString());
    labelSelection->setPropertyType(VPE::Property::Label);
    addProperty(labelSelection, IdLabelSelection);

    auto *nameProperty = new VPE::VStringProperty(tr("Name:"));
    nameProperty->setClearButtonEnable(true);
    nameProperty->setValue(piece.GetName());
    addProperty(nameProperty, IdName);

    // --- Properties ---
    auto *labelProperties = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Properties") + QStringLiteral("</b>"));
    labelProperties->setValue(QString());
    labelProperties->setPropertyType(VPE::Property::Label);
    addProperty(labelProperties, IdLabelProperties);

    auto *colorProperty = new VPE::VLineColorProperty(tr("Color:"));
    colorProperty->setColors(VAbstractTool::ColorsList());
    const qint32 colorIndex = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), piece.getColor());
    colorProperty->setValue(colorIndex);
    addProperty(colorProperty, IdColor);

    auto *fillProperty = new VPE::VEnumProperty(tr("Fill:"));
    fillProperty->setLiterals(VAbstractTool::fills());
    const qint32 fillIndex = VAbstractTool::fills().indexOf(piece.getFill());
    fillProperty->setValue(fillIndex >= 0 ? fillIndex : 0);
    addProperty(fillProperty, IdFill);

    // --- Seam Allowance ---
    auto *labelSA = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Seam Allowance") + QStringLiteral("</b>"));
    labelSA->setValue(QString());
    labelSA->setPropertyType(VPE::Property::Label);
    addProperty(labelSA, IdLabelSeamAllowance);

    auto *saEnabledProperty = new VPE::VEnumProperty(tr("Enabled:"));
    saEnabledProperty->setLiterals(yesNo);
    saEnabledProperty->setValue(piece.IsSeamAllowance() ? 1 : 0);
    addProperty(saEnabledProperty, IdSeamAllowance);

    auto *saBuiltInProperty = new VPE::VEnumProperty(tr("Built in:"));
    saBuiltInProperty->setLiterals(yesNo);
    saBuiltInProperty->setValue(piece.IsSeamAllowanceBuiltIn() ? 1 : 0);
    addProperty(saBuiltInProperty, IdSABuiltIn);

    auto *hideSeamLineProperty = new VPE::VEnumProperty(tr("Hide seam line:"));
    hideSeamLineProperty->setLiterals(yesNo);
    hideSeamLineProperty->setValue(piece.isHideSeamLine() ? 1 : 0);
    addProperty(hideSeamLineProperty, IdHideSeamLine);
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::updatePieceOptions()
{
    PatternPieceTool *tool = qgraphicsitem_cast<PatternPieceTool *>(m_currentItem);
    if (tool == nullptr)
    {
        return;
    }

    const VPiece piece = tool->getData()->GetPiece(tool->getId());

    if (m_idToProperty.contains(IdName))
    {
        m_idToProperty[IdName]->setValue(piece.GetName());
    }

    if (m_idToProperty.contains(IdColor))
    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), piece.getColor());
        m_idToProperty[IdColor]->setValue(index);
    }

    if (m_idToProperty.contains(IdFill))
    {
        const qint32 index = VAbstractTool::fills().indexOf(piece.getFill());
        m_idToProperty[IdFill]->setValue(index >= 0 ? index : 0);
    }

    if (m_idToProperty.contains(IdSeamAllowance))
    {
        m_idToProperty[IdSeamAllowance]->setValue(piece.IsSeamAllowance() ? 1 : 0);
    }

    if (m_idToProperty.contains(IdSABuiltIn))
    {
        m_idToProperty[IdSABuiltIn]->setValue(piece.IsSeamAllowanceBuiltIn() ? 1 : 0);
    }

    if (m_idToProperty.contains(IdHideSeamLine))
    {
        m_idToProperty[IdHideSeamLine]->setValue(piece.isHideSeamLine() ? 1 : 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::changePieceData(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    PatternPieceTool *tool = qgraphicsitem_cast<PatternPieceTool *>(m_currentItem);
    if (tool == nullptr)
    {
        return;
    }

    const QString id = m_propertyToId[property];
    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
    const quint32 toolId = tool->getId();

    VPiece oldPiece = tool->getData()->GetPiece(toolId);
    VPiece newPiece = oldPiece;

    if (id == IdName)
    {
        const QString name = value.toString();
        if (name.isEmpty())
        {
            return;
        }
        newPiece.SetName(name);
    }
    else if (id == IdColor)
    {
        const QMap<QString, QString> colors = VAbstractTool::ColorsList();
        const QStringList keys = colors.keys();
        const int index = value.toInt();
        if (index >= 0 && index < keys.size())
        {
            newPiece.setColor(keys.at(index));
        }
        else
        {
            return;
        }
    }
    else if (id == IdFill)
    {
        const QStringList fillsList = VAbstractTool::fills();
        const int index = value.toInt();
        if (index >= 0 && index < fillsList.size())
        {
            newPiece.setFill(fillsList.at(index));
        }
        else
        {
            return;
        }
    }
    else if (id == IdSeamAllowance)
    {
        newPiece.SetSeamAllowance(value.toInt() != 0);
    }
    else if (id == IdSABuiltIn)
    {
        newPiece.SetSeamAllowanceBuiltIn(value.toInt() != 0);
    }
    else if (id == IdHideSeamLine)
    {
        newPiece.setHideSeamLine(value.toInt() != 0);
    }
    else
    {
        return;
    }

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, m_doc, toolId);
    qApp->getUndoStack()->push(undoCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::showPatternSettings()
{
    clearPropertyBrowser();
    m_showingPatternSettings = true;

    m_formView->setTitle(tr("Fabric Settings"));

    const VFabricSettings settings = m_doc->GetFabricSettings();
    const QString units = UnitsToStr(qApp->patternUnit());

    auto *labelFabric = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Fabric") + QStringLiteral("</b>"));
    labelFabric->setValue(QString());
    labelFabric->setPropertyType(VPE::Property::Label);
    addProperty(labelFabric, IdLabelFabric);

    auto *fabricWidth = new VPE::DoubleSpinboxProperty(tr("Width:") + " (" + units + ")");
    fabricWidth->setValue(settings.fabricWidth);
    fabricWidth->setSetting(QStringLiteral("Min"), 0.0);
    fabricWidth->setSetting(QStringLiteral("Max"), 100000.0);
    fabricWidth->setSetting(QStringLiteral("Precision"), 2);
    addProperty(fabricWidth, IdFabricWidth);

    auto *heightRepeat = new VPE::DoubleSpinboxProperty(tr("Height repeat:") + " (" + units + ")");
    heightRepeat->setValue(settings.heightRepeat);
    heightRepeat->setSetting(QStringLiteral("Min"), 0.0);
    heightRepeat->setSetting(QStringLiteral("Max"), 100000.0);
    heightRepeat->setSetting(QStringLiteral("Precision"), 2);
    addProperty(heightRepeat, IdHeightRepeat);

    auto *lengthRepeat = new VPE::DoubleSpinboxProperty(tr("Length repeat:") + " (" + units + ")");
    lengthRepeat->setValue(settings.lengthRepeat);
    lengthRepeat->setSetting(QStringLiteral("Min"), 0.0);
    lengthRepeat->setSetting(QStringLiteral("Max"), 100000.0);
    lengthRepeat->setSetting(QStringLiteral("Precision"), 2);
    addProperty(lengthRepeat, IdLengthRepeat);

    auto *shrinkage = new VPE::DoubleSpinboxProperty(tr("Shrinkage:") + " (%)");
    shrinkage->setValue(settings.shrinkagePercent);
    shrinkage->setSetting(QStringLiteral("Min"), 0.0);
    shrinkage->setSetting(QStringLiteral("Max"), 100.0);
    shrinkage->setSetting(QStringLiteral("Precision"), 2);
    addProperty(shrinkage, IdShrinkagePercent);

    auto *stretch = new VPE::DoubleSpinboxProperty(tr("Stretch:") + " (%)");
    stretch->setValue(settings.stretchPercent);
    stretch->setSetting(QStringLiteral("Min"), 0.0);
    stretch->setSetting(QStringLiteral("Max"), 100.0);
    stretch->setSetting(QStringLiteral("Precision"), 2);
    addProperty(stretch, IdStretchPercent);

    auto *labelSA = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Seam Allowance") + QStringLiteral("</b>"));
    labelSA->setValue(QString());
    labelSA->setPropertyType(VPE::Property::Label);
    addProperty(labelSA, IdLabelSADefaults);

    qreal saDefault = settings.defaultSAWidth;
    if (qFuzzyIsNull(saDefault))
    {
        saDefault = qApp->Settings()->GetDefaultSeamAllowance();
    }

    auto *defaultSA = new VPE::DoubleSpinboxProperty(tr("Default width:") + " (" + units + ")");
    defaultSA->setValue(saDefault);
    defaultSA->setSetting(QStringLiteral("Min"), 0.0);
    defaultSA->setSetting(QStringLiteral("Max"), 100000.0);
    defaultSA->setSetting(QStringLiteral("Precision"), 2);
    addProperty(defaultSA, IdDefaultSAWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::updatePatternSettings()
{
    if (m_idToProperty.isEmpty())
    {
        return;
    }

    const VFabricSettings settings = m_doc->GetFabricSettings();

    if (m_idToProperty.contains(IdFabricWidth))
    {
        m_idToProperty[IdFabricWidth]->setValue(settings.fabricWidth);
    }
    if (m_idToProperty.contains(IdHeightRepeat))
    {
        m_idToProperty[IdHeightRepeat]->setValue(settings.heightRepeat);
    }
    if (m_idToProperty.contains(IdLengthRepeat))
    {
        m_idToProperty[IdLengthRepeat]->setValue(settings.lengthRepeat);
    }
    if (m_idToProperty.contains(IdShrinkagePercent))
    {
        m_idToProperty[IdShrinkagePercent]->setValue(settings.shrinkagePercent);
    }
    if (m_idToProperty.contains(IdStretchPercent))
    {
        m_idToProperty[IdStretchPercent]->setValue(settings.stretchPercent);
    }
    if (m_idToProperty.contains(IdDefaultSAWidth))
    {
        m_idToProperty[IdDefaultSAWidth]->setValue(settings.defaultSAWidth);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::changePatternSettingsData(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QString id = m_propertyToId[property];
    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);

    VFabricSettings oldSettings = m_doc->GetFabricSettings();
    VFabricSettings newSettings = oldSettings;

    if (id == IdFabricWidth)
    {
        newSettings.fabricWidth = value.toDouble();
    }
    else if (id == IdHeightRepeat)
    {
        newSettings.heightRepeat = value.toDouble();
    }
    else if (id == IdLengthRepeat)
    {
        newSettings.lengthRepeat = value.toDouble();
    }
    else if (id == IdShrinkagePercent)
    {
        newSettings.shrinkagePercent = value.toDouble();
    }
    else if (id == IdStretchPercent)
    {
        newSettings.stretchPercent = value.toDouble();
    }
    else if (id == IdDefaultSAWidth)
    {
        newSettings.defaultSAWidth = value.toDouble();
    }
    else
    {
        return;
    }

    SavePatternFabricSettings *undoCommand = new SavePatternFabricSettings(oldSettings, newSettings, m_doc);
    qApp->getUndoStack()->push(undoCommand);
}

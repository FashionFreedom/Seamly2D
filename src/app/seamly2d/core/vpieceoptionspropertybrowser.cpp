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
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpropertyexplorer/vproperties.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/undocommands/movepiece.h"
#include "../vtools/undocommands/savepatternfabricsettings.h"
#include "../vtools/undocommands/savepieceoptions.h"

#include <QDockWidget>
#include <QScrollArea>
#include <QTimer>
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
const QString IdSelvedge           = QStringLiteral("selvedge");
const QString IdHeightRepeat        = QStringLiteral("heightRepeat");
const QString IdLengthRepeat       = QStringLiteral("lengthRepeat");
const QString IdShrinkagePercent   = QStringLiteral("shrinkagePercent");
const QString IdStretchPercent     = QStringLiteral("stretchPercent");
const QString IdDefaultSAWidth     = QStringLiteral("defaultSAWidth");
const QString IdLabelNotch         = QStringLiteral("labelNotch");
const QString IdDefaultNotchType   = QStringLiteral("defaultNotchType");
const QString IdDefaultNotchSubType = QStringLiteral("defaultNotchSubType");
const QString IdLabelSnapping      = QStringLiteral("labelSnapping");
const QString IdRapportSnapping    = QStringLiteral("rapportSnapping");
const QString IdLabelRapport       = QStringLiteral("labelRapport");
const QString IdRapportAnchor      = QStringLiteral("rapportAnchor");
const QString IdRapportAlign       = QStringLiteral("rapportAlign");
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
    , m_fabricDefaults()
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

    // --- Rapport Alignment ---
    const VFabricSettings fabricSettings = m_doc->GetFabricSettings();
    if (fabricSettings.heightRepeat > 0 || fabricSettings.lengthRepeat > 0)
    {
        auto *labelRapport = new VPE::VLabelProperty(
            QStringLiteral("<b>") + tr("Rapport Alignment") + QStringLiteral("</b>"));
        labelRapport->setValue(QString());
        labelRapport->setPropertyType(VPE::Property::Label);
        addProperty(labelRapport, IdLabelRapport);

        const VContainer *data = tool->getData();
        const QVector<VPieceNode> nodes = piece.GetPath().getNodes();
        QStringList pointNames;
        pointNames << tr("-- None --");
        int savedAnchorIndex = 0;
        const quint32 savedAnchorId = piece.GetRapportAnchorId();
        int pointCount = 0;

        for (int i = 0; i < nodes.size(); ++i)
        {
            if (nodes.at(i).GetTypeTool() == Tool::NodePoint)
            {
                try
                {
                    const QSharedPointer<VPointF> point =
                        data->GeometricObject<VPointF>(nodes.at(i).GetId());
                    pointNames << point->name();
                    ++pointCount;
                    if (nodes.at(i).GetId() == savedAnchorId)
                    {
                        savedAnchorIndex = pointCount;
                    }
                }
                catch (...)
                {
                    continue;
                }
            }
        }

        auto *anchorProperty = new VPE::VEnumProperty(tr("Anchor point:"));
        anchorProperty->setLiterals(pointNames);
        anchorProperty->setValue(savedAnchorIndex);
        addProperty(anchorProperty, IdRapportAnchor);

        const QStringList alignOptions = QStringList()
            << tr("Nearest intersection")
            << tr("Half height")
            << tr("Half length")
            << tr("Center")
            << tr("Next to piece");

        auto *alignProperty = new VPE::VEnumProperty(tr("Align to:"));
        alignProperty->setLiterals(alignOptions);
        alignProperty->setValue(piece.GetRapportAlignMode());
        addProperty(alignProperty, IdRapportAlign);
    }
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
    else if (id == IdRapportAlign)
    {
        newPiece.SetRapportAlignMode(value.toInt());
        SavePieceOptions *saveCmd = new SavePieceOptions(oldPiece, newPiece, m_doc, toolId);
        qApp->getUndoStack()->push(saveCmd);
        tool->snapToRapport();
        return;
    }
    else if (id == IdRapportAnchor)
    {
        const int anchorIndex = value.toInt();
        if (anchorIndex <= 0)
        {
            newPiece.SetRapportAnchorId(0);
        }
        else
        {
            const QVector<VPieceNode> nodes = oldPiece.GetPath().getNodes();
            int pointCount = 0;
            for (int i = 0; i < nodes.size(); ++i)
            {
                if (nodes.at(i).GetTypeTool() == Tool::NodePoint)
                {
                    ++pointCount;
                    if (pointCount == anchorIndex)
                    {
                        newPiece.SetRapportAnchorId(nodes.at(i).GetId());
                        break;
                    }
                }
            }
        }
        // Save anchor first, then auto-snap
        SavePieceOptions *saveCmd = new SavePieceOptions(oldPiece, newPiece, m_doc, toolId);
        qApp->getUndoStack()->push(saveCmd);
        tool->snapToRapport();
        return;
    }
    else
    {
        return;
    }

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, m_doc, toolId);
    qApp->getUndoStack()->push(undoCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::setFabricDefaults(const VFabricSettings &defaults)
{
    m_fabricDefaults = defaults;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceOptionsPropertyBrowser::showPatternSettings()
{
    clearPropertyBrowser();
    m_showingPatternSettings = true;

    const VFabricSettings settings = m_doc->GetFabricSettings();
    const bool hasFabric = !qFuzzyIsNull(settings.fabricWidth);

    if (!hasFabric)
    {
        m_formView->setTitle(tr("Pattern Settings"));
        auto *infoLabel = new VPE::VLabelProperty(tr("No fabric loaded. Use Fabric menu to add fabrics."));
        infoLabel->setValue(QString());
        infoLabel->setPropertyType(VPE::Property::Label);
        addProperty(infoLabel, QStringLiteral("infoNoFabric"));
        return;
    }

    m_formView->setTitle(tr("Fabric Settings"));
    const QString units = UnitsToStr(qApp->patternUnit());
    const VFabricSettings &def = m_fabricDefaults;

    auto defHint = [](const QString &label, qreal current, qreal fabricDefault) -> QString
    {
        if (!qFuzzyIsNull(fabricDefault) && !qFuzzyCompare(current, fabricDefault))
        {
            return label + QStringLiteral(" [") + QString::number(fabricDefault, 'f', 1) + QStringLiteral("]");
        }
        return label;
    };

    auto *labelFabric = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Fabric") + QStringLiteral("</b>"));
    labelFabric->setValue(QString());
    labelFabric->setPropertyType(VPE::Property::Label);
    addProperty(labelFabric, IdLabelFabric);

    auto addResetEnum = [this](const QString &id, qreal current, qreal fabricDefault)
    {
        if (!qFuzzyIsNull(fabricDefault) && !qFuzzyCompare(current, fabricDefault))
        {
            const QStringList opts = QStringList()
                << QStringLiteral("---")
                << tr("Reset to %1").arg(QString::number(fabricDefault, 'f', 2));
            auto *resetProp = new VPE::VEnumProperty(QStringLiteral(""));
            resetProp->setLiterals(opts);
            resetProp->setValue(0);
            addProperty(resetProp, QStringLiteral("reset_") + id);
        }
    };

    auto *fabricWidth = new VPE::DoubleSpinboxProperty(
        defHint(tr("Width:") + " (" + units + ")", settings.fabricWidth, def.fabricWidth));
    fabricWidth->setValue(settings.fabricWidth);
    fabricWidth->setSetting(QStringLiteral("Min"), 0.0);
    fabricWidth->setSetting(QStringLiteral("Max"), 100000.0);
    fabricWidth->setSetting(QStringLiteral("Precision"), 2);
    addProperty(fabricWidth, IdFabricWidth);
    addResetEnum(IdFabricWidth, settings.fabricWidth, def.fabricWidth);

    auto *selvedgeProp = new VPE::DoubleSpinboxProperty(
        defHint(tr("Selvedge:") + " (" + units + ")", settings.selvedge, def.selvedge));
    selvedgeProp->setValue(settings.selvedge);
    selvedgeProp->setSetting(QStringLiteral("Min"), 0.0);
    selvedgeProp->setSetting(QStringLiteral("Max"), 1000.0);
    selvedgeProp->setSetting(QStringLiteral("Precision"), 2);
    addProperty(selvedgeProp, IdSelvedge);
    addResetEnum(IdSelvedge, settings.selvedge, def.selvedge);

    auto *heightRepeat = new VPE::DoubleSpinboxProperty(
        defHint(tr("Height repeat:") + " (" + units + ")", settings.heightRepeat, def.heightRepeat));
    heightRepeat->setValue(settings.heightRepeat);
    heightRepeat->setSetting(QStringLiteral("Min"), 0.0);
    heightRepeat->setSetting(QStringLiteral("Max"), 100000.0);
    heightRepeat->setSetting(QStringLiteral("Precision"), 2);
    addProperty(heightRepeat, IdHeightRepeat);
    addResetEnum(IdHeightRepeat, settings.heightRepeat, def.heightRepeat);

    auto *lengthRepeat = new VPE::DoubleSpinboxProperty(
        defHint(tr("Length repeat:") + " (" + units + ")", settings.lengthRepeat, def.lengthRepeat));
    lengthRepeat->setValue(settings.lengthRepeat);
    lengthRepeat->setSetting(QStringLiteral("Min"), 0.0);
    lengthRepeat->setSetting(QStringLiteral("Max"), 100000.0);
    lengthRepeat->setSetting(QStringLiteral("Precision"), 2);
    addProperty(lengthRepeat, IdLengthRepeat);
    addResetEnum(IdLengthRepeat, settings.lengthRepeat, def.lengthRepeat);

    auto *shrinkage = new VPE::DoubleSpinboxProperty(
        defHint(tr("Shrinkage:") + " (%)", settings.shrinkagePercent, def.shrinkagePercent));
    shrinkage->setValue(settings.shrinkagePercent);
    shrinkage->setSetting(QStringLiteral("Min"), 0.0);
    shrinkage->setSetting(QStringLiteral("Max"), 100.0);
    shrinkage->setSetting(QStringLiteral("Precision"), 2);
    addProperty(shrinkage, IdShrinkagePercent);
    addResetEnum(IdShrinkagePercent, settings.shrinkagePercent, def.shrinkagePercent);

    auto *stretch = new VPE::DoubleSpinboxProperty(
        defHint(tr("Stretch:") + " (%)", settings.stretchPercent, def.stretchPercent));
    stretch->setValue(settings.stretchPercent);
    stretch->setSetting(QStringLiteral("Min"), 0.0);
    stretch->setSetting(QStringLiteral("Max"), 100.0);
    stretch->setSetting(QStringLiteral("Precision"), 2);
    addProperty(stretch, IdStretchPercent);
    addResetEnum(IdStretchPercent, settings.stretchPercent, def.stretchPercent);

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
    defaultSA->setSetting(QStringLiteral("Step"), 0.1);
    defaultSA->setSetting(QStringLiteral("Precision"), 1);
    addProperty(defaultSA, IdDefaultSAWidth);

    // Default notch settings applied to notches added from the Canvas Editor.
    auto *labelNotch = new VPE::VLabelProperty(QStringLiteral("<b>") + tr("Notches") + QStringLiteral("</b>"));
    labelNotch->setValue(QString());
    labelNotch->setPropertyType(VPE::Property::Label);
    addProperty(labelNotch, IdLabelNotch);

    auto *notchType = new VPE::VEnumProperty(tr("Default type:"));
    notchType->setLiterals(QStringList()
                           << tr("Slit")
                           << tr("T-Notch")
                           << tr("V-Notch (internal)")
                           << tr("V-Notch (external)")
                           << tr("U-Notch")
                           << tr("Castle")
                           << tr("Diamond"));
    notchType->setValue(qApp->Settings()->GetDefaultNotchType());
    addProperty(notchType, IdDefaultNotchType);

    auto *notchSubType = new VPE::VEnumProperty(tr("Default alignment:"));
    notchSubType->setLiterals(QStringList()
                              << tr("Straightforward")
                              << tr("Bisector")
                              << tr("Intersection"));
    notchSubType->setValue(qApp->Settings()->GetDefaultNotchSubType());
    addProperty(notchSubType, IdDefaultNotchSubType);
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
    if (m_idToProperty.contains(IdSelvedge))
    {
        m_idToProperty[IdSelvedge]->setValue(settings.selvedge);
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
    else if (id == IdSelvedge)
    {
        newSettings.selvedge = value.toDouble();
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
        const qreal newSA = value.toDouble();
        newSettings.defaultSAWidth = newSA;

        // Update all pieces' SA width to the new default
        if (!qFuzzyIsNull(newSA))
        {
            QGraphicsScene *scene = qApp->getCurrentScene();
            if (scene != nullptr)
            {
                const QList<QGraphicsItem *> items = scene->items();
                for (QGraphicsItem *item : items)
                {
                    if (item->type() == PatternPieceTool::Type)
                    {
                        PatternPieceTool *tool = qgraphicsitem_cast<PatternPieceTool *>(item);
                        if (tool == nullptr) { continue; }
                        const quint32 pieceId = tool->getId();
                        VPiece oldPiece = tool->getData()->GetPiece(pieceId);
                        VPiece updatedPiece = oldPiece;
                        updatedPiece.setSeamAllowanceWidthFormula(
                            qApp->LocaleToString(newSA), newSA);
                        SavePieceOptions *cmd = new SavePieceOptions(
                            oldPiece, updatedPiece, m_doc, pieceId);
                        qApp->getUndoStack()->push(cmd);
                    }
                }
            }
        }
    }
    else if (id == IdDefaultNotchType)
    {
        qApp->Settings()->SetDefaultNotchType(value.toInt());
        return;
    }
    else if (id == IdDefaultNotchSubType)
    {
        qApp->Settings()->SetDefaultNotchSubType(value.toInt());
        return;
    }
    else if (id.startsWith(QStringLiteral("reset_")))
    {
        if (value.toInt() <= 0)
        {
            return;
        }
        const QString targetId = id.mid(6);
        if (targetId == IdFabricWidth)        { newSettings.fabricWidth = m_fabricDefaults.fabricWidth; }
        else if (targetId == IdSelvedge)      { newSettings.selvedge = m_fabricDefaults.selvedge; }
        else if (targetId == IdHeightRepeat)  { newSettings.heightRepeat = m_fabricDefaults.heightRepeat; }
        else if (targetId == IdLengthRepeat)  { newSettings.lengthRepeat = m_fabricDefaults.lengthRepeat; }
        else if (targetId == IdShrinkagePercent) { newSettings.shrinkagePercent = m_fabricDefaults.shrinkagePercent; }
        else if (targetId == IdStretchPercent)   { newSettings.stretchPercent = m_fabricDefaults.stretchPercent; }
        else { return; }
    }
    else
    {
        return;
    }

    SavePatternFabricSettings *undoCommand = new SavePatternFabricSettings(oldSettings, newSettings, m_doc);
    qApp->getUndoStack()->push(undoCommand);

    // SA default is per-document, not synced to app settings.
    // It overrides the app default when creating new pieces.

    QTimer::singleShot(0, this, &VPieceOptionsPropertyBrowser::showPatternSettings);
}

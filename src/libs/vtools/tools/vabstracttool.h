//---------------------------------------------------------------------------------------------------------------------
/// @file   vabstracttool.h
/// @author Douglas S Caskey
/// @date   17 Sep, 2023
///
/// @copyright
/// Copyright (C) 2017 - 2026 Seamly, LLC
/// https://github.com/fashionfreedom/seamly2d
///
/// @brief
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
/// @file   vabstracttool.h
/// @author Roman Telezhynskyi <dismine(at)gmail.com>
/// @date   November 15, 2013
///
/// @brief
/// @copyright
/// This source code is part of the Valentina project, a pattern making
/// program, whose allow create and modeling patterns of clothing.
/// Copyright (C) 2013 Valentina project
/// <https://bitbucket.org/dismine/valentina> All Rights Reserved.
///
/// Valentina is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Valentina is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef VABSTRACTTOOL_H
#define VABSTRACTTOOL_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <Qt>
#include <QtGlobal>
#include <QPixmap>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vtools/visualization/visualization.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vdatatool.h"

class VGraphicsSimpleTextItem;
class VAbstractNode;

class VAbstractTool: public VDataTool
{
    Q_OBJECT
public:
    VAbstractTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent = nullptr);
    virtual                      ~VAbstractTool() override;
    quint32                       getId() const;

    static bool                   m_suppressContextMenu;

    static const QString          AttrInUse;

    static qreal                  CheckFormula(const quint32 &toolId, QString &formula, VContainer *data);

    static const QStringList      Colors();
    static QMap<QString, QString> ColorsList();
    static QMap<QString, QString> supportColorsList();
    static QMap<QString, QString> backgroundColorsList();
    static QPixmap                createColorIcon(const int w, const int h, const QString &color);
    static const QStringList      fills();

    static void                   AddRecord(const quint32 id, const Tool &toolType, VAbstractPattern *doc);
    static void                   addNodes(VAbstractPattern *doc, QDomElement &domElement, const VPiecePath &path);
    static void                   addNodes(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);

    const VContainer             *getData() const;

    QMap<QString, quint32>        PointsList() const;
    virtual QString               getTagName() const =0;
    virtual void                  ShowVisualization(bool show) =0;
    virtual void                  setPointNamePosition(quint32 id, const QPointF &pos);
    virtual void                  setPointNameVisiblity(quint32 id, bool visible);

    template<typename T>
    static quint32                CreateNode(VContainer *data, quint32 id);

public slots:
    virtual void                 FullUpdateFromFile()=0; /// @brief FullUpdateFromFile update tool data form file.
    virtual void                 AllowHover(bool enabled)=0;
    virtual void                 AllowSelecting(bool enabled)=0;
    virtual void                 ToolSelectionType(const SelectionType &type);

signals:
    void                         toolHasChanges(); /// @brief toolHasChanges emit if tool create change that need save

    /// @brief chosenTool emit if object was clicked.
    /// @param id object id in container.
    /// @param type type of scene object.
    void                         chosenTool(quint32 id, SceneObject type);
    /// @brief FullUpdateTree emit if need reparse pattern file.
    void                         LiteUpdateTree(const Document &parse);
    void                         ToolTip(const QString &toolTip);

protected:
    VAbstractPattern            *doc;           /// @brief doc dom document container
    const quint32                m_id;          /// @brief id object id.
    QPointer<Visualization>      vis;
    SelectionType                selectionType;
    virtual void                 AddToFile()=0; /// @brief AddToFile add tag with Information about tool into file.

    /// @brief RefreshDataInFile refresh attributes in file. If attributes don't exist create them.
    virtual void                 RefreshDataInFile();
    virtual void                 RemoveReferens() {} ///@brief RemoveReferens decrement value of reference.
    virtual void                 deleteTool(bool ask = true);
    static int                   ConfirmDeletion();

    template <typename T>
    void                         AddVisualization();

    virtual void                 SetVisualization()=0;
    virtual void                 ToolCreation(const Source &typeCreation);

    static QDomElement           AddSANode(VAbstractPattern *doc, const QString &tagName, const VPieceNode &node);
    static void                  AddNode(VAbstractPattern *doc, QDomElement &domElement, const VPieceNode &node);

    static QVector<VPieceNode>   PrepareNodes(const VPiecePath &path, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                             VContainer *data, const QString &bloack_name = QString());
    static quint32               PrepareNode(const VPieceNode &node, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                             VContainer *data, const QString &bloack_name = QString());
private:
    Q_DISABLE_COPY(VAbstractTool)
};

//---------------------------------------------------------------------------------------------------------------------
/// @brief getId return object id.
/// @return id.
//---------------------------------------------------------------------------------------------------------------------
inline quint32 VAbstractTool::getId() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getData return pointer to data container.
/// @return container.
//---------------------------------------------------------------------------------------------------------------------
inline const VContainer *VAbstractTool::getData() const
{
    return &data;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void VAbstractTool::AddVisualization()
{
    T *visual = new T(getData());
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
    SCASSERT(scene != nullptr)
    scene->addItem(visual);

    connect(visual, &Visualization::ToolTip, this, &VAbstractTool::ToolTip);
    vis = visual;
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief CreateNode create new node for detail.
///  @param data container.
///  @param id id parent object.
///  @return id for new object.
//---------------------------------------------------------------------------------------------------------------------
template<typename T>
quint32 VAbstractTool::CreateNode(VContainer *data, quint32 id)
{
    //We can't use exist object. Need create new.
    T *node = new T(*data->GeometricObject<T>(id).data());
    node->setMode(Draw::Modeling);
    return data->AddGObject(node);
}

#endif // VABSTRACTTOOL_H

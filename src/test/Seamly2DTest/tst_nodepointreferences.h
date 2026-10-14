//---------------------------------------------------------------------------------------------------------------------
//  @file   tst_nodepointreferences.h
//  @date   14 Oct, 2026
//
//  @copyright
//  Copyright (C)  2026 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

#ifndef TST_NODEPOINTREFERENCES_H
#define TST_NODEPOINTREFERENCES_H

#include <QObject>
#include <QVector>

class MeasurePattern;
class PatternPieceTool;
class VContainer;
class VMainGraphicsScene;
class VMainGraphicsView;

class TST_NodePointReferences : public QObject
{
    Q_OBJECT
public:
    explicit TST_NodePointReferences(QObject *parent = nullptr);

private slots:
    void init();
    void cleanup();
    void removeNodesAfterLoad();

private:
    Q_DISABLE_COPY(TST_NodePointReferences)

    MeasurePattern     *m_doc;
    VContainer         *m_data;
    VMainGraphicsScene *m_scene;
    VMainGraphicsView  *m_view;
    QVector<quint32>    m_base_points;

    quint32 addBasePoint(qreal x, qreal y, const QString &name);
    quint32 loadNodePoint(quint32 base_id);
    quint32 loadPiece(const QVector<quint32> &node_ids);
    void    logNodes(const QString &stage, const QVector<quint32> &node_ids) const;
    void    pushPieceWithout(quint32 piece_id, const QVector<quint32> &removed_ids);
};

#endif // TST_NODEPOINTREFERENCES_H

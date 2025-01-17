/***************************************************************************
 **  @file   union_dialog.h
 **  @author Douglas S Caskey
 **  @date   Dec 27, 2022
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
 **  @file   dialoguniondetails.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 12, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
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

#ifndef UNION_DIALOG_H
#define UNION_DIALOG_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QSound>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class UnionDialog;
}

/**
 * @brief The UnionDialog class dialog for Union Tool.
 */
class UnionDialog : public DialogTool
{
    Q_OBJECT
public:
                 UnionDialog(const VContainer *data, const quint32 &toolId,
                             QWidget *parent = nullptr);
    virtual     ~UnionDialog() Q_DECL_OVERRIDE;

    quint32      getPiece1Id() const;
    quint32      getPiece2Id() const;
    int          getPiece1Index() const;
    int          getPiece2Index() const;

    bool         retainPieces() const;

public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(UnionDialog)


    Ui::UnionDialog *ui;   /** @brief ui keeps information about user interface */
    int      piece1_Index; /** @brief piece1_Index index edge first piece */
    int      piece2_Index; /** @brief piece2_Index index edge second piece */
    quint32  d1;           /** @brief d1 id first piece */
    quint32  d2;           /** @brief d2 id second piece */
    qint32   numberD;      /** @brief numberD number of piece, what we already have */
    qint32   numberP;      /** @brief numberP number of points, what we already have */
    quint32  p1;           /** @brief p1 id first point of piece */
    quint32  p2;           /** @brief p2 id second point of piece */
    QSound  *m_beep;

    bool     CheckObject(const quint32 &id, const quint32 &pieceId) const;
    bool     checkPiece(const quint32 &pieceId) const;

    void     chosenPiece(const quint32 &id, const SceneObject &type, quint32 &pieceId,
                         int &index);
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPiece1Id return id first piece
 * @return id
 */
inline quint32 UnionDialog::getPiece1Id() const
{
    return d1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPiece2Id return id second piece
 * @return id
 */
inline quint32 UnionDialog::getPiece2Id() const
{
    return d2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPiece1Index return index edge first piece
 * @return index
 */
inline int UnionDialog::getPiece1Index() const
{
    return piece1_Index;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPiece2Index return index edge second piece
 * @return index
 */
inline int UnionDialog::getPiece2Index() const
{
    return piece2_Index;
}

#endif // UNION_DIALOG_H

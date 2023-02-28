/***************************************************************************
 **  @file   rename_draftblock.h
 **  @author Douglas S Caskey
 **  @date   Jan 1, 2023
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
 **  @file   renamepp.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 7, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Seamly2D Valentina
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#ifndef RENAME_DRAFTBLOCK_H
#define RENAME_DRAFTBLOCK_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class RenameDraftBlock :public VUndoCommand
{
    Q_OBJECT
public:
    RenameDraftBlock(VAbstractPattern *doc, const QString &newBlockName, QComboBox *combo, QUndoCommand *parent = nullptr);
    virtual     ~RenameDraftBlock() Q_DECL_OVERRIDE;

    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
    virtual bool mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    virtual int  id() const Q_DECL_OVERRIDE;
    QString      getNewName() const;
    QString      getOldName() const;
private:
    Q_DISABLE_COPY(RenameDraftBlock)
    QComboBox   *m_combo;
    QString      m_newBlockName;
    QString      m_oldBlockName;
    void         changeName(const QString &oldName, const QString &newName);
};

//---------------------------------------------------------------------------------------------------------------------
inline QString RenameDraftBlock::getNewName() const
{
    return m_newBlockName;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString RenameDraftBlock::getOldName() const
{
    return m_oldBlockName;
}

#endif // RENAME_DRAFTBLOCK_H

/***************************************************************************
 **  @file   set_piece_color.cpp
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
 **  GNU General Public License for more elements.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "set_piece_color.h"

#include <QDomElement>
#include <QHash>
#include <QMessageLogger>
#include <QtDebug>

#include "vundocommand.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vwidgets/vabstractmainwindow.h"

//---------------------------------------------------------------------------------------------------------------------
SetPieceColor::SetPieceColor(quint32 id, QString color, VContainer *data, VAbstractPattern *doc, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_id(id)
    , m_data(data)
    , m_oldColor(m_data->DataPieces()->value(m_id).getColor())
    , m_newColor(color)
{
    setText(tr("Change piece color"));
}

//---------------------------------------------------------------------------------------------------------------------
SetPieceColor::~SetPieceColor()
{
}

//---------------------------------------------------------------------------------------------------------------------
void SetPieceColor::undo()
{
    if (m_newColor != m_oldColor)
    {
        doCmd(m_oldColor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SetPieceColor::redo()
{
    if (m_newColor != m_oldColor)
    {
        doCmd(m_newColor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
int SetPieceColor::id() const
{
    return static_cast<int>(UndoCommand::SetPieceColor);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 SetPieceColor::getpieceId() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
QString SetPieceColor::getColor() const
{
    return m_newColor;
}

//---------------------------------------------------------------------------------------------------------------------
void SetPieceColor::doCmd(QString color)
{
    QDomElement element = doc->elementById(m_id, VAbstractPattern::TagPiece);
    if (element.isElement())
    {
        doc->SetAttribute(element, QStringLiteral("color"), color);

        VPiece piece = m_data->DataPieces()->value(m_id);
        piece.setColor(color);
        m_data->UpdatePiece(m_id, piece);

        emit updateList(m_id);

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        window->ShowToolTip(tr("Piece color changed: ") + color);
    }
    else
    {
        return;
    }
}

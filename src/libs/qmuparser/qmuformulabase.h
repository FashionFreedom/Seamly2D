//-------------------------------------------------------------------------------------------------
//  @file   qmuformulabase.h
//  @author Douglas S Caskey
//  @date   20 Jul, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2025 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//   Copyright (C) 2015 Roman Telezhynskyi
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy of this
//   software and associated documentation files (the "Software"), to deal in the Software
//   without restriction, including without limitation the rights to use, copy, modify,
//   merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//   permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in all copies or
//   substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//   DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------------------------------

#ifndef QMUFORMULABASE_H
#define QMUFORMULABASE_H

#include <qcompilerdetection.h>
#include <QString>
#include <QtGlobal>

#include "qmuparser.h"

template <class Key, class T> class QMap;

namespace qmu
{

class QmuFormulaBase : public QmuParser
{
public:
                  QmuFormulaBase();
    virtual      ~QmuFormulaBase() Q_DECL_OVERRIDE;

    virtual void  InitCharSets() Q_DECL_OVERRIDE;

    static void   RemoveAll(QMap<int, QString> &map, const QString &val);

protected:
    static qreal *AddVariable(const QString &a_szName, void *a_pUserData);
    void          SetSepForTr(bool osSeparator, bool fromUser);
    void          SetSepForEval();

private:
    Q_DISABLE_COPY(QmuFormulaBase)
};

} // namespace qmu

#endif // QMUFORMULABASE_H

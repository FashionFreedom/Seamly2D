// @file   vformula.h
// @author Douglas S Caskey
// @date   14 May, 2026
//
// @copyright
// Copyright (C) 2017 - 2026 Seamly, LLC
// https://github.com/fashionfreedom/seamly2d
//
// @brief
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

//-----------------------------------------------------------------------------
//
//  @file   vformula.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   28 Aug, 2014
//
//  @copyright
//  Copyright (C) 2014 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#ifndef VFORMULA_H
#define VFORMULA_H

#include <QCoreApplication>
#include <QMetaType>
#include <QString>
#include <QtGlobal>

enum class FormulaType : char{ToUser, FromUser};

class VContainer;

class VFormula
{
    Q_DECLARE_TR_FUNCTIONS(VFormula)
public:
    VFormula();
    VFormula(const QString &formula, const VContainer *container);
    VFormula &operator=(const VFormula &formula);
    VFormula(const VFormula &formula);
    bool operator==(const VFormula &formula) const;
    bool operator!=(const VFormula &formula) const;

    QString           GetFormula(FormulaType type = FormulaType::ToUser) const;
    void              SetFormula(const QString &text, FormulaType type = FormulaType::ToUser);

    QString           getStringValue() const;
    qreal             getDoubleValue() const;

    bool              getCheckZero() const;
    void              setCheckZero(bool value);

    const VContainer *getData() const;
    void              setData(const VContainer *value);

    quint32           getToolId() const;
    void              setToolId(const quint32 &value);

    QString           getPostfix() const;
    void              setPostfix(const QString &text);

    bool              error() const;

    static int        FormulaTypeId();
    void              Eval();

private:
    QString           m_formula;
    QString           m_valueStr;
    bool              m_checkZero;
    const VContainer *m_data;
    quint32           m_toolId;
    QString           m_postfix;
    bool              m_error;
    qreal             m_value;
};
Q_DECLARE_METATYPE(VFormula)

#endif // VFORMULA_H

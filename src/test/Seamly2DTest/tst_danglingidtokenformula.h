/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************/

#ifndef TST_DANGLINGIDTOKENFORMULA_H
#define TST_DANGLINGIDTOKENFORMULA_H

#include <QObject>

// PROBE for issue-#1678 code-quality review Finding 2: no test previously confirmed what
// happens when a formula's stored id token refers to a point that no longer exists (deleted
// after the formula was stored). This is a temporary, uncommitted probe left in the working
// tree per the review task - not yet decided whether a real fix is warranted.
class TST_DanglingIdTokenFormula : public QObject
{
    Q_OBJECT
public:
    explicit TST_DanglingIdTokenFormula(QObject *parent = nullptr);

private slots:
    void TestDanglingIdTokenLeftUntouchedByTranslation();
    void TestDanglingIdTokenFailsEvaluationGracefullyInsteadOfCrashing();

private:
    Q_DISABLE_COPY(TST_DanglingIdTokenFormula)
};

#endif // TST_DANGLINGIDTOKENFORMULA_H

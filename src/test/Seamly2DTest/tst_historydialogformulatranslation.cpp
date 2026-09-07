/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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

#include "tst_historydialogformulatranslation.h"

#include <QtTest>

#include "../../libs/ifc/xml/vdomdocument.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_HistoryDialogFormulaTranslation::TST_HistoryDialogFormulaTranslation(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reproduces HistoryDialog::formulaValue()'s exact read/translate sequence - GetParametrString
 * followed by FormulaIdTranslator::formulaIdsToNames(..., PatternFormulaTokens::idTokenToNameMap(data))
 * - against a pattern element whose formula attribute is stored the way #1678 stores it: keyed by
 * object id, not by name.
 *
 * Before the fix, HistoryDialog read the attribute with GetParametrString() alone and skipped the
 * translation call entirely, so the history table showed the raw stored token (e.g. "id3001*2")
 * instead of the current object name (e.g. "A1*2"). This guards against that regression.
 */
void TST_HistoryDialogFormulaTranslation::TestFormulaAttributeIsTranslatedFromIdTokenToName()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3001;

    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    VDomDocument doc;
    QDomElement domElement = doc.createElement(QStringLiteral("line"));
    domElement.setAttribute(QStringLiteral("length"), QStringLiteral("id%1*2").arg(id1));

    const QString storedFormula = doc.GetParametrString(domElement, QStringLiteral("length"), QString());
    QCOMPARE(storedFormula, QStringLiteral("id%1*2").arg(id1));

    const QString displayedFormula = formulaIdsToNames(storedFormula, idTokenToNameMap(data.data()));
    QCOMPARE(displayedFormula, QStringLiteral("A1*2"));

    // The stored attribute itself must stay in id-token form - only the value shown to the user changes.
    QCOMPARE(doc.GetParametrString(domElement, QStringLiteral("length"), QString()),
             QStringLiteral("id%1*2").arg(id1));
}

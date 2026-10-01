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

#include "tst_splinetargetlengthformulapropagation.h"

#include <QtTest>
#include <type_traits>
#include <utility>

#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"

namespace
{
/*
 * Background: VToolSpline::Create() (the overload vpattern.cpp's ParseToolSpline() calls while
 * loading a file) receives all five spline formulas - angle1, angle2, length1, length2 and the
 * "curve length" target formula - and runs each one through VAbstractTool::CheckFormula(). If a
 * formula turns out to be broken, CheckFormula() shows the user a repair dialog and, once fixed,
 * writes the corrected text back through its "QString &formula" out-parameter - that only works
 * for a caller whose own variable was passed in by reference in the first place.
 *
 * angle1/angle2/length1/length2 are declared "QString &" (mutable reference) for exactly this
 * reason - a fix reaches the caller. Before this fix, targetLength was the only one of the five
 * declared "const QString &": a mutable reference cannot bind to a temporary, but a const
 * reference can, which is precisely the observable difference these two detectors probe. Because
 * of that one different declaration, any repair CheckFormula() made to a broken target-length
 * formula was silently thrown away instead of reaching VPattern::ParseToolSpline() - it never
 * made it into the saved file, and not even into the freshly constructed VToolSpline object's own
 * stored formula.
 *
 * A real end-to-end reproduction (load a file, let the repair dialog pop up, retype the formula,
 * save, reload) needs the interactive dialog CheckFormula() shows, which this test binary can
 * never trigger: VAbstractApplication::isAppInGUIMode() is hard-wired to false for every test
 * (see TestApplication2D in qttestmainlambda.cpp), and the dialog it would otherwise show
 * dereferences qApp->translateVariables(), which TestApplication2D also hard-wires to nullptr.
 * So instead, this test pins down the exact, narrower defect the bug actually lived in: the
 * *type* of the targetLength parameter, which is what made the repaired formula unreachable in
 * the first place, and is something a normal, headless unit test can observe directly, without
 * a GUI, a scene, a document or a pattern container at all - a value of type T() is never
 * actually constructed at runtime; decltype() only asks the compiler whether the call *would*
 * type-check, so this is not evaluated code, no VToolSpline instance is ever created.
 *
 * canBindRvalueForA1<T>(0) resolves to std::true_type if a temporary of type T can be passed as
 * VToolSpline::Create()'s "a1" argument, and to std::false_type otherwise (via the "..." fallback
 * SFINAE removes the first overload to when the call does not type-check). canBindRvalueFor-
 * TargetLength<T>(0) does the exact same probe, but for the "targetLength" argument instead.
 */
template <typename T>
auto canBindRvalueForA1(int)
    -> decltype(VToolSpline::Create(quint32(0), quint32(0), quint32(0),
                                    T(), std::declval<QString &>(),
                                    std::declval<QString &>(), std::declval<QString &>(),
                                    quint32(0), QString(), QString(), QString(),
                                    static_cast<VMainGraphicsScene *>(nullptr),
                                    static_cast<VAbstractPattern *>(nullptr),
                                    static_cast<VContainer *>(nullptr),
                                    Document::FullParse, Source::FromFile,
                                    false, 0, std::declval<QString &>()),
                std::true_type());

template <typename T>
std::false_type canBindRvalueForA1(...);

template <typename T>
auto canBindRvalueForTargetLength(int)
    -> decltype(VToolSpline::Create(quint32(0), quint32(0), quint32(0),
                                    std::declval<QString &>(), std::declval<QString &>(),
                                    std::declval<QString &>(), std::declval<QString &>(),
                                    quint32(0), QString(), QString(), QString(),
                                    static_cast<VMainGraphicsScene *>(nullptr),
                                    static_cast<VAbstractPattern *>(nullptr),
                                    static_cast<VContainer *>(nullptr),
                                    Document::FullParse, Source::FromFile,
                                    false, 0, T()),
                std::true_type());

template <typename T>
std::false_type canBindRvalueForTargetLength(...);
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
TST_SplineTargetLengthFormulaPropagation::TST_SplineTargetLengthFormulaPropagation(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves VToolSpline::Create()'s targetLength parameter is a mutable reference, exactly
 * like a1/a2/l1/l2, so a formula CheckFormula() repairs can propagate back to the caller
 * (VPattern::ParseToolSpline) instead of being silently discarded. See the anonymous namespace
 * above for why this is checked at the type level rather than by actually triggering a repair.
 */
void TST_SplineTargetLengthFormulaPropagation::TestCreateAcceptsTargetLengthOnlyAsAMutableReference()
{
    // Control: a1 has always been declared "QString &". A mutable reference cannot bind to a
    // temporary, so this must be false. If it were ever true, the detector itself would be
    // broken and the assertion below could not be trusted.
    QVERIFY2(!decltype(canBindRvalueForA1<QString>(0))::value,
             "Sanity check failed: VToolSpline::Create()'s a1 parameter unexpectedly accepts a "
             "temporary QString. It is declared 'QString &' and always was, so binding a "
             "temporary to it must be ill-formed - if this assertion fails, the SFINAE detector "
             "itself is broken and the targetLength assertion below cannot be trusted either.");

    // The actual regression: before the fix, targetLength was declared "const QString &" - the
    // only one of the five spline formula parameters not declared as a mutable reference - so it
    // accepted a temporary just fine. That is exactly why a repair CheckFormula() made to a
    // broken target-length formula never reached the caller: VToolSpline::Create() copied
    // targetLength into a throwaway local before repairing it, then discarded the local and
    // handed the ORIGINAL, still-broken text to both the DOM (VPattern::ParseToolSpline never
    // wrote AttrLength back either) and the newly constructed VToolSpline's own stored formula.
    QVERIFY2(!decltype(canBindRvalueForTargetLength<QString>(0))::value,
             "VToolSpline::Create()'s targetLength parameter accepts a temporary QString, meaning "
             "it is declared 'const QString &' instead of 'QString &'. Because of this, any fix "
             "CheckFormula() makes to a broken target-length formula (typed into the formula-"
             "repair dialog) is silently discarded instead of reaching the caller "
             "(VPattern::ParseToolSpline) and the constructed VToolSpline object itself - unlike "
             "the angle1/angle2/length1/length2 formulas, which are already mutable references "
             "and do carry a repaired formula back.");
}

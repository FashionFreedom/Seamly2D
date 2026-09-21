//-----------------------------------------------------------------------------
//  @file   qttestmainlambda.cpp
//  @author Douglas S Caskey
//  @date   13 July, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   qttestmainlambda.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date    31 3, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include <QtTest>

#include "tst_vposter.h"
#include "tst_vabstractpiece.h"
#include "tst_vspline.h"
#include "tst_nameregexp.h"
#include "tst_vlayoutdetail.h"
#include "tst_varc.h"
#include "tst_vellipticalarc.h"
#include "tst_qmutokenparser.h"
#include "tst_vmeasurements.h"
#include "tst_vlockguard.h"
#include "tst_misc.h"
#include "tst_vcommandline.h"
#include "tst_vpiece.h"
#include "tst_findpoint.h"
#include "tst_vabstractcurve.h"
#include "tst_vcubicbezierpath.h"
#include "tst_vgobject.h"
#include "tst_vsplinepath.h"
#include "tst_vpointf.h"
#include "tst_readval.h"
#include "tst_vtranslatevars.h"
#include "tst_vtoolmove.h"

#include "../vmisc/def.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/projectversion.h"

class TestApplication2D : public VAbstractApplication
{
public:

                                  TestApplication2D(int &argc, char ** argv);
    virtual                      ~TestApplication2D() Q_DECL_EQ_DEFAULT;

    virtual const VTranslateVars *translateVariables();
    virtual void                  openSettings();
    virtual bool                  isAppInGUIMode() const;
    virtual void                  initTranslateVariables();
};

//---------------------------------------------------------------------------------------------------------------------
TestApplication2D::TestApplication2D(int &argc, char **argv)
    : VAbstractApplication(argc, argv)
{
    setApplicationName(VER_INTERNALNAME_2D_STR);
    setOrganizationName(VER_COMPANYNAME_STR);
    openSettings();
}

//---------------------------------------------------------------------------------------------------------------------
const VTranslateVars *TestApplication2D::translateVariables()
{
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void TestApplication2D::openSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

    const QString qt5Settings = settings.fileName();
    const QString dir = QFileInfo(qt5Settings).absolutePath();
    const QString qt5Common   = dir + "/common.ini";
    const QString qt6Settings = dir + "/qt6_seamly2d.ini";
    const QString qt6Common   = dir + "/qt6_common.ini";

    if (!QFileInfo::exists(qt6Common) && QFileInfo::exists(qt5Common))
    {
        QFile::copy(qt5Common, qt6Common);
    }

    if (!QFileInfo::exists(qt6Settings) && QFileInfo::exists(qt5Settings))
    {
        QFile::copy(qt5Settings, qt6Settings);
    }

    m_settings = new VSettings(qt6Settings, QSettings::IniFormat, this);
}

//---------------------------------------------------------------------------------------------------------------------
bool TestApplication2D::isAppInGUIMode() const
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void TestApplication2D::initTranslateVariables()
{
}

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
    Q_INIT_RESOURCE(schema);

    TestApplication2D app( argc, argv );// For QPrinter

    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject* obj)
    {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };

    ASSERT_TEST(new TST_FindPoint());
    ASSERT_TEST(new TST_VPiece());
    ASSERT_TEST(new TST_VPoster());
    ASSERT_TEST(new TST_VAbstractPiece());
    ASSERT_TEST(new TST_VSpline());
    ASSERT_TEST(new TST_VSplinePath());
    ASSERT_TEST(new TST_NameRegExp());
    ASSERT_TEST(new TST_VLayoutPiece());
    ASSERT_TEST(new TST_VArc());
    ASSERT_TEST(new TST_VEllipticalArc());
    ASSERT_TEST(new TST_QmuTokenParser());
    ASSERT_TEST(new TST_Measurements());
    ASSERT_TEST(new TST_VLockGuard());
    ASSERT_TEST(new TST_Misc());
    ASSERT_TEST(new TST_VCommandLine());
    ASSERT_TEST(new TST_VAbstractCurve());
    ASSERT_TEST(new TST_VCubicBezierPath());
    ASSERT_TEST(new TST_VGObject());
    ASSERT_TEST(new TST_VPointF());
    ASSERT_TEST(new TST_ReadVal());
    ASSERT_TEST(new TST_VTranslateVars());
    ASSERT_TEST(new TST_VToolMove());

    return status;
}

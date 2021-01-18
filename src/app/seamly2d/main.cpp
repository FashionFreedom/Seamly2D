/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   main.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
 *************************************************************************/

#include "mainwindow.h"
#include "core/vapplication.h"
#include "../fervor/fvupdater.h"
#include "../vpatterndb/vpiecenode.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(cursor);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(schema);
    Q_INIT_RESOURCE(theme);
    Q_INIT_RESOURCE(flags);
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(toolicon);

    QT_REQUIRE_VERSION(argc, argv, "5.2.0")

    // Need to internally move a node inside a piece main path
    qRegisterMetaTypeStreamOperators<VPieceNode>("VPieceNode");
//------------------------------------------------------------------------
#include <QOperatingSystemVersion>
QOperatingSystemVersion os_ver = QOperatingSystemVersion::current();
int macos_major_ver = os_ver.majorVersion();
int macos_minor_ver = os_ver.minorVersion();
bool macos_affected = (macos_major_ver == 10 && macos_minor_ver >= 16 || \
                      macos_major_ver >= 11);
bool qtver_affected = (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0) && \
                      QT_VERSION < QT_VERSION_CHECK(5, 12, 11) || \
                      QT_VERSION >= QT_VERSION_CHECK(5, 15, 0) && \
                      QT_VERSION < QT_VERSION_CHECK(5, 15, 3) || \
                      QT_VERSION < QT_VERSION_CHECK(6, 0, 0));
bool qtenvvar_empty = qgetenv("QT_MAC_WANTS_LAYER").isEmpty();
int macosver_qtver_qtenvvar_affected = (macos_affected && qtver_affected && qtenvvar_empty);

#ifndef Q_OS_MAC // supports natively
    InitHighDpiScaling(argc, argv);
    // handle Qt issue with MacOS Big Sur https://bugreports.qt.io/browse/QTBUG-87014
    // Based on Wireshark's fix
    //    https://gitlab.com/wireshark/wireshark/-/commit/2428285d44939ad0054df329a9c2a109ed37c877
    // TODO - move this to def.cpp and call as a function
    // As of Jan 17, 2021 the QTBUG-87014 is...
    // ...in https://github.com/qt/qtbase/blob/5.12/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // ...not in https://github.com/qt/qtbase/blob/5.12.10/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // ...in https://github.com/qt/qtbase/blob/5.15/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // ...not in https://github.com/qt/qtbase/blob/5.15.2/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // ...not in https://github.com/qt/qtbase/blob/6.0/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // ...not in https://github.com/qt/qtbase/blob/6.0.0/src/plugins/platforms/cocoa/qnsview_drawing.mm
    // We'll assume that it will be fixed in 5.12.11, 5.15.3, and 6.0.1.
    #if defined(macosver_qtver_qtenvvar_affected)
        qputenv("QT_MAC_WANTS_LAYER", QByteArray("1"));
    #endif
#endif //Q_OS_MAC

    VApplication app(argc, argv);

    app.InitOptions();

    // Due to unknown reasons version checker cause a crash. See issue #633.
    // Before we will find what cause such crashes it will stay disabled in Release mode.
#ifndef V_NO_ASSERT
    if (VApplication::IsGUIMode())
    {
        // Set feed URL before doing anything else
        FvUpdater::sharedUpdater()->SetFeedURL(defaultFeedURL);

        // Check for updates automatically
        FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
    }
#endif // V_NO_ASSERT

    MainWindow w;
#if !defined(Q_OS_MAC)
    app.setWindowIcon(QIcon(":/icon/64x64/icon64x64.png"));
#endif // !defined(Q_OS_MAC)
    app.setMainWindow(&w);

    int msec = 0;
    //Before we load pattern show window.
    if (VApplication::IsGUIMode())
    {
        w.show();
        msec = 15; // set delay for correct the first fitbest zoom
    }

    QTimer::singleShot(msec, &w, SLOT(ProcessCMD()));

    return app.exec();
}

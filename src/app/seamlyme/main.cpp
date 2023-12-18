/***************************************************************************
 **  @file   mapplication.cpp
 **  @author Douglas S Caskey
 **  @date   28 Nov, 2023
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
 **  @file   main.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

/*
 * @brief entry point of SeamlyMe application
 * @return non-zero value is code of the error
 */

#include "tmainwindow.h"
#include "mapplication.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(seamlymeicon);
    Q_INIT_RESOURCE(theme);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(schema);
    Q_INIT_RESOURCE(flags);

    QT_REQUIRE_VERSION(argc, argv, "5.15.2");

    //------------------------------------------------------------------------
    // On macOS, correct WebView / QtQuick compositing and stacking requires running
    // Qt in layer-backed mode, which again requires rendering on the Gui thread.
    qWarning("SeamlyME: Setting QT_MAC_WANTS_LAYER=1 and QSG_RENDER_LOOP=basic");
    qputenv("QT_MAC_WANTS_LAYER", "1");
    //------------------------------------------------------------------------

#ifndef Q_OS_MAC // supports natively
    initHighDpiScaling(argc, argv);
#endif //Q_OS_MAC

    MApplication app(argc, argv);
    app.InitOptions();

    QPixmap pixmap(":/splash/coming_soon.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.show();

    QTimer::singleShot(10000, &splash, &QWidget::close);                 // display splash screenfor 10 secs.

    QTimer::singleShot(10000, &app, &MApplication::processCommandLine);  // delay main screen from opening until
                                                                         // splash screen closes.
    return app.exec();
}

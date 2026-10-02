///---------------------------------------------------------------------------------------------------------------------
/// @file   main.cpp
/// @author Douglas S Caskey
/// @date   31 Dec, 2023
///
/// @brief
/// @copyright
/// This source code is part of the Seamly2D project, a pattern making
/// program to create and model patterns of clothing.
/// Copyright (C) 2017-2026 Seamly2D project
/// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
///
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
///  @file   main.cpp
///  @author Roman Telezhynskyi <dismine(at)gmail.com>
///  @date   10 7, 2015
///
///  @brief
///  @copyright
///  This source code is part of the Valentina project, a pattern making
///  program, whose allow create and modeling patterns of clothing.
///  Copyright (C) 2015 Valentina project
///  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
///
///  Valentina is free software: you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation, either version 3 of the License, or
///  (at your option) any later version.
///
///  Valentina is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///  You should have received a copy of the GNU General Public License
//// along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
///---------------------------------------------------------------------------------------------------------------------

/// @brief entry point of SeamlyMe application
/// @return non-zero value is code of the error

#include "tmainwindow.h"
#include "application_me.h"
#include "dialogs/me_welcome_dialog.h"
#include "../vmisc/vseamlymesettings.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QTimer>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(seamlymeicon);
    Q_INIT_RESOURCE(theme);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(schema);
    Q_INIT_RESOURCE(flags);
    Q_INIT_RESOURCE(diagrams);

    // Check if the Qt version is at least 6.2.4
    QT_REQUIRE_VERSION(argc, argv, "6.2.4");

    ApplicationME app(argc, argv);
    app.initOptions();

    // Only show welcome dialog if in test mode
    if (!app.arguments().contains("--test"))
    {
        // Retrieve the SeamlyMe application settings
        auto settings = qApp->seamlyMeSettings();

        // The 'showWelcome' setting indicates whether to show the welcome dialog
        // 'true' means "do not show welcome again", so we invert it here
        bool showWelcome = !settings->getShowWelcome();

        if (showWelcome)
        {
            SeamlyMeWelcomeDialog *dialog = new SeamlyMeWelcomeDialog();
            dialog->setAttribute(Qt::WA_DeleteOnClose, true);
            dialog->exec();
            // Load translations based on the locale setting
            app.loadTranslations(settings->getLocale());
        }
    }

    QTimer::singleShot(0, &app, &ApplicationME::processCommandLine);

    return app.exec();
}

//---------------------------------------------------------------------------------------------------------------------
/// @file   command_line.h
/// @author Douglas S Caskey
/// @date   14 Nov, 2026
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
//
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
/// @file   vcmdexport.h
/// @author Alex Zaharov <alexzkhr@gmail.com>
/// @date   25 8, 2015
///
/// @brief
/// @copyright
/// This source code is part of the Valentine project, a pattern making
/// program, whose allow create and modeling patterns of clothing.
/// Copyright (C) 2015 Seamly2D project
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
//---------------------------------------------------------------------------------------------------------------------

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <memory>
#include <vector>
#include <QTextStream>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QString>

#include "../dialogs/layoutsettings_dialog.h"
#include "../vmisc/def.h"
#include "../vmisc/vsysexits.h"

class VCommandLine;
typedef std::shared_ptr<VCommandLine> VCommandLinePtr;
typedef QList<QCommandLineOption *> VCommandLineOptions;
typedef std::shared_ptr<VLayoutGenerator> VLayoutGeneratorPtr;

class VCommandLine
{
public:
    virtual               ~VCommandLine();

    bool                   isTestModeEnabled() const;
    bool                   isNoScalingEnabled() const;
    bool                   isExportEnabled() const;
    QString                measurementPathOption() const;
    QString                baseNameOption() const;
    QString                destinationPathOption() const;
    int                    exportTypeOption() const;
    int                    isBinaryDXF() const;
    int                    isTextAsPaths() const;
    int                    exportOnlyPieces() const;
    VLayoutGeneratorPtr    defaultlayoutGenerator() const;
    QStringList            inputFileNamesOption() const;
    bool                   isGuiEnabled()const;
    bool                   isGradationSizeSet() const;
    bool                   isGradationHeightSet() const;
    QString                gradationSizeOption() const;
    QString                gradationHeightOption() const;

protected:
                           VCommandLine();

    PaperSizeFormat        paperSizeOption() const;
    int                    rotationOption() const;
    Cases                  groupOption() const;
    static void            reset();
    static VCommandLinePtr getCommandLine(const QCoreApplication& app);

private:
                           Q_DISABLE_COPY(VCommandLine)
    static VCommandLinePtr command_line;
    QCommandLineParser     parser;
    VCommandLineOptions    m_options_used;
    QMap<QString, int>     m_options_index;
    bool                   m_is_gui_enabled;
    friend class           Application2D;

    [[noreturn]] void      showHelp(int exitCode) const;
    static qreal           layoutToPx(const QString& src,
                                 const LayoutSettingsDialog& converter);

    static qreal           pageToPx(const QString& src,
                                 const LayoutSettingsDialog& converter);

    static void            initOptions(VCommandLineOptions &options,
                                       QMap<QString, int> &m_options_index);
};

#endif // COMMAND_LINE_H

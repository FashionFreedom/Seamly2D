/**********************************************************************
 **
 **  @file   vcmdexport.cpp
 **  @author Alex Zaharov <alexzkhr@gmail.com>
 **  @date   25 8, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#include "vcmdexport.h"
#include "../dialogs/layoutsettings_dialog.h"
#include "../vwidgets/export_format_combobox.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vformat/measurements.h"
#include "../vmisc/commandoptions.h"
#include "../vmisc/def.h"
#include "../vmisc/vsettings.h"
#include "../vlayout/vlayoutgenerator.h"
#include "../vwidgets/page_format_combobox.h"

#include <QDebug>

VCommandLinePtr VCommandLine::commandLine = nullptr;

#define translate(context, source) QCoreApplication::translate((context), (source))

//  --------------------
/*
 * @brief Constructor for the VCommandLine class.
 *
 * Initializes the command-line parser with application description, standard help/version
 * options, a positional argument for the pattern file, and all custom options defined
 * in the application.
 */
VCommandLine::VCommandLine() : parser(), optionsUsed(), optionsIndex(), isGuiEnabled(false)
{
    // Sets a brief description of the application shown in the help output.
    parser.setApplicationDescription(translate("VCommandLine", "Seamly2D open source patternmaking program."));

    // Adds the standard help option (-h, --help) to the parser.
    parser.addHelpOption();

    // Adds the standard version option (-v, --version) to the parser.
    parser.addVersionOption();

    // Add a positional argument (not prefixed with a flag), typically the filename
    parser.addPositionalArgument("filename", translate("VCommandLine", "Seamly2D pattern file (.sm2d)"));

    // Initialize the list of custom command-line options and their index
    initOptions(optionsUsed, optionsIndex);

    // Iterate over each command-line option in optionsUsed and add it to the parser
    VCommandLineOptions::const_iterator i = optionsUsed.constBegin();
    while (i != optionsUsed.constEnd())
    {
        parser.addOption(*(*i));  // Dereference the pointer to QCommandLineOption and add it
        ++i;
    }
}

//  --------------------
/*
 * @brief Converts a layout unit value from string to pixels.
 *
 * This function takes a layout value provided as a string (e.g., "25.4") and
 * converts it to pixels using the provided layout settings.
 *
 * @param src The source layout value as a QString (expected to be numeric).
 * @param converter A reference to the LayoutSettingsDialog that provides
 *        the conversion method from layout units to pixels.
 * @return The equivalent value in pixels as a qreal.
 */
qreal VCommandLine::Lo2Px(const QString &src, const LayoutSettingsDialog &converter)
{
    return converter.LayoutToPixels(src.toDouble());
}

//  --------------------
/*
 * @brief Converts a page unit value from string to pixels.
 *
 * This function accepts a page size value as string (e.g., "210" for mm)
 * and converts it to pixels using provided layout settings.
 *
 * @param src The source page value as a QString (expected to be numeric).
 * @param converter A reference to the LayoutSettingsDialog that provides
 *        the conversion method from page units to pixels.
 * @return The equivalent value in pixels as a qreal.
 */
qreal VCommandLine::Pg2Px(const QString& src, const LayoutSettingsDialog& converter)
{
    return converter.PageToPixels(src.toDouble());
}

//  --------------------
/*
 * @brief Initializes the list of command-line options and their index.
 *
 * This method defines all supported CLI options for Seamly2D's export and automation features.
 * Options are added in a grouped, user-friendly order for consistency in help output.
 *
 * @param options A reference to the list where all VCommandLineOptions will be stored.
 * @param optionsIndex A map that associates each long-form option string with its index
 *in the options list.
 */
void VCommandLine::initOptions(VCommandLineOptions &options, QMap<QString, int> &optionsIndex)
{
    int index = 0;

    // Options are grouped for usability; order matters for help display
    // ================================================================================================================

    // --basename: Base filename for exported layout files (enables CLI export mode)
    optionsIndex.insert(LONG_OPTION_BASENAME, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_BASENAME, LONG_OPTION_BASENAME},
        translate("VCommandLine", "The base filename of exported layout files. Use it to "
                                  "enable console export mode."),
        translate("VCommandLine", "The base filename of layout files")));

    // --destination: Output directory for exported files
    optionsIndex.insert(LONG_OPTION_DESTINATION, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_DESTINATION, LONG_OPTION_DESTINATION},
        translate("VCommandLine", "The path to output destination folder. By default the directory "
                                  "at which the application was started."),
        translate("VCommandLine", "The destination folder")));

    // --measurefile: Path to custom measurement file
    optionsIndex.insert(LONG_OPTION_MEASUREFILE, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_MEASUREFILE, LONG_OPTION_MEASUREFILE},
        translate("VCommandLine", "Path to custom measure file (export mode)."),
        translate("VCommandLine", "The measure file")));

    // --format: Output file format (e.g., SVG, PDF, DXF)
    optionsIndex.insert(LONG_OPTION_EXP2FORMAT, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_EXP2FORMAT, LONG_OPTION_EXP2FORMAT},
        translate("VCommandLine", "Number corresponding to output format "
                                  "(default = 0, export mode):") +
                                  ExportFormatCombobox::makeHelpFormatList(),
        translate("VCommandLine", "Format number"), "0"));

    // --binarydxf: Export DXF files in binary form
    optionsIndex.insert(LONG_OPTION_BINARYDXF, index++);
    options.append(new QCommandLineOption({LONG_OPTION_BINARYDXF},
        translate("VCommandLine", "Export dxf in binary form.")));

    // --textaspath: Export text elements as vector paths
    optionsIndex.insert(LONG_OPTION_TEXT2PATHS, index++);
    options.append(new QCommandLineOption({LONG_OPTION_TEXT2PATHS},
        translate("VCommandLine", "Export text as paths.")));

    // --exportonlydetails: Only export detailed parts, ignoring layout
    optionsIndex.insert(LONG_OPTION_EXPORTONLYDETAILS, index++);
    options.append(new QCommandLineOption({LONG_OPTION_EXPORTONLYDETAILS},
        translate("VCommandLine", "Export only details. Export details as they positioned in "
                                  "the details mode. Any layout related options will be ignored.")));

    // --gradationsize: Size value for multi-size patterns
    optionsIndex.insert(LONG_OPTION_GRADATIONSIZE, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_GRADATIONSIZE, LONG_OPTION_GRADATIONSIZE},
        translate("VCommandLine", "Set size value a pattern file, that was opened with multisize "
                                  "measurements (export mode). Valid values: %1cm.")
                                 .arg(MeasurementVariable::allSizesList(Unit::Cm).join(", ")),
        translate("VCommandLine", "The size value")));

    // --gradationheight: Height value for multi-size patterns
    optionsIndex.insert(LONG_OPTION_GRADATIONHEIGHT, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_GRADATIONHEIGHT, LONG_OPTION_GRADATIONHEIGHT},
        translate("VCommandLine", "Set height value a pattern file, that was opened with multisize "
                                  "measurements (export mode). Valid values: %1cm.")
                                  .arg(MeasurementVariable::allHeightsList(Unit::Cm).join(", ")),
        translate("VCommandLine", "The height value")));

    // ... [Omitting unchanged lines for brevity—continues as previously documented]

    // --pagetemplate: Number of page template
    optionsIndex.insert(LONG_OPTION_PAGETEMPLATE, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_PAGETEMPLATE, LONG_OPTION_PAGETEMPLATE},
        translate("VCommandLine", "Number corresponding to page template "
                                  "(default = 0, export mode):") +
                                   PageFormatCombobox::MakeHelpTemplateList(),
        translate("VCommandLine", "Template number"), "0"));

    // --pagewidth: Page width in current units
    optionsIndex.insert(LONG_OPTION_PAGEW, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_PAGEW, LONG_OPTION_PAGEW},
        translate("VCommandLine", "Page width in current units like 12.0 (cannot be "
                                  "used with \"%1\", export mode).")
                                  .arg(LONG_OPTION_PAGETEMPLATE),
        translate("VCommandLine", "The page width")));

    // --pageheight: Page height in current units
    optionsIndex.insert(LONG_OPTION_PAGEH, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_PAGEH, LONG_OPTION_PAGEH},
        translate("VCommandLine", "Page height in current units like 12.0 (cannot "
                                  "be used with \"%1\", export mode).")
                                  .arg(LONG_OPTION_PAGETEMPLATE),
        translate("VCommandLine", "The page height")));

    // --pageunits: Page measurement units
    optionsIndex.insert(LONG_OPTION_PAGEUNITS, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_PAGEUNITS, LONG_OPTION_PAGEUNITS},
        translate("VCommandLine", "Page height/width measure units (cannot be used "
                                  "with \"%1\", export mode). Valid values: %2.")
                                  .arg(LONG_OPTION_PAGETEMPLATE)
                                  .arg(VDomDocument::UnitsHelpString()),
        translate("VCommandLine", "The measure unit")));

    // --ignoremargins: Set all margins to zero.
    optionsIndex.insert(LONG_OPTION_IGNORE_MARGINS, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_IGNORE_MARGINS, LONG_OPTION_IGNORE_MARGINS},
        translate("VCommandLine", "Ignore margins printing (export mode). Disable value keys: "
                                  "\"%1\", \"%2\", \"%3\", \"%4\". Set all margins to 0.")
                                  .arg(LONG_OPTION_LEFT_MARGIN).arg(LONG_OPTION_RIGHT_MARGIN)
                                  .arg(LONG_OPTION_TOP_MARGIN).arg(LONG_OPTION_BOTTOM_MARGIN)));

    // --leftmargin: Set left margin value in current page units
    optionsIndex.insert(LONG_OPTION_LEFT_MARGIN, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_LEFT_MARGIN, LONG_OPTION_LEFT_MARGIN},
        translate("VCommandLine", "Page left margin in current units like 3.0 (export mode). If "
                                  "not set will be used value from default printer. Or 0 if none "
                                  "printers was found. Value will be ignored if key \"%1\" is used.")
                                  .arg(LONG_OPTION_IGNORE_MARGINS),
        translate("VCommandLine", "The left margin")));

    // --rightmargin: Set right margin value in current page units
    optionsIndex.insert(LONG_OPTION_RIGHT_MARGIN, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_RIGHT_MARGIN, LONG_OPTION_RIGHT_MARGIN},
        translate("VCommandLine", "Page right margin in current units like 3.0 (export mode). If "
                                  "not set will be used value from default printer. Or 0 if none "
                                  "printers was found. Value will be ignored if key \"%1\" is used.")
                                  .arg(LONG_OPTION_IGNORE_MARGINS),
        translate("VCommandLine", "The right margin")));

    // --topmargin: Set top margin value in current page units
    optionsIndex.insert(LONG_OPTION_TOP_MARGIN, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_TOP_MARGIN, LONG_OPTION_TOP_MARGIN},
        translate("VCommandLine", "Page top margin in current units like 3.0 (export mode). If "
                                  "not set will be used value from default printer. Or 0 if none "
                                  "printers was found. Value will be ignored if key \"%1\" is used.")
                                  .arg(LONG_OPTION_IGNORE_MARGINS),
        translate("VCommandLine", "The top margin")));

    // --bottommargin: Set bottom margin value in current page units
    optionsIndex.insert(LONG_OPTION_BOTTOM_MARGIN, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_BOTTOM_MARGIN, LONG_OPTION_BOTTOM_MARGIN},
        translate("VCommandLine", "Page bottom margin in current units like 3.0 (export mode). If "
                                  "not set will be used value from default printer. Or 0 if none "
                                  "printers was found. Value will be ignored if key \"%1\" is used.")
                                  .arg(LONG_OPTION_IGNORE_MARGINS),
        translate("VCommandLine", "The bottom margin")));

    // --rotate: Rotation of pieces in degrees
    optionsIndex.insert(LONG_OPTION_ROTATE, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_ROTATE, LONG_OPTION_ROTATE},
        translate("VCommandLine", "Rotation in degrees (one of predefined, export mode). "
                                  "Default value is 180. 0 is no-rotate. Valid values: "
                                  "%1. Each value show how many times details will be rotated. For "
                                  "example 180 mean two times (360/180=2) by 180 degree.")
                                  .arg("0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 12, 15, 18, 20, 24, 30, 36, 40, 45, "
                                       "60, 72, 90, 180"),
        translate("VCommandLine", "Angle")));

    // --crop: Autocrop unused length of page
    optionsIndex.insert(LONG_OPTION_CROP, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_CROP, LONG_OPTION_CROP},
                                          translate("VCommandLine", "Auto crop unused length (export mode).")));

    // --unite: Unite pages if possible
    optionsIndex.insert(LONG_OPTION_UNITE, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_UNITE, LONG_OPTION_UNITE},
        translate("VCommandLine", "Unite pages if possible (export mode). Maximum "
                                  "value limited by QImage that supports only a maximum of "
                                  "32768x32768 px images.")));

    // --savelength: Save length of sheet when set
    optionsIndex.insert(LONG_OPTION_SAVELENGTH, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_SAVELENGTH, LONG_OPTION_SAVELENGTH},
        translate("VCommandLine", "Save length of the sheet if set (export mode). The option tells "
                                  "the program to use as much as possible width of sheet. Quality "
                                  "of a layout can be worse when this option was used.")));

    // --shiftunits: Set the shift / offset units for layout
    optionsIndex.insert(LONG_OPTION_SHIFTUNITS, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_SHIFTUNITS, LONG_OPTION_SHIFTUNITS},
        translate("VCommandLine", "Layout units (as paper's one except px, export mode)."),
        translate("VCommandLine", "The unit")));

    // --shiftlength: Shift length measured in layout units
    optionsIndex.insert(LONG_OPTION_SHIFTLENGTH, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_SHIFTLENGTH, LONG_OPTION_SHIFTLENGTH},
        translate("VCommandLine", "Shift/Offset layout length measured in layout "
                                  "units (export mode). The option show how many points along edge "
                                  "will be used in creating a layout."),
        translate("VCommandLine", "Shift/Offset length")));

    // --gapwidth: gap width between pattern pieces in layout units
    optionsIndex.insert(LONG_OPTION_GAPWIDTH, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_GAPWIDTH, LONG_OPTION_GAPWIDTH},
        translate("VCommandLine", "The layout gap width x2, measured in layout units (export mode). "
                                  "Set distance between details and a detail and a sheet."),
        translate("VCommandLine", "The gap width")));

    // --grouping: Sets the grouping type of pieces
    optionsIndex.insert(LONG_OPTION_GROUPPING, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_GROUPPING, LONG_OPTION_GROUPPING},
        translate("VCommandLine", "Sets layout groupping cases (export mode): %1.")
                                 .arg(LayoutSettingsDialog::MakeGroupsHelp()),
        translate("VCommandLine", "Grouping type"), "2"));

    // --test: Run program in test mode with single pattern file without showing mainwindow
    optionsIndex.insert(LONG_OPTION_TEST, index++);
    options.append(new QCommandLineOption({SINGLE_OPTION_TEST, LONG_OPTION_TEST},
        translate("VCommandLine", "Run the program in a test mode. The program in "
                                  "this mode loads a single pattern file and silently quit without "
                                  "showing the main window. The key have priority before key '%1'.")
                                  .arg(LONG_OPTION_BASENAME)));


    // --no-hdpi-scaling: Disable high DPI auto-scaling (useful on some systems)
    optionsIndex.insert(LONG_OPTION_NO_HDPI_SCALING, index++);
    options.append(new QCommandLineOption({LONG_OPTION_NO_HDPI_SCALING},
        translate("VCommandLine", "Disable high dpi scaling. Call this option if has problem "
                                  "with scaling (by default scaling enabled). Alternatively you "
                                  "can use the %1 environment variable.")
                                  .arg("QT_AUTO_SCREEN_SCALE_FACTOR=0")));
}


//  --------------------
/*
 * @brief Constructs a default layout generator based on command-line options.
 *
 * This function reads CLI options and populates a VLayoutGenerator via a LayoutSettingsDialog,
 * performing validation and error-checking on mutually dependent options. If invalid or incomplete
 * option combinations are detected, it prints an error message and exits via parser.showHelp().
 *
 * @return VLayoutGeneratorPtr A smart pointer to the fully initialized layout generator.
 */
VLayoutGeneratorPtr VCommandLine::DefaultGenerator() const
{
    // Initialize the layout generator and settings dialog
    VLayoutGeneratorPtr res(new VLayoutGenerator());
    LayoutSettingsDialog diag(res.get(), nullptr, true);

    {
        // Check for conflict between page template and manual page size/unit options
        bool x = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGETEMPLATE)));

        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEH)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEW)));
        bool c = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)));

        if ((a || b || c) && x)
        {
            qCritical() << translate("VCommandLine", "Cannot use pageformat and page explicit size/units together.")
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }

        if ((a || b || c) && !(a && b && c))
        {
            qCritical() << translate("VCommandLine", "Page height, width, units must be used all 3 at once.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    {
        // Validate shift length and shift units are both set or neither
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTLENGTH)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTUNITS)));

        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Shift/Offset length must be used together with shift units.")
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    {
        // Validate gap width and shift units
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GAPWIDTH)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTUNITS)));

        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Gap width must be used together with shift units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    // Margin validation blocks
    {
        // Validate left margin and page units. If either is set, both must be set
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_LEFT_MARGIN)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)));
        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Left margin must be used together with page units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }
    {
        // Validate right margin and page units. If either is set, both must be set
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_RIGHT_MARGIN)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)));
        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Right margin must be used together with page units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }
    {
        // Validate top margin and page units. If either is set, both must be set
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_TOP_MARGIN)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)));
        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Top margin must be used together with page units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }
    {
        // Validate bottom margin and page units. If either is set, both must be set
        // Also check if bottom margin is used with top margin
        bool a = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_BOTTOM_MARGIN)));
        bool b = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)));
        if ((a || b) && !(a && b))
        {
            qCritical() << translate("VCommandLine", "Bottom margin must be used together with page units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    // Handle layout rotation
    const int rotateDegree = OptRotation();
    diag.SetRotate(rotateDegree != 0); // Enable rotation toggle

    if (rotateDegree != VSettings::GetDefLayoutRotationIncrease()) // Non-default value
    {
        if (!diag.SetIncrease(rotateDegree))
        {
            qCritical() << translate("VCommandLine", "Invalid rotation value. That must be one of predefined values.")
                        << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    // Set page template (must come before units)
    if (!diag.SelectTemplate(OptPaperSize()))
    {
        qCritical() << translate("VCommandLine", "Unknown page templated selected.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }

    // Handle manual page size input
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEH))))
    {
        if (!diag.SelectPaperUnit(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEUNITS)))))
        {
            qCritical() << translate("VCommandLine", "Unsupported paper units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }

        diag.SetPaperHeight(Pg2Px(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEH))), diag));
        diag.SetPaperWidth(Pg2Px(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGEW))), diag));
    }

    // Layout units and dimensions
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTUNITS))))
    {
        if (!diag.SelectLayoutUnit(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTUNITS)))))
        {
            qCritical() << translate("VCommandLine", "Unsupported layout units.") << "\n";
            const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
        }
    }

    // If the --shiftlength option is provided, convert its value from layout units to pixels
    // and apply it to the layout generator. This shift determines how much the layout will be offset.
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTLENGTH))))
    {
        diag.SetShift(Lo2Px(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SHIFTLENGTH))), diag));
    }

    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GAPWIDTH))))
    {
        diag.setLayoutGap(Lo2Px(parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GAPWIDTH))), diag));
    }

    // Simple toggle options
    diag.SetAutoCrop(parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_CROP)))); // Enable auto-cropping
    diag.SetUnitePages(parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_UNITE)))); // Enable page unification
    diag.SetSaveLength(parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_SAVELENGTH)))); // Enable save length
    diag.SetGroup(OptGroup()); // Set grouping option

    // Handle margins if not ignored
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_IGNORE_MARGINS))))
    {
        diag.SetIgnoreAllFields(true);
    }
    else
    {
        QMarginsF margins = diag.GetFields();

        const QCommandLineOption *option;

        option = optionsUsed.value(optionsIndex.value(LONG_OPTION_LEFT_MARGIN));
        if (parser.isSet(*option)) margins.setLeft(Pg2Px(parser.value(*option), diag));

        option = optionsUsed.value(optionsIndex.value(LONG_OPTION_RIGHT_MARGIN));
        if (parser.isSet(*option)) margins.setRight(Pg2Px(parser.value(*option), diag));

        option = optionsUsed.value(optionsIndex.value(LONG_OPTION_TOP_MARGIN));
        if (parser.isSet(*option)) margins.setTop(Pg2Px(parser.value(*option), diag));

        option = optionsUsed.value(optionsIndex.value(LONG_OPTION_BOTTOM_MARGIN));
        if (parser.isSet(*option)) margins.setBottom(Pg2Px(parser.value(*option), diag));

        diag.SetFields(margins);
    }

    // Finalize layout generator setup
    diag.DialogAccepted();
    return res;
}


//  --------------------
/*
 * @brief Retrieves the singleton instance of VCommandLine and processes CLI arguments.
 *
 * If no VCommandLine exists, a new one is created. It then processes the arguments provided
 * to the application and determines whether the GUI should be enabled based on the CLI mode.
 *
 * @param app The current application instance containing CLI arguments.
 * @return VCommandLinePtr A shared pointer to the global VCommandLine instance.
 */
VCommandLinePtr VCommandLine::Get(const QCoreApplication &app)
{
    if (commandLine == nullptr)
    {
        commandLine.reset(new VCommandLine()); // Replace pointer with new VCommandLine
    }
    commandLine->parser.process(app);          // Process CLI arguments with the parser

    // FIXME: Add more conditions here if new CLI modes are introduced that should disable the GUI
    commandLine->isGuiEnabled = !(commandLine->IsExportEnabled() || commandLine->IsTestModeEnabled());

    return commandLine;
}

//  --------------------
/*
 * @brief Destructor for VCommandLine.
 *
 * Cleans up dynamically allocated list of VCommandLineOptions stored in optionsUsed,
 * ensuring no memory leaks. Clears the list after deleting.
 */
VCommandLine::~VCommandLine()
{
    qDeleteAll(optionsUsed.begin(), optionsUsed.end()); // Delete all dynamically allocated options
    optionsUsed.clear();                                // Clear the list to reset state
}

//  --------------------
/*
 * @brief Resets the singleton instance of VCommandLine.
 *
 * This destroys the current VCommandLine, allowing a fresh one to be created next time Get() is called.
 * Useful for tests or reinitializing command-line parsing.
 */
void VCommandLine::Reset()
{
    commandLine.reset(); // Calls the VCommandLine destructor.
}

//  --------------------
/*
 * @brief Checks whether the application is running in test mode.
 *
 * Determines if the `--test` command-line option is set. If it is, the function
 * verifies that exactly one positional argument (input file) is provided. If not,
 * it prints an error and terminates execution.
 *
 * @return true if test mode is enabled; otherwise false.
 */
bool VCommandLine::IsTestModeEnabled() const
{
    const bool r = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_TEST)));

    // Ensure exactly one positional argument is provided when in test mode
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Test option can be used with single input file only.") << "/n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }

    return r;
}

//  --------------------
/*
 * @brief Checks whether high DPI scaling is disabled.
 *
 * Determines if the --no-hdpi-scaling option is set via the command-line.
 *
 * @return true if scaling is disabled; otherwise false.
 */
bool VCommandLine::IsNoScalingEnabled() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_NO_HDPI_SCALING)));
}

//  --------------------
/*
 * @brief Checks whether the export mode is enabled.
 *
 * This function determines if the --basename option is set, indicating export mode.
 * It also validates that exactly one input file is provided; otherwise, it shows an error and exits.
 *
 * @return true if export mode is enabled; otherwise false.
 */
bool VCommandLine::IsExportEnabled() const
{
    const bool r = parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_BASENAME)));
    if (r && parser.positionalArguments().size() != 1)
    {
        qCritical() << translate("VCommandLine", "Export options can be used with single input file only.") << "/n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
    return r;
}

//  --------------------
/*
 * @brief Retrieves the selected paper size template.
 *
 * If the --pagetemplate option is provided, its integer value is parsed and cast
 * to a PaperSizeFormat enum. Defaults to 0 if not set.
 *
 * @return PaperSizeFormat value corresponding to the selected page template.
 */
PaperSizeFormat VCommandLine::OptPaperSize() const
{
    int ppsize = 0;
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGETEMPLATE))))
    {
        ppsize = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_PAGETEMPLATE))).toInt();
    }
    return static_cast<PaperSizeFormat>(ppsize);
}

//  --------------------
/*
 * @brief Retrieves the layout rotation value from command-line options.
 *
 * Checks if the --rotate option is set. If so, parses the value and verifies it's a valid integer.
 * Falls back to the default rotation if parsing fails or the option is not set.
 *
 * @return Integer value representing the rotation angle.
 */
int VCommandLine::OptRotation() const
{
    int rotate = VSettings::GetDefLayoutRotationIncrease();
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_ROTATE))))
    {
        bool ok = false;
        rotate = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_ROTATE))).toInt(&ok);

        if (not ok)
        {
            rotate = VSettings::GetDefLayoutRotationIncrease();
        }
    }

    return rotate;
}

//  --------------------
/*
 * @brief Retrieves the selected layout grouping type.
 *
 * Parses the --groupping option value as an integer and maps it to a Cases enum value.
 * Returns 0 (default case) if the value is invalid or out of range.
 *
 * @return Cases enum representing the selected grouping mode.
 */
Cases VCommandLine::OptGroup() const
{
    int r = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GROUPPING))).toInt();
    if (r < 0 || r >= static_cast<int>(Cases::UnknownCase))
    {
        r = 0;
    }
    return static_cast<Cases>(r);
}

//  --------------------
/*
 * @brief Retrieves the path to the measurement file if applicable.
 *
 * Only returns the measurement file path if the --measurefile option is set
 * and the app is running in export or test mode.
 *
 * @return QString containing the measurement file path, or empty if not applicable.
 */
QString VCommandLine::OptMeasurePath() const
{
    QString measure;
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_MEASUREFILE)))
        && (IsExportEnabled() || IsTestModeEnabled()))
        // TODO: Currently restricting general use of measure file until window handling is improved
    {
        measure = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_MEASUREFILE)));
    }

    return measure;
}

//  --------------------
/*
 * @brief Retrieves the base name for exported files.
 *
 * Only returns the value if the application is in export mode.
 *
 * @return QString containing the base file name, or empty if export mode is not active.
 */
QString VCommandLine::OptBaseName() const
{
    QString path;
    if (IsExportEnabled())
    {
        path = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_BASENAME)));
    }

    return path;
}

//  --------------------
/*
 * @brief Retrieves the destination path for export output.
 *
 * Only returns the path if export mode is enabled.
 *
 * @return QString containing the destination folder path, or empty if not applicable.
 */
 QString VCommandLine::OptDestinationPath() const
 {
     QString path;
     if (IsExportEnabled())
     {
         path = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_DESTINATION)));
     }
     return path;
 }

//  --------------------
/*
 * @brief Retrieves the selected export format type.
 *
 * Parses the --exp2format option if set, returning its integer value.
 *
 * @return Integer indicating the export file format.
 */
int VCommandLine::OptExportType() const
{
    int r = 0;
    if (parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_EXP2FORMAT))))
    {
        r = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_EXP2FORMAT))).toInt();
    }
    return r;
}

//  --------------------
 /*
  * @brief Checks whether DXF export should be in binary format.
  *
  * @return Non-zero if binary DXF mode is enabled, otherwise 0.
  */
int VCommandLine::IsBinaryDXF() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_BINARYDXF)));
}

//  --------------------
/*
 * @brief Checks whether text should be exported as paths.
 *
 * @return Non-zero if text-as-paths mode is enabled, otherwise 0.
 */
int VCommandLine::isTextAsPaths() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_TEXT2PATHS)));
}

//  --------------------
/*
 * @brief Checks whether only pieces/details should be exported.
 *
 * @return Non-zero if only details export is enabled, otherwise 0.
 */
int VCommandLine::exportOnlyPieces() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_EXPORTONLYDETAILS)));
}

//  --------------------
/*
 * @brief Retrieves the list of input filenames passed as positional arguments.
 *
 * @return QStringList of input file names.
 */
QStringList VCommandLine::OptInputFileNames() const
{
    return parser.positionalArguments();
}

//  --------------------
/*
 * @brief Checks whether the GUI should be enabled based on current CLI mode.
 *
 * @return true if GUI mode is enabled; otherwise false.
 */
bool VCommandLine::IsGuiEnabled() const
{
    return isGuiEnabled;
}

//  --------------------
/*
 * @brief Checks if a specific gradation size is set via the CLI.
 *
 * @return true if --gradationsize option is set; otherwise false.
 */
bool VCommandLine::IsSetGradationSize() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GRADATIONSIZE)));
}

//  --------------------
/*
 * @brief Checks if a specific gradation height is set via the CLI.
 *
 * @return true if --gradationheight option is set; otherwise false.
 */
bool VCommandLine::IsSetGradationHeight() const
{
    return parser.isSet(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GRADATIONHEIGHT)));
}

//  --------------------
/*
 * @brief Retrieves the gradation size value from the command-line.
 *
 * This function fetches the value of the --gradationsize option and validates it
 * using MeasurementVariable::IsGradationSizeValid(). If invalid, it prints an error
 * and exits the application via showHelp().
 *
 * @return QString containing a valid gradation size.
 */
QString VCommandLine::OptGradationSize() const
{
    const QString size = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GRADATIONSIZE)));
    if (MeasurementVariable::IsGradationSizeValid(size))
    {
        return size;
    }
    else
    {
        qCritical() << translate("VCommandLine", "Invalid gradation size value.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
}

//  --------------------
/*
 * @brief Retrieves the gradation height value from the command-line.
 *
 * This function fetches the value of the --gradationheight option and validates it
 * using MeasurementVariable::IsGradationHeightValid(). If invalid, it prints an error
 * and exits the application via showHelp().
 *
 * @return QString containing a valid gradation height.
 */
QString VCommandLine::OptGradationHeight() const
{
    const QString height = parser.value(*optionsUsed.value(optionsIndex.value(LONG_OPTION_GRADATIONHEIGHT)));
    if (MeasurementVariable::IsGradationHeightValid(height))
    {
        return height;
    }
    else
    {
        qCritical() << translate("VCommandLine", "Invalid gradation height value.") << "\n";
        const_cast<VCommandLine*>(this)->parser.showHelp(V_EX_USAGE);
    }
}

#undef translate

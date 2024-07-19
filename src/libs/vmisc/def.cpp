/************************************************************************
 **  @file   def.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2015
 **
 **  @author DS Caskey
 **  @date   Jul 31, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#include "def.h"

#include "vabstractapplication.h"

#include <QApplication>
#include <QChar>
#include <QColor>
#include <QComboBox>
#include <QCursor>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QLatin1Char>
#include <QList>
#include <QMap>
#include <QMessageLogger>
#include <QObject>
#include <QPixmap>
#include <QPrinterInfo>
#include <QProcess>
#include <QRgb>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QtDebug>
#include <QPixmapCache>
#include <QGraphicsItem>
#include <QDesktopServices>
#include <QUrl>

//functions
const QString degTorad_F = QStringLiteral("degTorad");
const QString radTodeg_F = QStringLiteral("radTodeg");
const QString sin_F   = QStringLiteral("sin");
const QString cos_F   = QStringLiteral("cos");
const QString tan_F   = QStringLiteral("tan");
const QString asin_F  = QStringLiteral("asin");
const QString acos_F  = QStringLiteral("acos");
const QString atan_F  = QStringLiteral("atan");
const QString sinh_F  = QStringLiteral("sinh");
const QString cosh_F  = QStringLiteral("cosh");
const QString tanh_F  = QStringLiteral("tanh");
const QString asinh_F = QStringLiteral("asinh");
const QString acosh_F = QStringLiteral("acosh");
const QString atanh_F = QStringLiteral("atanh");
const QString sinD_F   = QStringLiteral("sinD");
const QString cosD_F   = QStringLiteral("cosD");
const QString tanD_F   = QStringLiteral("tanD");
const QString asinD_F  = QStringLiteral("asinD");
const QString acosD_F  = QStringLiteral("acosD");
const QString atanD_F  = QStringLiteral("atanD");
const QString log2_F  = QStringLiteral("log2");
const QString log10_F = QStringLiteral("log10");
const QString log_F   = QStringLiteral("log");
const QString ln_F    = QStringLiteral("ln");
const QString exp_F   = QStringLiteral("exp");
const QString sqrt_F  = QStringLiteral("sqrt");
const QString sign_F  = QStringLiteral("sign");
const QString rint_F  = QStringLiteral("rint");
const QString abs_F   = QStringLiteral("abs");
const QString min_F   = QStringLiteral("min");
const QString max_F   = QStringLiteral("max");
const QString sum_F   = QStringLiteral("sum");
const QString avg_F   = QStringLiteral("avg");
const QString fmod_F  = QStringLiteral("fmod");

const QStringList builInFunctions = QStringList() << degTorad_F << radTodeg_F
                                                  << sin_F  << cos_F   << tan_F  << asin_F  << acos_F  << atan_F
                                                  << sinh_F << cosh_F  << tanh_F << asinh_F << acosh_F << atanh_F
                                                  << sinD_F  << cosD_F   << tanD_F  << asinD_F  << acosD_F  << atanD_F
                                                  << log2_F << log10_F << log_F  << ln_F    << exp_F   << sqrt_F
                                                  << sign_F << rint_F  << abs_F  << min_F   << max_F   << sum_F
                                                  << avg_F << fmod_F;

// Postfix operators
const QString cm_Oprt = QStringLiteral("cm");
const QString mm_Oprt = QStringLiteral("mm");
const QString in_Oprt = QStringLiteral("in");

const QStringList builInPostfixOperators = QStringList() << cm_Oprt
                                                         << mm_Oprt
                                                         << in_Oprt;

const QString pl_size          = QStringLiteral("size");
const QString pl_height        = QStringLiteral("height");
const QString pl_date          = QStringLiteral("date");
const QString pl_time          = QStringLiteral("time");
const QString pl_patternName   = QStringLiteral("patternName");
const QString pl_patternNumber = QStringLiteral("patternNumber");
const QString pl_author        = QStringLiteral("author");
const QString pl_customer      = QStringLiteral("customer");
const QString pl_pExt          = QStringLiteral("pExt");
const QString pl_pFileName     = QStringLiteral("pFileName");
const QString pl_mFileName     = QStringLiteral("mFileName");
const QString pl_mExt          = QStringLiteral("mExt");
const QString pl_pLetter       = QStringLiteral("pLetter");
const QString pl_pAnnotation   = QStringLiteral("pAnnotation");
const QString pl_pOrientation  = QStringLiteral("pOrientation");
const QString pl_pRotation     = QStringLiteral("pRotation");
const QString pl_pTilt         = QStringLiteral("pTilt");
const QString pl_pFoldPosition = QStringLiteral("pFoldPosition");
const QString pl_pName         = QStringLiteral("pName");
const QString pl_pQuantity     = QStringLiteral("pQuantity");
const QString pl_mFabric       = QStringLiteral("mFabric");
const QString pl_mLining       = QStringLiteral("mLining");
const QString pl_mInterfacing  = QStringLiteral("mInterfacing");
const QString pl_mInterlining  = QStringLiteral("mInterlining");
const QString pl_wCut          = QStringLiteral("wCut");
const QString pl_wOnFold       = QStringLiteral("wOnFold");

const QStringList labelTemplatePlaceholders = QStringList() << pl_size
                                                            << pl_height
                                                            << pl_date
                                                            << pl_time
                                                            << pl_patternName
                                                            << pl_patternNumber
                                                            << pl_author
                                                            << pl_customer
                                                            << pl_pExt
                                                            << pl_pFileName
                                                            << pl_mFileName
                                                            << pl_mExt
                                                            << pl_pLetter
                                                            << pl_pAnnotation
                                                            << pl_pOrientation
                                                            << pl_pRotation
                                                            << pl_pTilt
                                                            << pl_pFoldPosition
                                                            << pl_pName
                                                            << pl_pQuantity
                                                            << pl_mFabric
                                                            << pl_mLining
                                                            << pl_mInterfacing
                                                            << pl_mInterlining
                                                            << pl_wCut
                                                            << pl_wOnFold;

const QString cursorArrowOpenHand  = QStringLiteral("://cursor/cursor-arrow-openhand.png");
const QString cursorArrowCloseHand = QStringLiteral("://cursor/cursor-arrow-closehand.png");
const QString cursorResizeArrow = QStringLiteral("://cursor/arrow_resize_cursor.png");
const QString cursorImageOrigin = QStringLiteral("://cursor/image_origin_cursor.png");

// From documantation: If you use QStringLiteral you should avoid declaring the same literal in multiple places: This
// furthermore blows up the binary sizes.
const QString degreeSymbol = QStringLiteral("°");
const QString trueStr      = QStringLiteral("true");
const QString falseStr     = QStringLiteral("false");

const QString unitMM       = QStringLiteral("mm");
const QString unitCM       = QStringLiteral("cm");
const QString unitINCH     = QStringLiteral("inch");
const QString unitPX       = QStringLiteral("px");

const QString valExt       = QStringLiteral("val");
const QString vitExt       = QStringLiteral("vit");
const QString vstExt       = QStringLiteral("vst");
const QString sm2dExt      = QStringLiteral("sm2d");
const QString smisExt      = QStringLiteral("smis");
const QString smmsExt      = QStringLiteral("smms");

//---------------------------------------------------------------------------------------------------------------------
void SetItemOverrideCursor(QGraphicsItem *item, const QString &pixmapPath, int hotX, int hotY)
{
#ifndef QT_NO_CURSOR
    SCASSERT(item != nullptr)

    QPixmap pixmap;

    if (!QPixmapCache::find(pixmapPath, &pixmap))
    {
        pixmap = QPixmap(pixmapPath);
        QPixmapCache::insert(pixmapPath, pixmap);
    }

    item->setCursor(QCursor(pixmap, hotX, hotY));
#else
    Q_UNUSED(item)
    Q_UNUSED(pixmapPath)
    Q_UNUSED(hotX)
    Q_UNUSED(hotY)
#endif
}

const qreal PrintDPI = 96.0;

//---------------------------------------------------------------------------------------------------------------------
double ToPixel(double val, const Unit &unit)
{
    switch (unit)
    {
        case Unit::Mm:
            return (val / 25.4) * PrintDPI;
        case Unit::Cm:
            return ((val * 10.0) / 25.4) * PrintDPI;
        case Unit::Inch:
            return val * PrintDPI;
        case Unit::Px:
            return val;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
double FromPixel(double pix, const Unit &unit)
{
    switch (unit)
    {
        case Unit::Mm:
            return (pix / PrintDPI) * 25.4;
        case Unit::Cm:
            return ((pix / PrintDPI) * 25.4) / 10.0;
        case Unit::Inch:
            return pix / PrintDPI;
        case Unit::Px:
            return pix;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
qreal UnitConvertor(qreal value, const Unit &from, const Unit &to)
{
    switch (from)
    {
        case Unit::Mm:
            switch (to)
            {
                case Unit::Mm:
                    return value;
                case Unit::Cm:
                    return value / 10.0;
                case Unit::Inch:
                    return value / 25.4;
                case Unit::Px:
                    return (value / 25.4) * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Cm:
            switch (to)
            {
                case Unit::Mm:
                    return value * 10.0;
                case Unit::Cm:
                    return value;
                case Unit::Inch:
                    return value / 2.54;
                case Unit::Px:
                    return ((value * 10.0) / 25.4) * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Inch:
            switch (to)
            {
                case Unit::Mm:
                    return value * 25.4;
                case Unit::Cm:
                    return value * 2.54;
                case Unit::Inch:
                    return value;
                case Unit::Px:
                    return value * PrintDPI;
                default:
                    break;
            }
            break;
        case Unit::Px:
            switch (to)
            {
                case Unit::Mm:
                    return (value / PrintDPI) * 25.4;
                case Unit::Cm:
                    return ((value / PrintDPI) * 25.4) / 10.0;
                case Unit::Inch:
                    return value / PrintDPI;
                case Unit::Px:
                    return value;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnitConvertor Converts the values of the given margin from given unit to the new unit.
 * returns a new instand of QMarginsF.
 *
 * @param margin
 * @param from
 * @param to
 * @return
 */
QMarginsF UnitConvertor(const QMarginsF &margins, const Unit &from, const Unit &to)
{
    const qreal left = UnitConvertor(margins.left(),from, to);
    const qreal top = UnitConvertor(margins.top(),from, to);
    const qreal right = UnitConvertor(margins.right(),from, to);
    const qreal bottom = UnitConvertor(margins.bottom(),from, to);

    return QMarginsF(left, top, right, bottom);
}


//---------------------------------------------------------------------------------------------------------------------
QStringList SupportedLocales()
{
    const QStringList locales = QStringList() << QStringLiteral("ru_RU")
                                              << QStringLiteral("uk_UA")
                                              << QStringLiteral("de_DE")
                                              << QStringLiteral("cs_CZ")
                                              << QStringLiteral("he_IL")
                                              << QStringLiteral("fr_FR")
                                              << QStringLiteral("it_IT")
                                              << QStringLiteral("nl_NL")
                                              << QStringLiteral("id_ID")
                                              << QStringLiteral("es_ES")
                                              << QStringLiteral("fi_FI")
                                              << QStringLiteral("en_US")
                                              << QStringLiteral("en_CA")
                                              << QStringLiteral("en_IN")
                                              << QStringLiteral("ro_RO")
                                              << QStringLiteral("zh_CN")
                                              << QStringLiteral("pt_BR")
                                              << QStringLiteral("el_GR")
                                              << QStringLiteral("en_GB");
    return locales;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief strippedName the function call around curFile to exclude the path to the file.
 * @param fullFileName full path to the file.
 * @return file name.
 */
QString strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

/**
 * @brief makeHeaderName make a 1 char tablewidgetitem header name based on a translated string.
 * @param name full name of header item.
 * @return 1 char name.
 */
QString makeHeaderName(const QString &name)
{
    QString headerStr = QObject::tr("%1").arg(name);
    return headerStr.at(0).toUpper();
}

//---------------------------------------------------------------------------------------------------------------------
QString RelativeMPath(const QString &patternPath, const QString &absoluteMPath)
{
    if (patternPath.isEmpty() || absoluteMPath.isEmpty())
    {
        return absoluteMPath;
    }

    if (QFileInfo(absoluteMPath).isRelative())
    {
        return absoluteMPath;
    }

    return QFileInfo(patternPath).absoluteDir().relativeFilePath(absoluteMPath);
}

//---------------------------------------------------------------------------------------------------------------------
QString AbsoluteMPath(const QString &patternPath, const QString &relativeMPath)
{
    if (patternPath.isEmpty() || relativeMPath.isEmpty())
    {
        return relativeMPath;
    }

    if (QFileInfo(relativeMPath).isAbsolute())
    {
        return relativeMPath;
    }

    return QFileInfo(QFileInfo(patternPath).absoluteDir(), relativeMPath).absoluteFilePath();
}

QString fileDialog(QWidget *parent, const QString &title,  const QString &dir, const QString &filter,
                   QString *selectedFilter, QFileDialog::Options options, QFileDialog::FileMode mode,
                   QFileDialog::AcceptMode accept)
{
    QFileDialog dialog(parent, title, dir, filter);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setOptions(options);
    dialog.setFileMode(mode);
    dialog.setAcceptMode(accept);
    dialog.setSupportedSchemes(QStringList(QStringLiteral("file")));
    if (selectedFilter && !selectedFilter->isEmpty())
    {
        dialog.selectNameFilter(*selectedFilter);
    }

    QUrl selectedUrl;
    if (dialog.exec() == QDialog::Accepted)
    {
        if (selectedFilter)
        {
            *selectedFilter = dialog.selectedNameFilter();
        }
        selectedUrl = dialog.selectedUrls().value(0);
    }
    else
    {
        selectedUrl = QUrl();
    }

    if (selectedUrl.isLocalFile() || selectedUrl.isEmpty())
    {
        return selectedUrl.toLocalFile();
    }
    else
    {
        return selectedUrl.toString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QSharedPointer<QPrinter> PreparePrinter(const QPrinterInfo &info, QPrinter::PrinterMode mode)
{
    QPrinterInfo tmpInfo = info;
    if(tmpInfo.isNull() || tmpInfo.printerName().isEmpty())
    {
        const QStringList list = QPrinterInfo::availablePrinterNames();

        if(list.isEmpty())
        {
            return QSharedPointer<QPrinter>();
        }
        else
        {
            tmpInfo = QPrinterInfo::printerInfo(list.first());
        }
    }

    auto printer = QSharedPointer<QPrinter>(new QPrinter(tmpInfo, mode));
    printer->setResolution(static_cast<int>(PrintDPI));
    return printer;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF GetMinPrinterFields(const QSharedPointer<QPrinter> &printer)
{
    QPageLayout layout = printer->pageLayout();
    layout.setUnits(QPageLayout::Millimeter);
    const QMarginsF minMargins = layout.minimumMargins();
    QMarginsF min;
    min.setLeft(UnitConvertor(minMargins.left(), Unit::Mm, Unit::Px));
    min.setRight(UnitConvertor(minMargins.right(), Unit::Mm, Unit::Px));
    min.setTop(UnitConvertor(minMargins.top(), Unit::Mm, Unit::Px));
    min.setBottom(UnitConvertor(minMargins.bottom(), Unit::Mm, Unit::Px));
    return min;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF GetPrinterFields(const QSharedPointer<QPrinter> &printer)
{
    if (printer.isNull())
    {
        return QMarginsF();
    }

    // We can't use Unit::Px because our dpi in most cases is different
    return UnitConvertor(printer->pageLayout().margins(), Unit::Mm, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QPixmap darkenPixmap(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    const int imgh = img.height();
    const int imgw = img.width();
    for (int y = 0; y < imgh; ++y)
    {
        for (int x = 0; x < imgw; ++x)
        {
            int h, s, v;
            QRgb pixel = img.pixel(x, y);
            const int a = qAlpha(pixel);
            QColor hsvColor(pixel);
            hsvColor.getHsv(&h, &s, &v);
            s = qMin(100, s * 2);
            v = v / 2;
            hsvColor.setHsv(h, s, v);
            pixel = hsvColor.rgb();
            img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
        }
    }
    return QPixmap::fromImage(img);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowInGraphicalShell(const QString &filePath)
{
    QStringList args;
#ifdef Q_OS_MAC
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \""+filePath+"\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached(QStringLiteral("osascript"), args);
#elif defined(Q_OS_WIN)
    QProcess::startDetached(QStringLiteral("explorer"), QStringList{"/select", QDir::toNativeSeparators(filePath)});
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).path()));
#endif

}

const QString LONG_OPTION_NO_HDPI_SCALING = QStringLiteral("no-scaling");

//---------------------------------------------------------------------------------------------------------------------
bool IsOptionSet(int argc, char *argv[], const char *option)
{
    for (int i = 1; i < argc; ++i)
    {
        if (!qstrcmp(argv[i], option))
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void initHighDpiScaling(int argc, char *argv[])
{
    /* For more info see: http://doc.qt.io/qt-5/highdpi.html */
    if (IsOptionSet(argc, argv, qPrintable(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING)))
    {
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    }
    else
    {
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
    }
}
//---------------------------------------------------------------------------------------------------------------------

const QString strForward   = QStringLiteral("forward");
const QString strBackward  = QStringLiteral("backward");

QString directionToString(Direction type)
{
    switch(type)
    {
        case Direction::Forward:
            return strForward;
        case Direction::Backward:
            return strBackward;
            default:
                break;
    }
    return strForward;
}

Direction stringToDirection(const QString &value)
{
    const QStringList values = QStringList() << strForward << strBackward;

    switch(values.indexOf(value))
    {
        case 0:
            return Direction::Forward;
        case 1:
            return Direction::Backward;
            default:
                break;
    }
    return Direction::Forward;
}


const QString strSlit      = QStringLiteral("slit");
const QString strTNotch    = QStringLiteral("tNotch");
const QString strUNotch    = QStringLiteral("uNotch");
const QString strVInternal = QStringLiteral("vInternal");
const QString strVExternal = QStringLiteral("vExternal");
const QString strCastle    = QStringLiteral("castle");
const QString strDiamond   = QStringLiteral("diamond");

//---------------------------------------------------------------------------------------------------------------------
QString notchTypeToString(NotchType type)
{
    switch(type)
    {
        case NotchType::Slit:
            return strSlit;
        case NotchType::TNotch:
            return strTNotch;
        case NotchType::UNotch:
            return strUNotch;
        case NotchType::VInternal:
            return strVInternal;
        case NotchType::VExternal:
            return strVExternal;
        case NotchType::Castle:
            return strCastle;
        case NotchType::Diamond:
            return strDiamond;
        default:
            break;
    }

    return strSlit;
}

//---------------------------------------------------------------------------------------------------------------------
NotchType stringToNotchType(const QString &value)
{
    const QStringList values = QStringList() << strSlit << strTNotch << strUNotch << strVInternal << strVExternal
                                             << strCastle << strDiamond;

    switch(values.indexOf(value))
    {
        case 0:
            return NotchType::Slit;
        case 1:
            return NotchType::TNotch;
        case 2:
            return NotchType::UNotch;
        case 3:
            return NotchType::VInternal;
        case 4:
            return NotchType::VExternal;
        case 5:
            return NotchType::Castle;
        case 6:
            return NotchType::Diamond;
        default:
            break;
    }
    return NotchType::Slit;
}

const QString strStraightforward = QStringLiteral("straightforward");
const QString strBisector        = QStringLiteral("bisector");
const QString strIntersection    = QStringLiteral("intersection");

//---------------------------------------------------------------------------------------------------------------------
QString notchSubTypeToString(NotchSubType type)
{
    switch(type)
    {
        case NotchSubType::Straightforward:
            return strStraightforward;
        case NotchSubType::Bisector:
            return strBisector;
        case NotchSubType::Intersection:
            return strIntersection;
        default:
            break;
    }

    return strStraightforward;
}

//---------------------------------------------------------------------------------------------------------------------
NotchSubType stringToNotchSubType(const QString &value)
{
    const QStringList values = QStringList() << strStraightforward << strBisector << strIntersection;

    switch(values.indexOf(value))
    {
        case 0:
            return NotchSubType::Straightforward;
        case 1:
            return NotchSubType::Bisector;
        case 2:
            return NotchSubType::Intersection;
        default:
            break;
    }
    return NotchSubType::Straightforward;
}


//---------------------------------------------------------------------------------------------------------------------
Unit StrToUnits(const QString &unit)
{
    const QStringList units = QStringList() << unitMM << unitCM << unitINCH << unitPX;
    Unit result = Unit::Cm;
    switch (units.indexOf(unit))
    {
        case 0:// mm
            result = Unit::Mm;
            break;
        case 2:// inch
            result = Unit::Inch;
            break;
        case 3:// px
            result = Unit::Px;
            break;
        case 1:// cm
        default:
            result = Unit::Cm;
            break;
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnitsToStr translate unit to string.
 *
 * This method used when need write unit in xml file and for showing unit in dialogs.
 * @param unit curent unit
 * @param translate true if need show translated name. Default value false.
 * @return string reprezantation for unit.
 */
QString UnitsToStr(const Unit &unit, const bool translate)
{
    QString result;
    switch (unit)
    {
        case Unit::Mm:
            translate ? result = QObject::tr("mm") : result = unitMM;
            break;
        case Unit::Inch:
            translate ? result = QObject::tr("inch") : result = unitINCH;
            break;
        case Unit::Px:
            translate ? result = QObject::tr("px") : result = unitPX;
            break;
        case Unit::Cm:
        default:
            translate ? result = QObject::tr("cm") : result = unitCM;
            break;
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void InitLanguages(QComboBox *combobox)
{
    SCASSERT(combobox != nullptr)
    combobox->clear();

    QStringList fileNames;
    QDirIterator it(qApp->translationsPath(), QStringList("seamly2d_*.qm"), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        fileNames.append(it.fileName());
    }

    bool englishUS = false;
    const QString en_US = QStringLiteral("en_US");

    for (int i = 0; i < fileNames.size(); ++i)
    {
        // get locale extracted by filename
        QString locale;
        locale = fileNames.at(i);                  // "seamly2d_de_De.qm"
        locale.truncate(locale.lastIndexOf('.'));  // "seamly2d_de_De"
        locale.remove(0, locale.indexOf('_') + 1); // "de_De"

        if (!englishUS)
        {
            englishUS = (locale == en_US);
        }

        QLocale loc = QLocale(locale);
        QString lang = loc.nativeLanguageName();
        QString country = QLocale::countryToString(loc.country());
        if (country == QLatin1String("Czechia"))
        {
            country = QLatin1String("CzechRepublic");
        }
        QIcon ico(QString("://flags/%1.png").arg(country));

        combobox->addItem(ico, lang, locale);
    }

    if (combobox->count() == 0 || !englishUS)
    {
        // English language is internal and doens't have own *.qm file.
        QIcon ico(QString(":/flags/United States.png"));
        QString lang = QLocale(en_US).nativeLanguageName();
        combobox->addItem(ico, lang, en_US);
    }

    // set default translators and language checked
    qint32 index = combobox->findData(qApp->Settings()->getLocale());
    if (index != -1)
    {
        combobox->setCurrentIndex(index);
    }
    else
    {
        combobox->setCurrentIndex(combobox->findData(en_US));
    }
}

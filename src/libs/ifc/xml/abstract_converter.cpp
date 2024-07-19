 //  @file   abstract_converter.cpp
 //  @author Douglas S Caskey
 //  @date   24 Jun, 2024
 //
 //  @brief
 //  @copyright
 //  This source code is part of the Seamly2D project, a pattern making
 //  program to create and model patterns of clothing.
 //  Copyright (C) 2017-2024 Seamly2D project
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

 /************************************************************************
 **
 **  @file   vabstractconverter.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "abstract_converter.h"

#include <QDir>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>
#include <QFileInfo>
#include <QLatin1String>
#include <QMap>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QStringList>

#include "vdomdocument.h"
#include "../exception/vexception.h"
#include "../exception/vexceptionwrongid.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vsettings.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractConverter::VAbstractConverter(const QString &fileName)
    : VDomDocument()
    , m_ver(0x0)
    , m_convertedFileName(fileName)
    , m_tmpFile()
{
    setXMLContent(m_convertedFileName);// Throw an exception on error
    m_ver = GetVersion(getVersionStr());

    qInfo() << "VAbstractConverter::GetVersion() = " << m_ver;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractConverter::Convert()
{
    if (m_ver == maxVer())
    {
        return m_convertedFileName;
    }

    if (!isReadOnly())
    {
        saveBackupFile();
    }

    if (m_tmpFile.open())
    {
        m_convertedFileName = m_tmpFile.fileName();
    }
    else
    {
        const QString errorMsg(tr("Error Opening a temp file: %1.").arg(m_tmpFile.errorString()));
        throw VException(errorMsg);
    }

    qInfo() << " m_ver = " << m_ver;
    qInfo() << " MaxVer = " << maxVer();

    m_ver < maxVer() ? applyPatches() : downgradeToCurrentMaxVersion();

    return m_convertedFileName;
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractConverter::getCurrentFormatVersion() const
{
    return m_ver;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractConverter::getVersionStr() const
{
    const QDomNodeList nodeList = this->elementsByTagName(TagVersion);
    if (nodeList.isEmpty())
    {
        const QString errorMsg(tr("Couldn't get version information."));
        throw VException(errorMsg);
    }

    if (nodeList.count() > 1)
    {
        const QString errorMsg(tr("Too many tags <%1> in file.").arg(TagVersion));
        throw VException(errorMsg);
    }

    const QDomNode domNode = nodeList.at(0);
    if (domNode.isNull() == false && domNode.isElement())
    {
        const QDomElement domElement = domNode.toElement();
        if (domElement.isNull() == false)
        {
            qInfo() << " VAbstractConverter::getVersionStr()" << domElement.text();
            return domElement.text();
        }
    }
    return QString(QStringLiteral("0.0.0"));
}

//---------------------------------------------------------------------------------------------------------------------
int VAbstractConverter::GetVersion(const QString &version)
{
    ValidateVersion(version);

    const QStringList ver = version.split(".");

    bool ok = false;
    const int major = ver.at(0).toInt(&ok);
    if (!ok)
    {
        return 0x0;
    }

    ok = false;
    const int minor = ver.at(1).toInt(&ok);
    if (!ok)
    {
        return 0x0;
    }

    ok = false;
    const int patch = ver.at(2).toInt(&ok);
    if (!ok)
    {
        return 0x0;
    }
    return (major<<16)|(minor<<8)|(patch);
}

// @brief Removes single or repeated '.bak' extension (as long as it is at the end of \arg fileName)
// @Param filename file to backup
// @Return newfileName
QString VAbstractConverter::removeBakExtension(const QString &fileName)
{
    QString newFileName = fileName;
    int dotPos = newFileName.lastIndexOf(QLatin1Char('.'));

    while (dotPos > -1 && newFileName.mid(dotPos) == QLatin1String(".bak"))
    {
        newFileName = newFileName.left(dotPos);
        dotPos = newFileName.lastIndexOf(QLatin1Char('.'));
    }

    return newFileName;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::ValidateVersion(const QString &version)
{
    const QRegularExpression rx(QStringLiteral("^(0|([1-9][0-9]*)).(0|([1-9][0-9]*)).(0|([1-9][0-9]*))$"));

    if (rx.match(version).hasMatch() == false)
    {
        const QString errorMsg(tr("Version \"%1\" invalid.").arg(version));
        throw VException(errorMsg);
    }

    if (version == QLatin1String("0.0.0"))
    {
        const QString errorMsg(tr("Version \"0.0.0\" invalid."));
        throw VException(errorMsg);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::saveBackupFile() const
{
    //It's not possible in all cases to convert file without a loss of data.
    //For such cases we will save a backup of the old version.
    if (qApp->Settings()->getConvertBackupEnabled())
    {
        QString error;
        const QFileInfo info(removeBakExtension(m_convertedFileName));
        QString baseFileName = info.baseName();

        // replace any spaces in filename with underscore to pevent
        // errors when opening file in Linux through the command line.
        baseFileName.replace(" ", "_");

        QString path = qApp->Settings()->getBackupFilePath();
        if (!info.exists(path))
        {
            path = info.absoluteDir().absolutePath();
        }
        QString backupFileName;
        backupFileName = QString("%1/%2%3.%4").arg(path, baseFileName, "_(backup)", info.completeSuffix());

        if (!SafeCopy(m_convertedFileName, backupFileName, error))
        {
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
        const bool isFileWritable = info.isWritable();
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

            if (!isReadOnly() && isFileWritable)
            {
                const QString errorMsg(tr("Error creating a reserv copy: %1.").arg(error));
                throw VException(errorMsg);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::Replace(QString &formula, const QString &newName, int position, const QString &token,
                                 int &bias) const
{
    formula.replace(position, token.length(), newName);
    bias = token.length() - newName.length();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::CorrectionsPositions(int position, int bias, QMap<int, QString> &tokens) const
{
    if (bias == 0)
    {
        return;// Nothing to correct;
    }

    BiasTokens(position, bias, tokens);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::BiasTokens(int position, int bias, QMap<int, QString> &tokens)
{
    QMap<int, QString> newTokens;
    QMap<int, QString>::const_iterator i = tokens.constBegin();
    while (i != tokens.constEnd())
    {
        if (i.key()<= position)
        { // Tokens before position "position" did not change his positions.
            newTokens.insert(i.key(), i.value());
        }
        else
        {
            newTokens.insert(i.key()-bias, i.value());
        }
        ++i;
    }
    tokens = newTokens;
}

//---------------------------------------------------------------------------------------------------------------------
Q_NORETURN void VAbstractConverter::InvalidVersion(int ver) const
{
    if (ver < minVer())
    {
        const QString errorMsg(tr("Invalid version. Minimum supported version is %1").arg(minVerStr()));
        throw VException(errorMsg);
    }

    if (ver > maxVer())
    {
        const QString errorMsg(tr("Invalid version. Maximum supported version is %1").arg(maxVerStr()));
        throw VException(errorMsg);
    }

    const QString errorMsg(tr("Unexpected version \"%1\".").arg(ver, 0, 16));
    throw VException(errorMsg);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::ValidateInputFile(const QString &currentSchema) const
{
    QString schema;
    try
    {
        schema = getSchema(m_ver);
    }
    catch(const VException &error)
    {
        if (m_ver < minVer())
        { // Version less than minimally supported version. Can't do anything.
            throw;
        }
        else if (m_ver > maxVer())
        { // Version bigger than maximum supported version. We still have a chance to open the file.
            try
            { // Try to open like the current version.
                ValidateXML(currentSchema, m_convertedFileName);
            }
            catch(const VException &exp)
            { // Nope, we can't.
                Q_UNUSED(exp)
                throw error;
            }
        }
        else
        { // Unexpected version. Most time mean that we do not catch all versions between min and max.
            throw;
        }

        return; // All is fine and we can try to convert to current max version.
    }

    ValidateXML(schema, m_convertedFileName);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::Save()
{
    try
    {
        TestUniqueId();
    }
    catch (const VExceptionWrongId &error)
    {
        Q_UNUSED(error)
        VException ex(tr("Error no unique id."));
        throw ex;
    }

    m_tmpFile.resize(0);//clear previous content
    const int indent = 4;
    QTextStream out(&m_tmpFile);
    out.setCodec("UTF-8");
    save(out, indent);

    if (!m_tmpFile.flush())
    {
        VException e(m_tmpFile.errorString());
        throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractConverter::setVersion(const QString &version)
{
    ValidateVersion(version);

    if (setTagText(TagVersion, version) == false)
    {
        VException e(tr("Could not change version."));
        throw e;
    }
}

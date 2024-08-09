//  @file   abstract_converter.h
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
 **  @file   vabstractconverter.h
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef ABSTRACT_CONVERTER_H
#define ABSTRACT_CONVERTER_H

#include <qcompilerdetection.h>

#if !defined(Q_OS_OSX) && !defined(Q_OS_WIN) && defined(Q_CC_GNU)
#include <sys/sysmacros.h>
#endif

#include <QCoreApplication>
#include <QString>
#include <QTemporaryFile>
#include <QtGlobal>

#include "vdomdocument.h"

template <class Key, class T> class QMap;

#define CONVERTER_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

class VAbstractConverter :public VDomDocument
{
    Q_DECLARE_TR_FUNCTIONS(VAbstractConverter)
public:
    explicit        VAbstractConverter(const QString &fileName);
    virtual        ~VAbstractConverter() Q_DECL_EQ_DEFAULT;

    QString         Convert();

    int             getCurrentFormatVersion() const;
    QString         getVersionStr() const;

    static int      GetVersion(const QString &version);

protected:
    int             m_ver;
    QString         m_convertedFileName;

    void            ValidateInputFile(const QString &currentSchema) const;
    Q_NORETURN void InvalidVersion(int ver) const;
    void            Save();
    void            setVersion(const QString &version);

    virtual int     minVer() const =0;
    virtual int     maxVer() const =0;

    virtual QString minVerStr() const =0;
    virtual QString maxVerStr() const =0;

    virtual QString getSchema(int ver) const =0;
    virtual void    applyPatches() =0;
    virtual void    downgradeToCurrentMaxVersion() =0;

    virtual bool    isReadOnly() const =0;

    void            Replace(QString &formula, const QString &newName, int position,
                            const QString &token, int &bias) const;
    void            CorrectionsPositions(int position, int bias, QMap<int, QString> &tokens) const;
    static void     BiasTokens(int position, int bias, QMap<int, QString> &tokens);

private:
    Q_DISABLE_COPY(VAbstractConverter)

    QTemporaryFile  m_tmpFile;

    static void     ValidateVersion(const QString &version);
    void            saveBackupFile() const;
    static QString  removeBakExtension(const QString& fileName);
};

#endif // ABSTRACT_CONVERTER_H

/***************************************************************************************************
 **
 **  Copyright (c) 2012 Linas Valiukas and others.
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#ifndef FVAVAILABLEUPDATE_H
#define FVAVAILABLEUPDATE_H

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtGlobal>

class FvAvailableUpdate final : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit FvAvailableUpdate(QObject *parent = nullptr);

    auto GetEnclosureUrl() const -> QUrl;
    void SetEnclosureUrl(const QUrl &enclosureUrl);
    void SetEnclosureUrl(const QString &enclosureUrl);

    auto GetEnclosureVersion() const -> QString;
    void SetEnclosureVersion(const QString &enclosureVersion);

    auto GetEnclosurePlatform() const -> QString;
    void SetEnclosurePlatform(const QString &enclosurePlatform);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(FvAvailableUpdate) // NOLINT

    QUrl m_enclosureUrl;
    QString m_enclosureVersion;
    QString m_enclosurePlatform;
};

#endif // FVAVAILABLEUPDATE_H

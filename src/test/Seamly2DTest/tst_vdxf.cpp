// Copyright (C) 2026 Seamly2D contributors.
// SPDX-License-Identifier: GPL-3.0-or-later
// Seamly2D is free software under version 3 of the GNU General Public License,
// or any later version. It is distributed without any warranty. See LICENSE.

#include "tst_vdxf.h"
#include "../vdxf/vdxfpaintdevice.h"
#include "../vlayout/vlayoutpiece.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>
#include <limits>

namespace
{
VLayoutPiece squarePiece(const QString& name)
{
    VLayoutPiece piece;
    piece.SetName(name);
    piece.setMainPathPoints(QVector<QPointF>() << QPointF(0, 0) << QPointF(100, 0)
        << QPointF(100, 100) << QPointF(0, 100));
    return piece;
}

void configureDevice(VDxfPaintDevice& device, const QString& path)
{
    device.setFileName(path);
    device.setSize(QSize(1000, 1000));
    device.setVersion(DRW::AC1009);
}
}

void TST_VDxf::validLayers()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    VDxfPaintDevice device;
    const QString path = directory.filePath("valid.dxf");
    configureDevice(device, path);
    VLayoutPiece piece = squarePiece("Original square");
    piece.setSeamAllowancePoints(QVector<QPointF>() << QPointF(-10, -10) << QPointF(110, -10)
        << QPointF(110, 110) << QPointF(-10, 110));
    QVERIFY(device.ExportToAAMA(QVector<VLayoutPiece>() << piece));
    QVERIFY(device.exportError().isEmpty());
    QFile output(path);
    QVERIFY(output.open(QIODevice::ReadOnly));
    const QStringList lines = QString::fromUtf8(output.readAll()).split(QLatin1Char('\n'));
    bool cut_layer = false;
    bool stitch_layer = false;
    for (int i = 0; i + 1 < lines.size(); i += 2)
    {
        if (lines.at(i).trimmed() == QLatin1String("8"))
        {
            cut_layer |= lines.at(i + 1).trimmed() == QLatin1String("1");
            stitch_layer |= lines.at(i + 1).trimmed() == QLatin1String("8");
        }
    }
    QVERIFY(cut_layer);
    QVERIFY(stitch_layer);
}

void TST_VDxf::invalidPieces_data()
{
    QTest::addColumn<QVector<QPointF>>("points");
    QTest::newRow("empty") << QVector<QPointF>();
    QTest::newRow("two-points") << (QVector<QPointF>() << QPointF(0, 0) << QPointF(10, 0));
    QTest::newRow("collinear") << (QVector<QPointF>() << QPointF(0, 0) << QPointF(10, 0) << QPointF(20, 0));
    QTest::newRow("non-finite") << (QVector<QPointF>() << QPointF(0, 0) << QPointF(10, 0)
        << QPointF(10, std::numeric_limits<qreal>::quiet_NaN()));
}

void TST_VDxf::invalidPieces()
{
    QFETCH(QVector<QPointF>, points);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath("existing.dxf");
    QFile output(path);
    QVERIFY(output.open(QIODevice::WriteOnly));
    output.write("existing file");
    output.close();
    VLayoutPiece piece = squarePiece("Invalid piece");
    piece.setMainPathPoints(points);
    VDxfPaintDevice device;
    configureDevice(device, path);
    QVERIFY(!device.ExportToAAMA(QVector<VLayoutPiece>() << piece));
    QVERIFY(device.exportError().contains(QLatin1String("Invalid piece")));
    QVERIFY(output.open(QIODevice::ReadOnly));
    QCOMPARE(output.readAll(), QByteArray("existing file"));
}

void TST_VDxf::duplicateNames()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    VDxfPaintDevice device;
    const QString path = directory.filePath("duplicate.dxf");
    configureDevice(device, path);
    QVERIFY(!device.ExportToAAMA(QVector<VLayoutPiece>() << squarePiece("Front A") << squarePiece("FRONT_A")));
    QVERIFY(device.exportError().contains(QLatin1String("Duplicate")));
    QVERIFY(!QFile::exists(path));
}

void TST_VDxf::failedWrite()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    VDxfPaintDevice device;
    configureDevice(device, directory.path());
    QVERIFY(!device.ExportToAAMA(QVector<VLayoutPiece>() << squarePiece("Valid piece")));
    QVERIFY(device.exportError().contains(QLatin1String("Cannot write")));
}

void TST_VDxf::emptyExport()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    VDxfPaintDevice device;
    const QString path = directory.filePath("empty.dxf");
    configureDevice(device, path);
    QVERIFY(!device.ExportToAAMA(QVector<VLayoutPiece>()));
    QVERIFY(!device.exportError().isEmpty());
    QVERIFY(!QFile::exists(path));
    device.setSize(QSize(0, 100));
    QVERIFY(!device.ExportToAAMA(QVector<VLayoutPiece>() << squarePiece("Valid piece")));
    QVERIFY(!QFile::exists(path));
}

// SPDX-License-Identifier: GPL-3.0-or-later
#include "mainwindow.h"
#include "../seamly3d/model.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>
#include <cmath>

void MainWindow::openSeamly3D()
{
    Seamly3D::Project project;
    project.source=FileName();
    auto ids=pattern->DataPieces()->keys();
    std::sort(ids.begin(),ids.end());
    double spacing=0;
    try
    {
        for (quint32 id : ids)
        {
            const VPiece source=pattern->DataPieces()->value(id);
            if (!source.isInLayout()) continue;
            if (source.IsSeamAllowanceBuiltIn())
            {
                QMessageBox::warning(this,tr("Seamly3D"),tr("%1 has a built-in seam allowance. Define a separate sewing contour before exporting to 3D.").arg(source.GetName()));
                return;
            }
            Seamly3D::Piece piece;
            piece.id=QStringLiteral("piece-%1").arg(id); piece.name=source.GetName();
            const auto points=source.MainPathPoints(pattern);
            for (const auto &p : points)
            {
                const QPointF mm(FromPixel(p.x(),Unit::Mm),-FromPixel(p.y(),Unit::Mm));
                if (piece.outline.isEmpty() || QLineF(piece.outline.last(),mm).length()>1e-5) piece.outline << mm;
            }
            if (piece.outline.size()>1 && QLineF(piece.outline.first(),piece.outline.last()).length()<1e-5) piece.outline.removeLast();
            QRectF bounds;
            if (!piece.outline.isEmpty()) bounds=QPolygonF(piece.outline).boundingRect();
            spacing=qMax(spacing,qMax(bounds.width(),bounds.height())+30);
            project.pieces << piece;
        }
    }
    catch (const std::exception &exception)
    {
        QMessageBox::warning(this,tr("Seamly3D"),tr("Cannot evaluate pattern pieces: %1").arg(QString::fromUtf8(exception.what())));
        return;
    }
    if (project.pieces.isEmpty())
    {
        QMessageBox::information(this,tr("Seamly3D"),tr("Create at least one pattern piece and include it in the layout before opening the 3D preview."));
        return;
    }
    const int columns=int(std::ceil(std::sqrt(double(project.pieces.size()))));
    for (int i=0; i<project.pieces.size(); ++i)
        project.pieces[i].translation=QVector3D(float((i%columns)*spacing),float(-(i/columns)*spacing),float(i*2));
    QString message; Seamly3D::Assembly validation;
    if (!validation.reset(project,message))
    {
        QMessageBox::warning(this,tr("Seamly3D"),message); return;
    }
    const QString suggested=FileName().isEmpty() ? QStringLiteral("assembly.sm3d") : QFileInfo(FileName()).absolutePath()+QDir::separator()+QFileInfo(FileName()).completeBaseName()+QStringLiteral(".sm3d");
    QFileDialog dialog(this,tr("Export pieces for Seamly3D"),suggested,tr("Seamly3D projects (*.sm3d)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave); dialog.setDefaultSuffix(QStringLiteral("sm3d"));
    if (dialog.exec()!=QDialog::Accepted) return;
    QString path=dialog.selectedFiles().first();
    if (path.isEmpty()) return;
    if (!path.endsWith(QStringLiteral(".sm3d"),Qt::CaseInsensitive)) path+=QStringLiteral(".sm3d");
    // This file contains new geometry only. Keep existing assembly work safe.
    if (QFileInfo::exists(path))
    {
        Seamly3D::Project existing;
        if (Seamly3D::load(path,existing,message) && (!existing.seams.isEmpty() || existing.dressForm!=Seamly3D::DressForm()))
        {
            QMessageBox::warning(this,tr("Seamly3D"),tr("This file already contains assembly seams or dress form settings. Export to a new file, then use Import updated geometry in Seamly3D."));
            return;
        }
    }
    if (!Seamly3D::save(path,project,message)) { QMessageBox::warning(this,tr("Seamly3D"),message); return; }
    const QDir directory(QCoreApplication::applicationDirPath());
    QStringList candidates;
#ifdef Q_OS_MAC
    candidates << directory.filePath(QStringLiteral("../Helpers/seamly3d.app/Contents/MacOS/seamly3d"))
               << directory.filePath(QStringLiteral("seamly3d"))
               << directory.filePath(QStringLiteral("../../../seamly3d.app/Contents/MacOS/seamly3d"))
               << directory.filePath(QStringLiteral("../../../../../seamly3d/bin/seamly3d.app/Contents/MacOS/seamly3d"));
#elif defined(Q_OS_WIN)
    candidates << directory.filePath(QStringLiteral("seamly3d.exe"))
               << directory.filePath(QStringLiteral("../../seamly3d/bin/seamly3d.exe"));
#else
    candidates << directory.filePath(QStringLiteral("seamly3d"))
               << directory.filePath(QStringLiteral("../../seamly3d/bin/seamly3d"));
#endif
    candidates << QStandardPaths::findExecutable(QStringLiteral("seamly3d"));
    QString executable;
    for (const auto &candidate : candidates)
        if (!candidate.isEmpty() && QFileInfo(candidate).isExecutable()) { executable=QFileInfo(candidate).absoluteFilePath(); break; }
    if (executable.isEmpty() || !QProcess::startDetached(executable,QStringList{QFileInfo(path).absoluteFilePath()}))
        QMessageBox::information(this,tr("Seamly3D"),tr("Pieces exported to %1. Seamly3D could not be started; open this file from the companion app.").arg(path));
}

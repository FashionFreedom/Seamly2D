// Copyright (C) 2026 Seamly2D contributors.
// SPDX-License-Identifier: GPL-3.0-or-later
// This file is part of Seamly2D and is distributed without any warranty.

#ifndef TST_VDXF_H
#define TST_VDXF_H

#include <QObject>

class TST_VDxf : public QObject
{
    Q_OBJECT
private slots:
    void validLayers();
    void invalidPieces_data();
    void invalidPieces();
    void duplicateNames();
    void failedWrite();
    void emptyExport();
};

#endif // TST_VDXF_H

//---------------------------------------------------------------------------------------------------------------------
//   @file   vlayoutgenerator.cpp
//   @author Douglas S Caskey
//   @date   Nov 28, 2024
//
//   @copyright
//   Copyright (C) 2017 - 2024 Seamly, LLC
//   https://github.com/fashionfreedom/seamly2d
//
//   @brief
//   Seamly2D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Seamly2D is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//
//   @file   vlayoutgenerator.cpp
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   2 1, 2015
//
//   @brief
//   @copyright
//   This source code is part of the Valentina project, a pattern making
//   program, whose allow create and modeling patterns of clothing.
//   Copyright (C) 2013-2015 Valentina project
//   <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//   Valentina is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Valentina is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef VLAYOUTGENERATOR_H
#define VLAYOUTGENERATOR_H

#include <qcompilerdetection.h>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>
#include <memory>
#include <atomic>
#include <QMargins>

#include "vbank.h"
#include "vlayoutdef.h"

class QMarginsF;
class QGraphicsItem;
class VLayoutPaper;

class VLayoutGenerator :public QObject
{
    Q_OBJECT
public:
    explicit     VLayoutGenerator(QObject *parent = nullptr);
    virtual     ~VLayoutGenerator() Q_DECL_OVERRIDE;

    void         setPieces(const QVector<VLayoutPiece> &details);
    void         setLayoutGap(qreal width);
    void         setCaseType(Cases caseType);
    int          PieceCount();

    qreal        GetPaperHeight() const;
    void         SetPaperHeight(qreal value);

    qreal        GetPaperWidth() const;
    void         SetPaperWidth(qreal value);

    bool         IsUsePrinterFields() const;
    QMarginsF    GetPrinterFields() const;
    void         SetPrinterFields(bool usePrinterFields, const QMarginsF &value);

    quint32      GetShift() const;
    void         SetShift(quint32 shift);

    void         Generate();

    LayoutErrors State() const;

    Q_REQUIRED_RESULT QList<QGraphicsItem *> GetPapersItems() const;
    Q_REQUIRED_RESULT QList<QList<QGraphicsItem *>> getAllPieceItems() const;

    QVector<QVector<VLayoutPiece>> getAllPieces() const;

    bool         GetRotate() const;
    void         SetRotate(bool value);

    int          GetRotationIncrease() const;
    void         SetRotationIncrease(int value);

    bool         GetAutoCrop() const;
    void         SetAutoCrop(bool value);

    bool         IsSaveLength() const;
    void         SetSaveLength(bool value);

    bool         IsUnitePages() const;
    void         SetUnitePages(bool value);

    quint8       GetMultiplier() const;
    void         SetMultiplier(const quint8 &value);

    bool         useStripOptimization() const;
    void         setStripOptimization(bool value);

    bool         isTextAsPaths() const;
    void         setTextAsPaths(bool value);

signals:
    void         Start();
    void         Arranged(int count);
    void         Error(const LayoutErrors &state);
    void         Finished();

public slots:
    void         Abort();

private:
    Q_DISABLE_COPY(VLayoutGenerator)
    QVector<VLayoutPaper> m_papers;
    VBank                *m_bank;
    qreal                 m_paperHeight;
    qreal                 m_paperWidth;
    QMarginsF             m_margins;
    bool                  m_usePrinterFields;
    std::atomic_bool      m_stopGeneration;
    LayoutErrors          m_state;
    quint32               m_shift;
    bool                  m_rotate;
    int                   m_rotationIncrement;
    bool                  m_autoCrop;
    bool                  m_saveLength;
    bool                  m_unitePages;
    bool                  m_stripOptimized;
    quint8                m_multiplier;
    bool                  m_useStripOptimization;
    bool                  m_textAsPaths;

    int                   pageHeight() const;
    int                   pageWidth() const;

    void                  gatherPages();
    void                  unitePages();
    void                  unitePieces(int j, QList<QList<VLayoutPiece> > &pieces, qreal length, int i);
    void                  unitePapers(int j, QList<qreal> &papersLength, qreal length);
    QList<VLayoutPiece>   movePieces(qreal length, const QVector<VLayoutPiece> &pieces);
};

#endif // VLAYOUTGENERATOR_H

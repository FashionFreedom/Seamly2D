/******************************************************************************
 *   @file   grade_breaks_dialog.h
 **  @author Claude Code
 **  @date   29 Mar, 2026
 **
 **  @brief  Dialog for editing piecewise (non-linear) grade breaks per
 **          measurement, for size or height dimension.
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2026 Seamly2D project
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

#ifndef GRADE_BREAKS_DIALOG_H
#define GRADE_BREAKS_DIALOG_H

#include <QDialog>
#include <QMap>
#include <QVector>
#include <QDoubleSpinBox>

#include "../vmisc/def.h"
#include "../vpatterndb/variables/grade_break.h"
#include "../vformat/measurements.h"

class GradeBreaksDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Dimension { Size, Height };

    GradeBreaksDialog(Dimension dim,
                      const QString &measurementName,
                      const QStringList &activeValues,
                      qreal baseValue,
                      qreal defaultIncrement,
                      qreal baseMeasurement,
                      const QVector<GradeBreak> &existingBreaks,
                      Unit unit,                      
                      const QMap<int, QString> &aliases = QMap<int, QString>(),
                      QWidget *parent = nullptr);
    ~GradeBreaksDialog();

    QVector<GradeBreak> gradeBreaks() const;

private slots:
    void incrementChanged(int row);
    void resetAll();
    void updatePreview();

private:
    Q_DISABLE_COPY(GradeBreaksDialog)

    Dimension           m_dimension;
    QStringList         m_activeValues;
    qreal               m_baseValue;
    qreal               m_defaultIncrement;
    qreal               m_baseMeasurement;
    Unit                m_unit;
    QMap<int, QString>  m_aliases;
    QVector<QDoubleSpinBox *> m_spinBoxes;
    class QLabel       *m_previewLabel;

    void setupUI(const QString &measurementName, const QVector<GradeBreak> &existingBreaks);
    qreal calcPreviewValue(qreal targetValue) const;
    int   decimals() const;
    qreal singleStep() const;
};

#endif // GRADE_BREAKS_DIALOG_H

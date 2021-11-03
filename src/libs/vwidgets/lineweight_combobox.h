/******************************************************************************
 *   @file   lineweight_combobox.h
 **  @author DS Caskey
 **  @date   Nov 2, 2021
 **
 **  @brief
 **  @copyright
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
 *****************************************************************************/


#ifndef LINEWEIGHT_COMBOBOX_H
#define LINEWEIGHT_COMBOBOX_H

#include <QComboBox>
#include <QWidget>

/**
 * A comboBox for choosing a line weight.
 */
class LineWeightComboBox: public QComboBox
{
    Q_OBJECT

public:
                     LineWeightComboBox(QWidget *parent = nullptr, const char *name = nullptr);
                     LineWeightComboBox(int width, int height,	QWidget *parent = nullptr, const char *name=nullptr);
    virtual         ~LineWeightComboBox();

    void             init();

    qreal            getLineWeight() const;
    void             setLineWeight(const qreal &weight);

private slots:
    void             updateLineWeight(int index);

signals:
    void             lineWeightChanged(const qreal &weight);

private:
    QIcon            createIcon(const qreal &width);
    qreal            m_currentWeight;
    int              m_iconWidth;
    int              m_iconHeight;
};

#endif

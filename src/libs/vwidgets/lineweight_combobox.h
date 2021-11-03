/******************************************************************************
 *   linewidth_combobox.cpp                                                 *
 *****************************************************************************/

#ifndef LINEWEIGHT_COMBOBOX_H
#define LINEWEIGHT_COMBOBOX_H

#include <QComboBox>
#include <QWidget>

#include "dc.h"

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

    DC2::LineWeight  getLineWeight() const;
    void             setLineWeight(DC2::LineWeight wight);

private slots:
    void             updateLineWeight(int index);

signals:
    void             lineWeightChanged(DC2::LineWeight);

private:
    DC2::LineWeight  m_currentWeight;
    int              m_iconWidth;
    int              m_iconHeight;
};

#endif

/******************************************************************************
 *   color_combobox.h                                                         *
 *****************************************************************************/

#ifndef COLOR_COMBOBOX_H
#define COLOR_COMBOBOX_H

#include <QComboBox>
#include <QString>
#include <QWidget>
#include <memory>

/**
 * A comboBox for choosing a color.
 */
class ColorComboBox: public QComboBox
{
    Q_OBJECT

public:
             ColorComboBox(QWidget *parent = nullptr , const char *name = nullptr );
             ColorComboBox(int width, int height, QWidget *parent = nullptr , const char *name = nullptr );
    virtual ~ColorComboBox();

    QString  getColor() const;
    void     setColor(const QString &color);
    void     init();
    int      getIconWidth();
    int      getIconHeight();

private slots:
    void     colorChanged(int index);

signals:
    void     colorChangedSignal(const QString &color);

private:
    QString  m_currentColor;
    int      m_colorIndexStart;
    int      m_iconWidth;
    int      m_iconHeight;
};

#endif

#ifndef VABSTRACTLAYOUTDIALOG_H
#define VABSTRACTLAYOUTDIALOG_H

#include "../ifc/ifcdef.h"

#include <QComboBox>
#include <QDialog>



class VAbstractLayoutDialog : public QDialog
{
public:
    enum class PaperSizeTemplate : char { A0 = 0,
                                          A1,
                                          A2,
                                          A3,
                                          A4,
                                          Letter,
                                          Legal,
                                          Roll24in = 7, // Be carefull when change order roll type
                                          Roll30in,     // Used also for showing icon
                                          Roll36in,
                                          Roll42in,
                                          Roll44in = 11,
                                          Custom = 12};

    explicit VAbstractLayoutDialog(QWidget *parent = nullptr);

protected:
        typedef QStringList FormatsVector;
        const static  FormatsVector pageFormatNames;
        typedef int VIndexType;

        QSizeF GetTemplateSize(const PaperSizeTemplate &tmpl, const Unit &unit) const;
        QSizeF RoundTemplateSize(qreal width, qreal height, Unit unit) const;

        void InitTemplates(QComboBox *comboBoxTemplates);


};

#endif // VABSTRACTDIALOG_H

/******************************************************************************
 *   @file   vformulapropertyeditor.cpp
 **  @author DS Caskey
 **  @date   Feb 18, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   vformulapropertyeditor.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vformulapropertyeditor.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <QKeyEvent>
#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QRegularExpression>

#include "../vpropertyexplorer/vproperty.h"
#include "../vtools/dialogs/support/edit_formula_dialog.h"

// VFormulaPropertyEditor
//---------------------------------------------------------------------------------------------------------------------
VFormulaPropertyEditor::VFormulaPropertyEditor(QWidget *parent)
    : QWidget(parent)
    , formula(VFormula())
    , toolButton(nullptr)
    , textLabel(nullptr)
    , spacer(nullptr)
{
    setAutoFillBackground(true);

    // Create the text label
    textLabel = new QLabel(this);
    textLabel->setText(formula.getStringValue());
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // spacer (this is needed for proper display of the label and button)
    spacer = new QSpacerItem(1, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Create the tool button and make it the focus proxy
    toolButton = new QToolButton(this);
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolButton->setText("...");
    toolButton->setIcon(QIcon("://icon/16x16/fx.png"));
    toolButton->setFixedWidth(18);
    toolButton->installEventFilter(this);
    setFocusProxy(toolButton);
    setFocusPolicy(toolButton->focusPolicy());
    connect(toolButton, &QToolButton::clicked, this, &VFormulaPropertyEditor::onToolButtonClicked);

    // Add label, spacer & button to a horizontal layout)
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(3);
    layout->setMargin(0);
    layout->addWidget(textLabel);
    layout->addItem(spacer);
    layout->addWidget(toolButton);
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaPropertyEditor::SetFormula(const VFormula& formula)
{
    if (this->formula != formula)
    {
        this->formula = formula;
        textLabel->setText(this->formula.getStringValue());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaPropertyEditor::onToolButtonClicked()
{
    EditFormulaDialog* dialog = new EditFormulaDialog(formula.getData(), formula.getToolId(),
                                                                qApp->getMainWindow());
    dialog->setCheckZero(formula.getCheckZero());
    dialog->setPostfix(formula.getPostfix());
    dialog->SetFormula(formula.GetFormula(FormulaType::FromUser));

    if (dialog->exec() == QDialog::Accepted)
    {
        formula.SetFormula(dialog->GetFormula(), FormulaType::ToUser);
        textLabel->setText(formula.getStringValue());
        delete dialog;
        emit dataChangedByUser(formula, this);
        VPE::UserChangeEvent *event = new VPE::UserChangeEvent();
        QCoreApplication::postEvent ( this, event );
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VFormulaPropertyEditor::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == toolButton && event->type() == QEvent::KeyPress)
    {
        // Ignore the event, so that eventually the delegate gets the event.
        event->ignore();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VFormulaPropertyEditor::GetFormula() const
{
    return formula;
}

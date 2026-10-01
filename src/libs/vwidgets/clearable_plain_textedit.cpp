#include "clearable_plain_textedit.h"
#include <QStyle>
#include <QTextCursor>
#include <QResizeEvent>

ClearablePlainTextEdit::ClearablePlainTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setUndoRedoEnabled(true);

    // 1. Create the clear button using the standard native asset
    m_clearButton = new QPushButton(this);
    QIcon standardIcon = style()->standardIcon(QStyle::SP_LineEditClearButton);
    m_clearButton->setIcon(standardIcon);
    m_clearButton->setCursor(Qt::ArrowCursor);
    m_clearButton->setFixedSize(24, 24);
    m_clearButton->setVisible(false);

    // We modify only the button's properties to keep it borderless
    m_clearButton->setFlat(true);

    // 2. Set viewport margins natively to prevent text from overlapping the button area.
    // This leaves the QPlainTextEdit stylesheet completely untouched so native icons work.
    setViewportMargins(0, 0, 28, 0);

    // 3. Connect signals
    connect(this, &QPlainTextEdit::textChanged, this, &ClearablePlainTextEdit::toggleClearButton);
    connect(m_clearButton, &QPushButton::clicked, this, &ClearablePlainTextEdit::undoableClear);
}

void ClearablePlainTextEdit::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);

    // 4. Calculate position relative to the viewport right edge
    int x = width() - m_clearButton->width() - 4;
    int y = (viewport()->height() - m_clearButton->height()) / 2; // Centers it vertically

    m_clearButton->move(x, y);
}

void ClearablePlainTextEdit::toggleClearButton()
{
    m_clearButton->setVisible(!toPlainText().isEmpty());
}

void ClearablePlainTextEdit::undoableClear()
{
    QTextCursor cursor = textCursor();

    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    cursor.endEditBlock();

    setFocus();
}

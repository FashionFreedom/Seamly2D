#ifndef CLEARABLE_PLAIN_TEXT_EDIT_H
#define CLEARABLE_PLAIN_TEXT_EDIT_H

#include <QPlainTextEdit>
#include <QPushButton>

class ClearablePlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit     ClearablePlainTextEdit(QWidget* parent = nullptr);
    virtual     ~ClearablePlainTextEdit() = default;

protected:
    virtual void resizeEvent(QResizeEvent* event) override;

private slots:
    void         toggleClearButton();
    void         undoableClear();

private:
    QPushButton *m_clearButton;
};

#endif // CLEARABLE_PLAIN_TEXT_EDIT_H

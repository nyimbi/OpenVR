#include "lineedit.h"

LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent)
{

}

void LineEdit::focusOutEvent(QFocusEvent *evnt)
{
        QLineEdit::focusOutEvent(evnt);
        if (evnt->lostFocus()) {
                emit focusOut(text());
        }
}

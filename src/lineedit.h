#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QFocusEvent>

class LineEdit : public QLineEdit
{    
    Q_OBJECT

public:
    explicit LineEdit(QWidget *parent = 0);
    void focusOutEvent(QFocusEvent *);

signals:
    void focusOut(QString);

public slots:

};

#endif // LINEEDIT_H

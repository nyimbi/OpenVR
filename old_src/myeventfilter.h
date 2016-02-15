#ifndef MYEVENTFILTER_H
#define MYEVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QDebug>

class MyEventFilter : public QObject
{
    Q_OBJECT
protected:
  bool eventFilter(QObject *obj, QEvent *ev)
  {
    if(ev->type() == QEvent::KeyPress ||
       ev->type() == QEvent::MouseMove){
         // now reset your timer, for example
         //resetMyTimer();
       // qDebug()<< " Event Detected ";
        emit(userActivityDetected());
    }

    return QObject::eventFilter(obj, ev);
  }
public:
    explicit MyEventFilter(QObject *parent = 0);
    int lastKnownActivity;

signals:
   void userActivityDetected();

public slots:
    int checkLastActivity();
};

#endif // MYEVENTFILTER_H

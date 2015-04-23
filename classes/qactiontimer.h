#ifndef QACTIONTIMER_H
#define QACTIONTIMER_H
#include <QAbstractEventDispatcher>
#include <QTimerEvent>

static const int INV_TIMER = -1;                // invalid timer id

class QSingleActionTimer : public QObject
{
    Q_OBJECT

public:
    QSingleActionTimer(void* data, int msec, const QObject *receiver, const char *member) :
        QObject(QAbstractEventDispatcher::instance()), hasValidReceiver(true), slotObj(0)
    {
        this->data = data;
        this->timerId = QObject::startTimer(msec);
        connect(this, SIGNAL(timeout(void*)), receiver, member);
    }

Q_SIGNALS:
    void timeout(void* data);

protected:
    void timerEvent(QTimerEvent *)
    {
        // need to kill the timer _before_ we emit timeout() in case the
        // slot connected to timeout calls processEvents()
        if (timerId > 0)
            QObject::killTimer(timerId);
        timerId = -1;

        emit timeout(this->data);

        // we would like to use delete later here, but it feels like a
        // waste to post a new event to handle this event, so we just unset the flag
        // and explicitly delete...
        delete this;
        //qDeleteInEventHandler(this);
    }

private:
    void* data;
    int timerId;
    bool hasValidReceiver;
    QtPrivate::QSlotObjectBase *slotObj;
};

class QActionTimer : public QObject
{
    Q_OBJECT

public:
    QActionTimer(void* data, int msec, const QObject *receiver, const char *member) :
        QObject(QAbstractEventDispatcher::instance()), hasValidReceiver(true), slotObj(0)
    {
        this->data = data;
        this->timerId = QObject::startTimer(msec);
        connect(this, SIGNAL(timeout(void*)), receiver, member);
    }
    ~QActionTimer()
    {
        stop();
    }
    void stop()
    {
        if (timerId != INV_TIMER)
        {
            QObject::killTimer(timerId);
            timerId = INV_TIMER;
        }
    }

Q_SIGNALS:
    void timeout(void* data);

protected:
    void timerEvent(QTimerEvent *e)
    {
        if (e->timerId() == timerId)
        {
            emit timeout(this->data);
        }
    }

private:
    void* data;
    int timerId;
    bool hasValidReceiver;
    QtPrivate::QSlotObjectBase *slotObj;
};

#endif // QACTIONTIMER_H

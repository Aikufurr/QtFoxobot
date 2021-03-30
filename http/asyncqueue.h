#ifndef ASYNCQUEUE_H
#define ASYNCQUEUE_H

#include <QEventLoop>
#include <QFuture>
#include <QList>
#include <QObject>
#include <QtConcurrent/QtConcurrent>

class AsyncQueue : public QObject {
    Q_OBJECT

public:
    AsyncQueue();
    int remaining();
    QFuture<int> wait();
    void shift();

private:
    QList<QFuture<int> *> futures;

};

#endif // ASYNCQUEUE_H

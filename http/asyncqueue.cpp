#include "asyncqueue.h"

AsyncQueue::AsyncQueue() {

}

int AsyncQueue::remaining() {
    return this->futures.size();
}

int _delayFor() {
    qDebug() << "loop1";
    QEventLoop loop;
    qDebug() << "loop2";
//    loop.exec();
    qDebug() << "loop3";
    return 0;
};
int _noop() {
    return 0;
};

QFuture<int> AsyncQueue::wait() {
    // Will this work? honestly, I don't know.
//    qDebug() << "wait";
    QFuture<int> *next;
    if (this->futures.isEmpty()) {
//        qDebug() << "wait1";
        QFuture<int> temp = QtConcurrent::run(_noop);
        next = &temp;
    } else {
//        qDebug() << "wait2";
        next = this->futures[this->futures.size()-1];
    }

//    qDebug() << "wait3";
    QFuture<int> future = QtConcurrent::run(_delayFor);

    this->futures.push_back(&future);

    return *next;
}

void AsyncQueue::shift() {
    if (!this->futures.isEmpty()) {
        qDebug() << "wait4";
        /*QFuture<int> *next = */this->futures.takeFirst();
        qDebug() << "wait5";
//        next->cancel();
    }
}

#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "apirequest.h"
#include "asyncqueue.h"
#include "../util/util.h"
#include <QDateTime>
#include <QDebug>
#include <QFuture>
#include <QNetworkReply>
#include <QObject>
#include <limits>

class RequestHandler : public QObject {
    Q_OBJECT

public:
    RequestHandler(QFuture<int> *globalTimeout, int restTimeOffset, int retryLimit);
    bool limited();
    bool _inactive();
    QByteArray push(APIRequest *apiRequest);
    QByteArray execute(APIRequest *apiRequest);


private:
    qint64 reset;
    int remaining;
    int limit;
    int retryAfter;
    QFuture<int> *globalTimeout;
    bool delaying = false;
    int restTimeOffset;
    int retryLimit;
    AsyncQueue *queue;
    qint64 getAPIOffset(QDateTime *serverDate);
    qint64 calculateReset(qint64 reset, QDateTime *serverDate);

};

#endif // REQUESTHANDLER_H

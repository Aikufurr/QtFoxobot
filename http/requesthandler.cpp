#include "requesthandler.h"

RequestHandler::RequestHandler(QFuture<int> *globalTimeout, int restTimeOffset, int retryLimit) {
    queue = new AsyncQueue();
    this->reset = -1;
    this->remaining = -1;
    this->limit = -1;
    this->retryAfter = -1;
    this->globalTimeout = globalTimeout;
    this->restTimeOffset = restTimeOffset;
    this->retryLimit = retryLimit;
}

qint64 RequestHandler::getAPIOffset(QDateTime *serverDate) {
    return serverDate->toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch();
}

qint64 RequestHandler::calculateReset(qint64 reset, QDateTime *serverDate) {
    return QDateTime::fromMSecsSinceEpoch(reset * 1000).toMSecsSinceEpoch() - this->getAPIOffset(serverDate);
}

bool RequestHandler::limited()  {
    return (this->globalTimeout->isRunning()) || (this->remaining <= 0 && QDateTime::currentMSecsSinceEpoch() < this->reset);
}

bool RequestHandler::_inactive() {
    return this->queue->remaining() == 0 && !this->limited();
}

QByteArray RequestHandler::push(APIRequest *apiRequest) {
//    qDebug() << "debug 8";
//    queue->wait();
//    qDebug() << "debug 9";
    QByteArray result = this->execute(apiRequest); // Get return type
//    qDebug() << "debug 10";
//    queue->shift();
//    qDebug() << "debug 11";
    return result;
}

QByteArray RequestHandler::execute(APIRequest *request) {
    if (this->limited()) {
        const qint64 timeout = this->reset + this->restTimeOffset - QDateTime::currentMSecsSinceEpoch();

        //        this->globalTimeout == 0 ? Util::delayFor(*this->globalTimeout) : Util::delayFor(timeout);
        if (this->globalTimeout->isRunning()) {
            this->globalTimeout->result();
        } else {
            QFuture<int> future = Util::delayFor(timeout);
            this->globalTimeout = &future;
        }
    }

    APIRequest::payload_t res = request->make();

    // If network error
    if (res.error) {
        if (request->retries >= this->retryLimit) {
            qDebug() << "{RequestHandler::execute::error}" << res.status << res.errorString << res.data;
            return "";
        }

        request->retries++;
        return this->execute(request);
    }

    // If response contains headers
    if (!res.rawHeaderHash.isEmpty()) {
        QDateTime serverDate = QDateTime::fromString(res.rawHeaderHash["date"].mid(5, 20), "dd MMM yyyy hh:mm:ss");
        // https://discord.com/developers/docs/topics/rate-limits
        const int limit = res.rawHeaderHash["x-ratelimit-limit"].toInt(); // The number of requests that can be made
        const int remaining = res.rawHeaderHash["x-ratelimit-remaining"].toInt(); // The number of remaining requests that can be made
        const qint64 reset = res.rawHeaderHash["x-ratelimit-reset"].toLongLong(); // Epoch time (seconds since 00:00:00 UTC on January 1, 1970) at which the rate limit resets
        const int retryAfter = res.rawHeaderHash["retry-after"].toInt(); // Total time (in seconds) of when the current rate limit bucket will reset. Can have decimals to match previous millisecond ratelimit precision

        this->limit = limit ? limit : std::numeric_limits<int>::max();
        this->remaining = remaining ? remaining : 1;
        this->reset = reset ? calculateReset(reset, &serverDate) : QDateTime::currentMSecsSinceEpoch();
        this->retryAfter = retryAfter ? retryAfter * 1000 : -1;

        if (request->route.contains("reactions")) {
            this->reset = serverDate.toMSecsSinceEpoch() - getAPIOffset(&serverDate) + 250;
        }

        // Handle global ratelimit
        // Returned only on a HTTP 429 response if the rate limit headers returned are of the global rate limit (not per-route)
        if (res.rawHeaderHash.contains("x-ratelimit-global")) {
            QFuture<int> future = Util::delayFor(this->retryAfter);
            this->globalTimeout = &future;
            future.result();
        }
    }

    // Handles 2xx to 3xx
    if (res.status >= 200 &&
            res.status < 400) {
        return res.data;
    }

    // Handles 4xx
    if (res.status >= 400 &&
            res.status < 500) {
        // Handles 429
        if (res.status == 429) {
            QFuture<int> future = Util::delayFor(this->retryAfter);
            this->globalTimeout = &future;
            future.result();
            return this->execute(request);
        }

        // If data
        if (!res.data.isEmpty()) {
            return res.data;
        } else {
            qDebug() << "{RequestHandler::execute::error}" << res.status << res.errorString;
            return "";
        }

        qDebug() << "{RequestHandler::execute::error}" << res.status << res.errorString;
        return "";
    }

    // Handles 5xx
    if (res.status >= 500 &&
            res.status < 600) {
        if(request->retries >= this->retryLimit) {
            qDebug() << "{RequestHandler::execute::error}" << res.status << res.errorString;
            return "";
        }

        request->retries++;
        return this->execute(request);
    }

    return "";
}

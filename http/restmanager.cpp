#include "restmanager.h"

RESTManager::RESTManager(QString token, int restSweepMSInterval, int restTimeOffset, int retryLimit) {
    this->token = token;
    this->restTimeOffset = restTimeOffset;
    this->retryLimit = retryLimit;
    QTimer *sweep = new QTimer(this);
    QObject::connect(sweep, &QTimer::timeout, this, [=](){
        QHash<QString, RequestHandler *>::iterator i = this->handlers.begin();
        while (i != this->handlers.end()) {
            if (i.value()->_inactive()) {
                i = this->handlers.erase(i);
            } else {
                ++i;
            }
        }
    });
    sweep->start(restSweepMSInterval);
    this->globalTimeout = new QFuture<int>;
}


QByteArray RESTManager::request(QString method, QString url) {
    QJsonObject body;
    return this->request(method, url, body);
}

QByteArray RESTManager::request(QString method, QString url, QJsonObject body) {
    qDebug() << "Function: " << Q_FUNC_INFO << " :->" << method << url;
    QJsonObject options;
    QJsonObject http {
        {"version", 8},
        {"api", "https://discord.com/api"},
        {"cdn", "https://cdn.discordapp.com"},
        {"invite",  "https://discord.gg"},
        {"template", "https://discord.new"}
    };
    options.insert("http", http);
    options.insert("auth", this->token);
    options.insert("pathed", false);

    if (body.contains("data")) {
        options.insert("data", body["data"].toObject());
    }

//    qDebug() << "debug 4";
    APIRequest *apiRequest = new APIRequest(method, url, options);

    RequestHandler *handler;

    if (this->handlers.contains(apiRequest->route)) {
//        qDebug() << "debug 5";
        handler = this->handlers[apiRequest->route];
    } else {
//        qDebug() << "debug 6";
        handler = new RequestHandler(this->globalTimeout, this->restTimeOffset, this->retryLimit); // Get what parameters are needing to be passed from "(this)"
        this->handlers.insert(apiRequest->route, handler);
    }

    // delete apiRequest;

//    qDebug() << "debug 7";
    return handler->push(apiRequest);
//    qDebug() << "debug 20";
}

QByteArray RESTManager::request(QString method, QString url, QJsonObject body, QHash<QByteArray, QByteArray> files) {
    QJsonObject options;
    QJsonObject http {
        {"version", 8},
        {"api", "https://discord.com/api"},
        {"cdn", "https://cdn.discordapp.com"},
        {"invite",  "https://discord.gg"},
        {"template", "https://discord.new"}
    };
    options.insert("http", http);
    options.insert("auth", this->token);
    options.insert("pathed", false);

    if (body.contains("data")) {
        options.insert("data", body["data"].toObject());
    }
    APIRequest *apiRequest;
    // If files
    if (!files.isEmpty()) {
        apiRequest = new APIRequest(method, url, options, files);
    } else {
        apiRequest = new APIRequest(method, url, options);
    }

    RequestHandler *handler;

    if (this->handlers.contains(apiRequest->route)) {
        handler = this->handlers[apiRequest->route];
    } else {
        handler = new RequestHandler(this->globalTimeout, this->restTimeOffset, this->retryLimit); // Get what parameters are needing to be passed from "(this)"
        this->handlers.insert(apiRequest->route, handler);
    }

    // delete apiRequest;

    return handler->push(apiRequest);
}

QByteArray RESTManager::custom_request(QString method, QString url, QJsonObject body) {
    QJsonObject options;
    QJsonObject http {
        {"version", 8},
        {"api", "https://discord.com/api"},
        {"cdn", "https://cdn.discordapp.com"},
        {"invite",  "https://discord.gg"},
        {"template", "https://discord.new"}
    };
    options.insert("http", http);
    options.insert("pathed", true);

    if (body.contains("data")) {
        options.insert("data", body["data"].toObject());
    }

    APIRequest *apiRequest = new APIRequest(method, url, options);

    RequestHandler *handler;

    if (this->handlers.contains(apiRequest->route)) {
        handler = this->handlers[apiRequest->route];
    } else {
        handler = new RequestHandler(this->globalTimeout, this->restTimeOffset, this->retryLimit); // Get what parameters are needing to be passed from "(this)"
        this->handlers.insert(apiRequest->route, handler);
    }

    // delete apiRequest;

    return handler->push(apiRequest);
}

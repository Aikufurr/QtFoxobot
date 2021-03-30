#ifndef RESTMANAGER_H
#define RESTMANAGER_H

#include "apirequest.h"
#include "requesthandler.h"
#include <QByteArray>
#include <QFuture>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTimer>

class RESTManager : public QObject {
    Q_OBJECT

public:
    RESTManager(QString token, int restSweepMSInterval, int restTimeOffset, int retryLimit);
    QByteArray request(QString method, QString url);
    QByteArray request(QString method, QString url, QJsonObject body);
    QByteArray request(QString method, QString url, QJsonObject body, QHash<QByteArray, QByteArray> files);
    QByteArray custom_request(QString method, QString url, QJsonObject body);

private:
//    QHash handlers; // Find Hash Type
    QString token;
    QHash<QString, RequestHandler *> handlers;
    QFuture<int> *globalTimeout;
    int restTimeOffset;
    int retryLimit;
};

#endif // RESTMANAGER_H

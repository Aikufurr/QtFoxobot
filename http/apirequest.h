#ifndef APIREQUEST_H
#define APIREQUEST_H

#include <QByteArray>
#include <QEventLoop>
#include <QHash>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUuid>
#include <QVariantMap>



class APIRequest : public QObject {
    Q_OBJECT

public:
    struct payload_t {
        bool error;
        QString errorString;
        int status;
        QHash<QByteArray, QByteArray> rawHeaderHash;
        QByteArray data;
    };

    APIRequest(QString method, QString path, QJsonObject options);
    APIRequest(QString method, QString path, QJsonObject options, QHash<QByteArray, QByteArray> files);
    payload_t make();
    
    QString route;
    int retries;

private:
    QByteArray method;
    QJsonObject options;
    QString path;
    QHash<QByteArray, QByteArray> files;

};

#endif // APIREQUEST_H

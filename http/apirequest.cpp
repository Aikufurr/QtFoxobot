#include "apirequest.h"

APIRequest::APIRequest(QString method, QString path, QJsonObject options) {
    this->method = method.toUpper().toUtf8();
//    this->route = options["route"].toString();
    this->options = options;
    this->retries = 0;

    this->path = path;
    this->route = path;

    // options.query = [
    // ["key": "value"]
    // ]
    if (options.contains("query")) {
        QString queryString("?");
        foreach(const QJsonValue &value, options["query"].toArray()) {
            QJsonArray pair = value.toArray();
            queryString = QString("%1%2=%3&").arg(queryString,
                                                  pair[0].toString(),
                    pair[1].toString());

        }
        this->path += queryString.midRef(0, queryString.size()-1);
    }

}

APIRequest::APIRequest(QString method, QString path, QJsonObject options, QHash<QByteArray, QByteArray> files) {
    this->method = method.toUpper().toUtf8();
//    this->route = options["route"].toString();
    this->options = options;
    this->retries = 0;
    this->files = files;

    this->path = path;
    this->route = path;

    // options.query = [
    // ["key": "value"]
    // ]
    if (options.contains("query")) {
        QString queryString("?");
        foreach(const QJsonValue &value, options["query"].toArray()) {
            QJsonArray pair = value.toArray();
            queryString = QString("%1%2=%3&").arg(queryString,
                                                  pair[0].toString(),
                    pair[1].toString());

        }
        this->path += queryString.midRef(0, queryString.size()-1);
    }

}

APIRequest::payload_t APIRequest::make() {
    QString API = "";
    if (this->options["pathed"].toBool() == false) {
        if (this->options.contains("versioned") == false) {
            API = this->options["http"].toObject()["api"].toString();
        } else if (this->options["versioned"].toBool() == false) {
            API = this->options["http"].toObject()["api"].toString();
        } else {
            API = this->options["http"].toObject()["api"].toString() + "/v" + this->options["http"].toObject()["version"].toString();
        }
    }

    const QString url = API + this->path;
    QNetworkRequest *request = new QNetworkRequest(QUrl(url));
    request->setRawHeader("User-Agent", "FoxoBot/1.0 (windows/linux)");
    request->setRawHeader("Keep-Alive", "true");

    if (this->options.contains("headers")) {
        for(QJsonObject::const_iterator it = this->options["headers"].toObject().constBegin(); it != this->options["headers"].toObject().constEnd(); it++) {
            request->setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
        }
    }
    //    qDebug() << this->options;
    //    qDebug() << this->options["auth"].toString().toUtf8();
    if (this->options.contains("auth")) {
        request->setRawHeader("Authorization", this->options["auth"].toString().toUtf8());
    }
    if (this->options.contains("reason")) {
        request->setRawHeader("X-Audit-Log-Reason", QUrl::toPercentEncoding(this->options["reason"].toString()));
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply;

    if (!this->files.isEmpty()) {
        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        if (this->options.contains("data")) {
            QJsonDocument doc(this->options["data"].toObject());
            QByteArray data = doc.toJson(QJsonDocument::Compact);
            QHttpPart dataPart;
            /* payload_json */
            dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"payload_json\""));
            dataPart.setBody(data);
            multiPart->append(dataPart);
        }

        int i = 0;
        QHash<QByteArray, QByteArray>::const_iterator iterator = this->files.constBegin();
        while (iterator != this->files.constEnd()) {

            //            for(int i = 0; i < this->options["files"].toArray().size(); i++) {
            //                // Message file
            //                payload.data += R"**(Content-Disposition: form-data; name="file)**";
            //                payload.data += i+1;
            //                payload.data += R"**("; filename=")**";
            //                payload.data += this->options["file_names"].toArray()[i].toString().toUtf8();
            //                payload.data += R"**(")**";
            //                payload.data += "\n";
            //                payload.data += file;
            //                payload.data += "\n";

            QString ContentDispositionHeader = QString(R"**(form-data; name="file%1"; filename="%2")**").arg(QString::number(++i), QString(iterator.key()));

            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(ContentDispositionHeader));
            filePart.setHeader(QNetworkRequest::ContentLengthHeader, iterator.value().size());

            filePart.setBody(iterator.value());
            multiPart->append(filePart);

            ++iterator;
        }



        reply = manager->sendCustomRequest(*request, this->method, multiPart);



        //            payload_t payload;
        //            payload.request = *request;
        //            payload.method = this->method;

        //            // https://discord.com/developers/docs/resources/channel#create-message-example-request-bodies-multipartformdata
        //            QByteArray boundry = "Boundry.FoxoBot." + QUuid::createUuid().toString().replace("{", "").replace("}", "").replace("-", "").toUtf8();
        //            request->setRawHeader("Content-Type", "multipart/form-data; boundary=" + boundry);

        //            // Boundry
        //            payload.data = "--";
        //            payload.data += boundry;
        //            payload.data += "\n";

        //            // If the message has data
        //            if (this->options.contains("data")) {
        //                QJsonDocument doc(this->options["data"].toObject());
        //                QByteArray data = doc.toJson(QJsonDocument::Compact);

        //                // Message content
        //                payload.data += R"**(Content-Disposition: form-data; name="payload_json")**";
        //                payload.data += "\n";
        //                payload.data += data;
        //                payload.data += "\n";

        //                // Boundry
        //                payload.data = "--";
        //                payload.data += boundry;
        //                payload.data += "\n";
        //            }


        //            // Content-Length ?
        //            for(int i = 0; i < this->options["files"].toArray().size(); i++) {
        //                // Message file
        //                QByteArray file = this->options["files"].toArray()[i].toString().toUtf8();

        //                payload.data += R"**(Content-Disposition: form-data; name="file)**";
        //                payload.data += i+1;
        //                payload.data += R"**("; filename=")**";
        //                payload.data += this->options["file_names"].toArray()[i].toString().toUtf8();
        //                payload.data += R"**(")**";
        //                payload.data += "\n";
        //                payload.data += file;
        //                payload.data += "\n";
        //            }

        //            // End boundry
        //            payload.data += "--";
        //            payload.data += boundry;
        //            payload.data += "--";
        //            return payload;

    } else {

        request->setRawHeader("Content-Type", "application/json");

        QJsonDocument data(this->options["data"].toObject());

        //        qDebug() << data.toJson(QJsonDocument::Compact);

        if (data.toJson(QJsonDocument::Compact) == "{}" || data.toJson(QJsonDocument::Compact) == "") {
            reply = manager->sendCustomRequest(*request, this->method, "");
        } else {
            reply = manager->sendCustomRequest(*request, this->method, data.toJson(QJsonDocument::Compact));
        }

        //        payload_t payload;
        //        payload.request = *request;
        //        payload.method = this->method;
        //        payload.data = data.toJson(QJsonDocument::Compact);
        //        return payload;
    }

    QEventLoop *loop = new QEventLoop(this);
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    QObject::connect(timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);

    timer->start(10000); // 10 second time out
    loop->exec();

    payload_t payload;
    payload.data = reply->readAll();
    // If network error
    if (reply->error() != QNetworkReply::NoError) {
        payload.error = true;
        payload.errorString = reply->errorString();
    } else {
        payload.error = false;
    }

    foreach(const QNetworkReply::RawHeaderPair pair, reply->rawHeaderPairs()) {
        payload.rawHeaderHash.insert(pair.first, pair.second);
    }

    payload.status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    manager->deleteLater();

    return payload;
}

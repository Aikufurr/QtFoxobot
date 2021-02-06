#include "websocket.h"

Websocket::Websocket(QString _token) {
    srand(time(0));
    token = _token;
}

void Websocket::start() {
    qDebug() << "WebSocket connecting";
    m_webSocket.open(QUrl("wss://gateway.discord.gg/?encoding=json"));
    connect(&m_webSocket, &QWebSocket::connected, this, &Websocket::onSocketConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Websocket::onSocketDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Websocket::onSocketTextMessageReceived);


    heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, QOverload<>::of(&Websocket::heartbeat));
}


void Websocket::heartbeat() {
    QJsonObject jObject;
    jObject.insert("op", opcodes::HEARTBEAT);
    jObject.insert("d", sequenceNumber);

    QJsonDocument jDocument(jObject);
    qDebug() << "[Send] Opcode 1 Heartbeat - S:" << sequenceNumber;
    m_webSocket.sendTextMessage(jDocument.toJson(QJsonDocument::Compact));
}


void Websocket::onSocketConnected() {
    qDebug() << "WebSocket connected";
}

void Websocket::resumeGateway() {
    if (!session_id.isEmpty()) {
        QJsonObject jObject;
        jObject.insert("op", opcodes::RESUME);

        QJsonObject jObjectD;
        jObjectD.insert("token", token);
        jObjectD.insert("session_id", session_id);
        jObjectD.insert("seq", sequenceNumber);
        jObject.insert("d", jObjectD);

        QJsonDocument jDocument(jObject);
        qDebug() << "[Send] Opcode 6 RESUME - " << jDocument.toJson(QJsonDocument::Compact);
        m_webSocket.sendTextMessage(jDocument.toJson(QJsonDocument::Compact));
    }
}
void Websocket::onSocketDisconnected() {
    qDebug() << "WebSocket disonnected";
    if (!session_id.isEmpty()) {
        m_webSocket.close();
        m_webSocket.open(QUrl("wss://gateway.discord.gg/?v=6&encoding=json"));
        resumeGateway();
    }
}


void Websocket::onSocketTextMessageReceived(QString message) {
    //    qDebug() << "Message received:" << message;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject payload = jsonResponse.object();
//    qDebug() << payload;

    switch (payload["op"].toInt()) {
    case opcodes::DISPATCH: {
        qDebug() << "[Recv] Opcode 0 DISPATCH -" << payload["t"].toString();
        sequenceNumber = payload["s"].toInt();

        if (payload["t"].toString() == "READY") {
            session_id = payload["d"].toObject()["session_id"].toString();
            QJsonObject response;
            response.insert("user", payload["d"].toObject()["user"]);
            emit READY(response);
        } else if (payload["t"].toString() == "GUILD_CREATE") {
            emit GUILD_CREATE(payload["d"].toObject());
        } else if (payload["t"].toString() == "MESSAGE_CREATE") {
            emit MESSAGE_CREATE(payload["d"].toObject());
        } else if (payload["t"].toString() == "INTERACTION_CREATE") {
            emit INTERACTION_CREATE(payload["d"].toObject());
        } else if (payload["t"].toString() == "GUILD_MEMBER_UPDATE") {
            emit GUILD_MEMBER_UPDATE(payload["d"].toObject());
        }
        break;
    }
    case opcodes::INVALID_SESSION: {
        qDebug() << "[Recv] Opcode 9 INVALID_SESSION - d:" << payload["d"].toBool();
        if (payload["d"].toBool() == true) {
            resumeGateway();
        } else {
            QTimer::singleShot((rand() % 5 + 1)*1000, this, &Websocket::resumeGateway);
        }
        break;
    }
    case opcodes::HELLO: {
        int heartbeat_interval = payload["d"].toObject()["heartbeat_interval"].toInt();
        if (heartbeat_interval == 0) {
            qDebug() << "[Erro] Recieved Heatbeat Interval of 0";
            return;
        }
        qDebug() << "[Recv] Opcode 10 Hello - Heartbeat Interval:" << heartbeat_interval;

        // Acknowledge Heartbeat
        {
            QJsonObject jObject;
            jObject.insert("op", opcodes::HEARTBEAT);
            jObject.insert("d", sequenceNumber);

            QJsonDocument jDocument(jObject);
            qDebug() << "[Send] Opcode 1 Heartbeat - S:" << sequenceNumber;
            m_webSocket.sendTextMessage(jDocument.toJson(QJsonDocument::Compact));
        }
        // Interval heartbeat
        heartbeatTimer->start(heartbeat_interval);

        // Identify bot
        {
            QJsonObject jObject;
            jObject.insert("op", opcodes::IDENTIFY);

            QJsonObject jObjectD;
            jObjectD.insert("token", token);
            jObjectD.insert("intents", intents);
            QJsonObject jObjectProperties;
            jObjectProperties.insert("$os", os);
            //            jObjectProperties.insert("$browser", os);
            //            jObjectProperties.insert("$device", os);

            jObjectD.insert("properties", jObjectProperties);

            QJsonObject jObjectPresence;
            jObjectPresence.insert("status", "online");
            jObjectPresence.insert("afk", false);
            jObjectPresence.insert("since", 0);

            QJsonArray jObjectPresenceActivities;
            QJsonObject jObjectPresenceActivity1;
            jObjectPresenceActivity1.insert("name", "with a deerfox");
            jObjectPresenceActivity1.insert("type", 0);
            jObjectPresenceActivities.push_back(jObjectPresenceActivity1);
            jObjectPresence.insert("activities", jObjectPresenceActivities);

            jObjectD.insert("presence", jObjectPresence);

            jObject.insert("d", jObjectD);

            QJsonDocument jDocument(jObject);
            qDebug() << "[Send] Opcode 2 Identity";
            m_webSocket.sendTextMessage(jDocument.toJson(QJsonDocument::Compact));
        }
        break;
    }
    case opcodes::HEARTBEAT_ACK: {
        qDebug() << "[Recv] Opcode 11 Heartbeat ACK";
        break;
    }
    }
}

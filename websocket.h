#ifndef WEBSOCKET_H
#define WEBSOCKET_H


#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <QtWebSockets/QtWebSockets>

namespace opcodes {
    enum Opcode {
        DISPATCH                =   0,
        HEARTBEAT               =   1,
        IDENTIFY                =   2,
        PRESENCE_UPDATE         =   3,
        VOICE_STATE_UPDATE      =   4,
        RESUME                  =   6,
        RECONNECT               =   7,
        REQUEST_GUILD_MEMBERS   =   8,
        INVALID_SESSION         =   9,
        HELLO                   =   10,
        HEARTBEAT_ACK           =   11
    };
}


class Websocket : public QObject {
    Q_OBJECT

public:
    explicit Websocket(QString token = "");
    QWebSocket m_webSocket;
    void start();

private:

#ifdef __linux__
    QString os = "linux";
#elif _WIN32
    QString os = "windows";
#endif

    int intents = 1539;
    int sequenceNumber = 0;
    QString token;
    QString session_id;
    QTimer *heartbeatTimer;
    void resumeGateway();

private slots:
    void heartbeat();
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketTextMessageReceived(QString message);

signals:
    void READY(QJsonObject);
    void GUILD_CREATE(QJsonObject);
    void MESSAGE_CREATE(QJsonObject);
    void INTERACTION_CREATE(QJsonObject);
    void GUILD_MEMBER_UPDATE(QJsonObject);
};

#endif // WEBSOCKET_H

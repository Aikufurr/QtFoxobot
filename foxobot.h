#ifndef DISCORDBOT_H
#define DISCORDBOT_H

#include "client.h"
#include "dbmanager.h"

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class foxobot : public QObject {
    Q_OBJECT
public:
    foxobot();

private:
    Client *client;
    DbManager *dbManager;
    QString token;
    QString application_id;
    QString dev_application_id;;
    QString dev_guild_id;

private slots:
    void ready(QString name);
    void message_create(Client::message_t message);
    void interaction_create(Client::interaction_t *interaction);
    void foxobot::create_slash_commands();
};

#endif // DISCORDBOT_H

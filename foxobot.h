#ifndef DISCORDBOT_H
#define DISCORDBOT_H

#include "client.h"
#include "dbmanager.h"
#include "util/rankreset.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QThread>

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
    void create_slash_commands();

private slots:
    void ready(QString name);
    void message(Client::message_t *message);
    void interaction_create(Client::interaction_t *interaction);
};

#endif // DISCORDBOT_H

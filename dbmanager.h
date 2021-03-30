#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>

class DbManager {
public:
    DbManager(const QString &path);
    QSqlDatabase m_db;

    QString config_get(QString key);

    void rank_createIfNotExist(QString userID, QString guildID);
    void rank_increment(QString userID, QString guildID);
    int rank_get(QString userID, QString guildID);
    QJsonObject rank_position(QString userID, QString guildID);
    QJsonArray rank_leaderboard(QString guildID, int results);
    void rank_reset();

    QString cmd_id(QString command);
    QJsonObject cmd_get(QString command);
    QList<QString> cmd_get();
    void cmd_set(QString command_id, QString command, QString value);
    bool cmd_exists(QString command);

    QHash<QString, QHash<QString, QString>> setting_get(QString guild_id);
    QString setting_get(QString guild_id, QString group, QString setting);
    bool setting_set(QString guild_id, QString group, QString setting, QString value);
};

#endif // DBMANAGER_H

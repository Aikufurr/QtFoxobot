#ifndef DBMANAGER_H
#define DBMANAGER_H


#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>

class DbManager
{
public:
    DbManager(const QString &path);
    QSqlDatabase m_db;

    QString config_get(QString key);
    void rank_createIfNotExist(QString userID, QString guildID);
    void rank_increment(QString userID, QString guildID);
    int rank_get(QString userID, QString guildID);
    QJsonObject rank_position(QString userID, QString guildID);
    QJsonArray rank_leaderboard(QString guildID, int results);
};

#endif // DBMANAGER_H

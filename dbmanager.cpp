#include "dbmanager.h"
DbManager::DbManager(const QString &path) {
    qDebug() << QSqlDatabase::drivers();
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "Attempting path" << path;
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qDebug() << "Database: Connection with database failed" << m_db.lastError();
    } else {
        qDebug() << "Database: Connection OK";
    }

    {
        QSqlQuery tableQuery = QSqlQuery(m_db);
        tableQuery.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='ranks'");
        tableQuery.exec();

        if (!tableQuery.next()) {
            QSqlQuery query = QSqlQuery(m_db);
            query.prepare(R"**(CREATE TABLE IF NOT EXISTS "ranks" (
                          "ID"	INTEGER UNIQUE,
                          "USER_ID"	TEXT NOT NULL,
                          "GUILD_ID"	TEXT NOT NULL,
                          "RANK"	INTEGER NOT NULL,
                          PRIMARY KEY("ID" AUTOINCREMENT)
                          ))**");

            if (!query.exec()) {
                qDebug() << "createTable ranks error:"
                         << query.lastError();
            }
        }
    } {
        QSqlQuery tableQuery = QSqlQuery(m_db);
        tableQuery.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='config'");
        tableQuery.exec();

        if (!tableQuery.next()) {
            QSqlQuery query = QSqlQuery(m_db);
            query.prepare(R"**(CREATE TABLE IF NOT EXISTS "config" (
                          "ID"	INTEGER UNIQUE,
                          "KEY"	TEXT NOT NULL,
                          "VALUE"	TEXT NOT NULL,
                          PRIMARY KEY("ID" AUTOINCREMENT)
                          ))**");

            if (!query.exec()) {
                qDebug() << "createTable config error:"
                         << query.lastError();
            }
        }
    } {
        QSqlQuery tableQuery = QSqlQuery(m_db);
        tableQuery.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='commands'");
        tableQuery.exec();

        if (!tableQuery.next()) {
            QSqlQuery query = QSqlQuery(m_db);
            query.prepare(R"**(CREATE TABLE IF NOT EXISTS "commands" (
                          "ID"	INTEGER UNIQUE,
                          "COMMAND_ID"	TEXT NOT NULL UNIQUE,
                          "COMMAND"	TEXT NOT NULL UNIQUE,
                          "VALUE"	TEXT NOT NULL UNIQUE,
                          PRIMARY KEY("ID" AUTOINCREMENT)
                          ))**");

            if (!query.exec()) {
                qDebug() << "createTable commands error:"
                         << query.lastError();
            }
        }
    } {
        QSqlQuery tableQuery = QSqlQuery(m_db);
        tableQuery.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='settings'");
        tableQuery.exec();

        if (!tableQuery.next()) {
            QSqlQuery query = QSqlQuery(m_db);
            query.prepare(R"**(CREATE TABLE IF NOT EXISTS "settings" (
                          "ID"	INTEGER UNIQUE,
                          "GUILD_ID"	TEXT NOT NULL,
                          "SETTING_GROUP"	TEXT NOT NULL,
                          "SETTING"	TEXT NOT NULL,
                          "VALUE"	TEXT NOT NULL,
                          PRIMARY KEY("ID" AUTOINCREMENT)
                          ))**");

            if (!query.exec()) {
                qDebug() << "createTable commands error:"
                         << query.lastError();
            }
        }
    }
}

QHash<QString, QHash<QString, QString>> DbManager::setting_get(QString guild_id) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT GROUP, SETTING, VALUE FROM settings WHERE GUILD_ID = :GUILD_ID");
    query.bindValue(":GUILD_ID", guild_id);
    query.exec();
    QHash<QString, QHash<QString, QString>> settings;
    while (query.next()) {
        QHash<QString, QString> setting;
        setting[query.value("SETTING").toString()] = query.value("VALUE").toString();
        settings[query.value("SETTING_GROUP").toString()] = setting;
    }
    return settings;
}

QString DbManager::setting_get(QString guild_id, QString group, QString setting) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT VALUE FROM settings WHERE GUILD_ID = :GUILD_ID AND SETTING_GROUP = :GROUP AND SETTING = :SETTING");
    query.bindValue(":GUILD_ID", guild_id);
    query.bindValue(":GROUP", group);
    query.bindValue(":SETTING", setting);
    query.exec();
    query.next();
    return query.value(0).toString();
}

bool DbManager::setting_set(QString guild_id, QString group, QString setting, QString value) {
    qDebug() << "{DB} Selecting" << guild_id << group << setting;
    QSqlQuery selectQuery = QSqlQuery(m_db);
    selectQuery.prepare("SELECT ID FROM settings WHERE GUILD_ID = :GUILD_ID AND SETTING_GROUP = :GROUP AND SETTING = :SETTING");
    selectQuery.bindValue(":GUILD_ID", guild_id);
    selectQuery.bindValue(":GROUP", group);
    selectQuery.bindValue(":SETTING", setting);
    selectQuery.exec();
    if (selectQuery.next()) {
        QSqlQuery query = QSqlQuery(m_db);
        query.prepare("UPDATE settings SET VALUE = :VALUE WHERE GUILD_ID = :GUILD_ID AND SETTING_GROUP = :GROUP AND SETTING = :SETTING");
        query.bindValue(":GUILD_ID", guild_id);
        query.bindValue(":GROUP", group);
        query.bindValue(":SETTING", setting);
        query.bindValue(":VALUE", value);
        if(!query.exec()) {
            return false;
        }
        qDebug() << "{DB}" << guild_id << group << setting << "updated to" << value;
    } else {
        qDebug() << "{DB}" << guild_id << group << setting << "does not exist";
        QSqlQuery query = QSqlQuery(m_db);
        query.prepare("INSERT INTO settings (GUILD_ID, SETTING_GROUP, SETTING, VALUE) VALUES (:GUILD_ID, :GROUP, :SETTING, :VALUE)");
        query.bindValue(":GUILD_ID", guild_id);
        query.bindValue(":GROUP", group);
        query.bindValue(":SETTING", setting);
        query.bindValue(":VALUE", value);
        if(!query.exec()) {
            return false;
        }
        qDebug() << "{DB}" << guild_id << group << setting << "created" << value;
    }
    return true;
}


bool DbManager::cmd_exists(QString command) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT ID FROM commands WHERE COMMAND = :COMMAND");
    query.bindValue(":COMMAND", command);
    query.exec();
    if (query.next()) {
        return true;
    } else {
        return false;
    }
}
QString DbManager::cmd_id(QString command) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT COMMAND_ID FROM commands WHERE COMMAND = :COMMAND");
    query.bindValue(":COMMAND", command);
    query.exec();
    if (query.next()) {
        return query.value(0).toString();
    } else {
        return "";
    }
}
QJsonObject DbManager::cmd_get(QString command) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT VALUE FROM commands WHERE COMMAND = :COMMAND");
    query.bindValue(":COMMAND", command);
    query.exec();
    if (query.next()) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(query.value(0).toString().toUtf8());
        QJsonObject payload = jsonResponse.object();
        return payload;
    } else {
        QJsonObject payload;
        return payload;
    }
}
QList<QString> DbManager::cmd_get() {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT COMMAND FROM commands");
    query.exec();
    QList<QString> commands;
    while (query.next()) {
        commands.push_back(query.value(0).toString());
    }
    return commands;
}
void DbManager::cmd_set(QString command_id, QString command, QString value) {
    qDebug() << "{DB} Selecting" << command << command_id;
    QSqlQuery selectQuery = QSqlQuery(m_db);
    selectQuery.prepare("SELECT ID FROM commands WHERE COMMAND = :COMMAND");
    selectQuery.bindValue(":COMMAND", command);
    selectQuery.exec();
    if (selectQuery.next()) {
        QSqlQuery query = QSqlQuery(m_db);
        query.prepare("UPDATE commands SET COMMAND_ID = :COMMAND_ID, VALUE = :VALUE WHERE COMMAND = :COMMAND");
        query.bindValue(":COMMAND_ID", command_id);
        query.bindValue(":COMMAND", command);
        query.bindValue(":VALUE", value);
        query.exec();
        qDebug() << "{DB}" << command << "updated";
    } else {
        qDebug() << "{DB}" << command << "does not exist";
        QSqlQuery query = QSqlQuery(m_db);
        query.prepare("INSERT INTO commands (COMMAND_ID, COMMAND, VALUE) VALUES (:COMMAND_ID, :COMMAND, :VALUE)");
        query.bindValue(":COMMAND_ID", command_id);
        query.bindValue(":COMMAND", command);
        query.bindValue(":VALUE", value);
        query.exec();
        qDebug() << "{DB}" << command << "created";
    }
}

QString DbManager::config_get(QString key) {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT VALUE FROM config WHERE KEY = :KEY");
    query.bindValue(":KEY", key);
    query.exec();
    query.first();
    return query.value(0).toString();
}
void DbManager::rank_createIfNotExist(QString userID, QString guildID) {
    QSqlQuery selectQuery = QSqlQuery(m_db);
    selectQuery.prepare("SELECT ID FROM ranks WHERE USER_ID = :USER_ID AND GUILD_ID = :GUILD_ID");
    selectQuery.bindValue(":USER_ID", userID);
    selectQuery.bindValue(":GUILD_ID", guildID);
    selectQuery.exec();
    if (!selectQuery.next()) {
        QSqlQuery query = QSqlQuery(m_db);
        query.prepare("INSERT INTO ranks (USER_ID, GUILD_ID, RANK) VALUES (:USER_ID, :GUILD_ID, :RANK)");
        query.bindValue(":USER_ID", userID);
        query.bindValue(":GUILD_ID", guildID);
        query.bindValue(":RANK", 0);
        query.exec();
    }
}

void DbManager::rank_increment(QString userID, QString guildID) {
    this->rank_createIfNotExist(userID, guildID);
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("UPDATE ranks SET RANK = RANK + 1 WHERE USER_ID = :USER_ID AND GUILD_ID = :GUILD_ID");
    query.bindValue(":USER_ID", userID);
    query.bindValue(":GUILD_ID", guildID);
    query.exec();
}

int DbManager::rank_get(QString userID, QString guildID) {
    this->rank_createIfNotExist(userID, guildID);
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("SELECT RANK FROM ranks WHERE USER_ID = :USER_ID AND GUILD_ID = :GUILD_ID");
    query.bindValue(":USER_ID", userID);
    query.bindValue(":GUILD_ID", guildID);
    query.exec();
    query.nextResult();
    return query.value(0).toInt();
}

QJsonObject DbManager::rank_position(QString userID, QString guildID) {
    this->rank_createIfNotExist(userID, guildID);
    QSqlQuery query = QSqlQuery(m_db);
    QJsonObject jObject;
    int count = 1;
    {
        query.prepare("SELECT COUNT(*) as cnt FROM ranks WHERE GUILD_ID = :GUILD_ID");
        query.bindValue(":GUILD_ID", guildID);
        query.exec();
        query.first();
        count = query.value(0).toInt();
    }
    {
        query.prepare("SELECT RowNum, RANK FROM (SELECT ROW_NUMBER () OVER (ORDER BY RANK DESC) RowNum, USER_ID, GUILD_ID, RANK FROM ranks WHERE GUILD_ID = :GUILD_ID ORDER BY RANK DESC) WHERE USER_ID = :USER_ID");
        query.bindValue(":USER_ID", userID);
        query.bindValue(":GUILD_ID", guildID);
        query.exec();
        query.first();
        jObject.insert("position", query.value("RowNum").toInt());
        jObject.insert("count", count);
        jObject.insert("rank", query.value("RANK").toInt());
    }
    {
        query.prepare("SELECT RowNum, USER_ID, RANK FROM (SELECT ROW_NUMBER () OVER (ORDER BY RANK DESC) RowNum, USER_ID, RANK FROM ranks WHERE GUILD_ID = :GUILD_ID ORDER BY RANK DESC) WHERE RowNum = :RowNum");
        query.bindValue(":RowNum", jObject["position"].toInt()-1);
        query.bindValue(":GUILD_ID", guildID);
        query.exec();
        query.first();

        jObject.insert("behind_position", query.value("RowNum").toInt());
        jObject.insert("behind_user_id", query.value("USER_ID").toString());
        jObject.insert("behind_rank", query.value("RANK").toInt());
    }

    return jObject;
}


QJsonArray DbManager::rank_leaderboard(QString guildID, int results) {
    QSqlQuery query = QSqlQuery(m_db);
    QJsonArray leaderboard;

    query.prepare("SELECT RowNum, USER_ID, RANK FROM (SELECT ROW_NUMBER () OVER (ORDER BY RANK DESC) RowNum, USER_ID, GUILD_ID, RANK FROM ranks WHERE GUILD_ID = :GUILD_ID ORDER BY RANK DESC LIMIT :LIMIT)");
    query.bindValue(":GUILD_ID", guildID);
    query.bindValue(":LIMIT", results);
    query.exec();
    while (query.next()) {
        QJsonObject result;
        result.insert("position", query.value("RowNum").toInt());
        result.insert("user_id", query.value("USER_ID").toString());
        result.insert("rank", query.value("RANK").toInt());
        leaderboard.push_back(result);
    }


    return leaderboard;
}


void DbManager::rank_reset() {
    QSqlQuery query = QSqlQuery(m_db);
    query.prepare("UPDATE `ranks` SET `RANK` = 0");
    query.exec();
}

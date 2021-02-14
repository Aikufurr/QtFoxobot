#include "client.h"
#include "websocket.h"

Client::Client(QString _application_id, DbManager *_dbmanager) {
    application_id = _application_id;
    dbmanager = _dbmanager;
}

int Client::DaysInMonth(QDate date) {
    QDateTime monthStart(QDate(date.year(), date.month(), 1));
    QDateTime monthEnd(QDate(date.year(), date.month() + 1, 1));
    int monthLength = (monthEnd.toMSecsSinceEpoch() - monthStart.toMSecsSinceEpoch()) / (1000 * 60 * 60 * 24);
    return monthLength;
}

QString Client::getAge(QDate then, QDate now) {
    //--------------------------------------------------------------
    int days = now.day() - then.day();
    if (days < 0) {
        now.setDate(now.year(), now.month()-1, now.day());
        days += this->DaysInMonth(now);
    }
    //--------------------------------------------------------------
    int months = now.month() - then.month();
    if (months < 0) {
        now.setDate(now.year()-1, now.month(), now.day());
        months += 12;
    }
    //--------------------------------------------------------------
    int years = now.year() - then.year();

    QString output;

    if (years != 0) {
        output += QString("%1 year%2, ").arg(QString::number(years), years == 1 ? "0" : "s");
    }
    if (months != 0) {
        output += QString("%1 month%2, ").arg(QString::number(months), months == 1 ? "0" : "s");
    }
    if (days != 0) {
        output += QString("%1 day%2").arg(QString::number(days), days == 1 ? "0" : "s");
    }

    return output;
}

void Client::login(QString _token) {
    token = _token;

    websocket = new Websocket(token);

    connect(websocket, SIGNAL(READY(QJsonObject)), this, SLOT(READY(QJsonObject)));

    connect(websocket, SIGNAL(INTERACTION_CREATE(QJsonObject)), this, SLOT(INTERACTION_CREATE(QJsonObject)));

    connect(websocket, SIGNAL(GUILD_CREATE(QJsonObject)), this, SLOT(GUILD_CREATE(QJsonObject)));
    connect(websocket, SIGNAL(GUILD_UPDATE(QJsonObject)), this, SLOT(GUILD_UPDATE(QJsonObject)));

    connect(websocket, SIGNAL(GUILD_MEMBER_ADD(QJsonObject)), this, SLOT(GUILD_MEMBER_ADD(QJsonObject)));
    connect(websocket, SIGNAL(GUILD_MEMBER_UPDATE(QJsonObject)), this, SLOT(GUILD_MEMBER_UPDATE(QJsonObject)));
    connect(websocket, SIGNAL(GUILD_MEMBER_REMOVE(QJsonObject)), this, SLOT(GUILD_MEMBER_REMOVE(QJsonObject)));

    connect(websocket, SIGNAL(MESSAGE_CREATE(QJsonObject)), this, SLOT(MESSAGE_CREATE(QJsonObject)));
    connect(websocket, SIGNAL(MESSAGE_UPDATE(QJsonObject)), this, SLOT(MESSAGE_UPDATE(QJsonObject)));
    connect(websocket, SIGNAL(MESSAGE_DELETE(QJsonObject)), this, SLOT(MESSAGE_DELETE(QJsonObject)));

    websocket->start();
}

// COMMANDS
void Client::change_presence(QString name, int type, QString status) {
    QJsonObject presence;
    presence.insert("status", status);

    QJsonArray presenceActivity1;
    presence.insert("name", name);
    presence.insert("type", type);
    QJsonArray presenceActivities;
    presenceActivities.push_back(presenceActivity1);

    presence.insert("activities", presenceActivities);

    QJsonObject jObject;
    jObject.insert("op", opcodes::PRESENCE_UPDATE);
    jObject.insert("d", presence);
    QJsonDocument jDocument(jObject);
    qDebug() << "[Send] Opcode 3 PRESENCE_UPDATE";
    websocket->m_webSocket.sendTextMessage(jDocument.toJson(QJsonDocument::Compact));
}


void Client::create_slash_command(QJsonObject command, QString guild_id) {
    QJsonDocument doc(command);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    if (dbmanager->cmd_exists(command["name"].toString())) {

        QJsonDocument jDocument(dbmanager->cmd_get(command["name"].toString()));
        QJsonDocument jCommand(command);

        if (jDocument.toJson(QJsonDocument::Compact) == jCommand.toJson(QJsonDocument::Compact)) {
            qDebug() << "Skipping" << command["name"].toString() << "- Already exists";
            return;
        }
    }

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

    QString urlString = QString("https://discord.com/api/v8/applications/%1").arg(application_id);

    if (command["name"].toString() == "close" && !guild_id.isEmpty()) {
        qDebug() << "Creating slash command" << command["name"].toString() << "locally";
        urlString += QString("/guilds/%1/commands").arg(guild_id);
    } else {
        if (guild_id.isEmpty()) {
            qDebug() << "Creating slash command" << command["name"].toString() << "globally";
            urlString += QString("/commands");
        } else {
            qDebug() << "Creating slash command" << command["name"].toString() << "locally";
            urlString += QString("/guilds/%1/commands").arg(guild_id);
        }
    }

    const QUrl url(urlString);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

    qDebug() << data;
    QNetworkReply *reply;
    qDebug() << "POSTING" << command["name"].toString();
    reply = mgr->post(request, data);

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QString contents = QString::fromUtf8(reply->readAll());
            QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
            QJsonObject payload = jsonResponse.object();
            //            qDebug() << payload;

            QJsonDocument jDocument(command);
            dbmanager->cmd_set(payload["id"].toString(), payload["name"].toString(), jDocument.toJson(QJsonDocument::Compact));
        }
        else{
            QString err = reply->errorString();
            qDebug() << err;
        }
        reply->deleteLater();
        mgr->deleteLater();
    });

}

void Client::delete_slash_command(QString command_id, QString guild_id) {
    {
        QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

        QString urlString = QString("https://discord.com/api/v8/applications/%1/commands").arg(application_id);
        const QUrl url(urlString);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

        QNetworkReply *reply = mgr->get(request);

        QObject::connect(reply, &QNetworkReply::finished, [=](){
            if(reply->error() == QNetworkReply::NoError){
                QString contents = QString::fromUtf8(reply->readAll());
                QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
                QJsonArray payload = jsonResponse.array();
                qDebug() << payload;
            }
            else{
                QString err = reply->errorString();
                qDebug() << err;
            }
            reply->deleteLater();
            mgr->deleteLater();
        });
    }
    {
        QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

        QString urlString = QString("https://discord.com/api/v8/applications/%1").arg(application_id);

        if (guild_id.isEmpty()) {
            urlString += QString("/commands/%1").arg(command_id);
        } else {
            urlString += QString("/guilds/%1/commands/%1").arg(guild_id, command_id);
        }

        const QUrl url(urlString);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

        QNetworkReply *reply = mgr->sendCustomRequest(request, "DELETE");

        QObject::connect(reply, &QNetworkReply::finished, [=](){
            if(reply->error() == QNetworkReply::NoError){
                QString contents = QString::fromUtf8(reply->readAll());
                qDebug() << contents;
            }
            else{
                QString err = reply->errorString();
                qDebug() << err;
            }
            reply->deleteLater();
            mgr->deleteLater();
        });
    }
}

void Client::getGateway() {
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

    QString urlString = QString("https://discord.com/api/gateway/bot");


    const QUrl url(urlString);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());


    QNetworkReply *reply = mgr->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QString contents = QString::fromUtf8(reply->readAll());
            qDebug() << contents;
        }
        else{
            QString err = reply->errorString();
            qDebug() << err;
        }
        reply->deleteLater();
        delete mgr;
    });
}

void Client::send_message(QString channel_id, QString content) {
    embed_t embed;
    this->send_message(channel_id, content, embed);
}

void Client::send_message(QString channel_id, QString content, embed_t embed) {
    qDebug() << "Sending message to" << channel_id;
    //    QNetworkAccessManager * manager = new QNetworkAccessManager(this);

    //    QUrl url(QString("https://discord.com/api/channels/%1/messages").arg(channel_id));
    //    QNetworkRequest request(url);

    //    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    //    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    //    request.setRawHeader("Authorization", "Bot ODA1Nzg4MzE4MTQyNjkzMzg2.YBf-2Q.7_kYeO5QY5W7Vu9V8GUsekPexH4");

    //    QUrlQuery params;
    //    params.addQueryItem("content", content);

    //    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));

    //    manager->post(request, params.query().toUtf8());

    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    const QUrl url(QString("https://discord.com/api/channels/%1/messages").arg(channel_id));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

    QJsonObject obj;
    if (!content.isEmpty()) {
        obj.insert("content", content);
    }
    if (!embed.title.isEmpty()) {
        QJsonObject json_embed;
        json_embed.insert("title", embed.title);
        if (!embed.description.isEmpty()) {
            json_embed.insert("description", embed.description);
        }
        if (embed.colour != 0) {
            json_embed.insert("color", embed.colour == -1 ? 16757760 : embed.colour); // 14643990 - Darker
        }
        if (!embed.url.isEmpty()) {
            json_embed.insert("url", embed.url);
        }
        if (!embed.timestamp.isNull()) {
            // convert QDateTime to ISO8601 timestamp
            //            json_embed.insert("timestamp", embed.timestamp.t);
        }
        if (!embed.thumbnail_url.isEmpty()) {
            QJsonObject json_thumbnail_url;
            json_thumbnail_url.insert("url", embed.thumbnail_url);
            json_embed.insert("thumbnail", json_thumbnail_url);
        }
        if (!embed.author.name.isEmpty()) {
            QJsonObject json_embed_author;
            json_embed_author.insert("name", embed.author.name);
            json_embed_author.insert("icon_url", embed.author.icon_url);
            json_embed.insert("author", json_embed_author);
        }
        if (embed.fields.size() > 0) {
            QJsonArray fields;
            foreach(const embed_field_t &field, embed.fields) {
                QJsonObject json_embed_field;
                json_embed_field.insert("name", field.name);
                json_embed_field.insert("value", field.value);
                json_embed_field.insert("inline", field.is_inline);
                fields.push_back(json_embed_field);
            }
            json_embed.insert("fields", fields);
        }
        obj.insert("embed", json_embed);
        //        qDebug() << obj;
    }
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = mgr->post(request, data);

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            //            QString contents = QString::fromUtf8(reply->readAll());
            //            qDebug() << contents;
            qDebug() << "Message Sent";
        }
        else{
            QString err = reply->errorString();
            qDebug() << err << data;
        }
        reply->deleteLater();
        delete mgr;
    });
}

void Client::send_file_message(QString channel_id, QByteArray file) {
    embed_t embed;
    this->send_message(channel_id, file, embed);
}

void Client::send_file_message(QString channel_id, QByteArray _file, embed_t embed) {
    QUrl url(QString("https://discord.com/api/channels/%1/messages").arg(channel_id));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QJsonObject obj;
    if (!embed.title.isEmpty()) {
        QJsonObject json_embed;
        json_embed.insert("title", embed.title);
        if (!embed.description.isEmpty()) {
            json_embed.insert("description", embed.description);
        }
        if (embed.colour != 0) {
            json_embed.insert("color", embed.colour == -1 ? 16757760 : embed.colour); // 14643990 - Darker
        }
        if (!embed.url.isEmpty()) {
            json_embed.insert("url", embed.url);
        }
        if (!embed.timestamp.isNull()) {
            // convert QDateTime to ISO8601 timestamp
            //            json_embed.insert("timestamp", embed.timestamp.t);
        }
        if (!embed.thumbnail_url.isEmpty()) {
            QJsonObject json_thumbnail_url;
            json_thumbnail_url.insert("url", embed.thumbnail_url);
            json_embed.insert("thumbnail", json_thumbnail_url);
        }
        if (!embed.author.name.isEmpty()) {
            QJsonObject json_embed_author;
            json_embed_author.insert("name", embed.author.name);
            json_embed_author.insert("icon_url", embed.author.icon_url);
            json_embed.insert("author", json_embed_author);
        }
        if (embed.fields.size() > 0) {
            QJsonArray fields;
            foreach(const embed_field_t &field, embed.fields) {
                QJsonObject json_embed_field;
                json_embed_field.insert("name", field.name);
                json_embed_field.insert("value", field.value);
                json_embed_field.insert("inline", field.is_inline);
                fields.push_back(json_embed_field);
            }
            json_embed.insert("fields", fields);
        }
        obj.insert("embed", json_embed);
    }
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    QHttpPart loginPart;
    /* username */
    loginPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"payload_json\""));
    loginPart.setBody(data);
    multiPart->append(loginPart);

    QHttpPart filePart;
    /* important that the files[] variable have the brackets, for PHP to interpret correctly */
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"rank_chart.png\""));
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, _file.size());

    //        file->open(QIODevice::ReadOnly);
    //        filePart.setBodyDevice(file);
    //        file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    filePart.setBody(_file);
    multiPart->append(filePart);


    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    QNetworkReply* reply = networkManager->post(request, multiPart);

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            //            QString contents = QString::fromUtf8(reply->readAll());
            //            qDebug() << contents;
            qDebug() << "Message Sent";
        } else{
            QString err = reply->errorString();
            qDebug() << err;
        }
        reply->deleteLater();
        delete networkManager;
    });
}

//void Client::replyFinished(QNetworkReply *rep)
//{
//    QByteArray bts = rep->readAll();
//    QString str(bts);
//    qDebug() << str;

//}

// SLOTS
void Client::READY(QJsonObject response) {
    QJsonObject userResponse = response["user"].toObject();
    user.avatar = userResponse["avatar"].toString();
    user.bot = userResponse["bot"].toBool();
    user.discriminator = userResponse["discriminator"].toString();
    user.flags = userResponse["flags"].toInt();
    user.id = userResponse["id"].toString();
    user.mfa_enabled= userResponse["mfa_enabled"].toBool();
    user.username = userResponse["username"].toString();
    user.verified = userResponse["verified"].toBool();
    me = user;
    users[user.id] = user;
    qDebug() << "{Client::ready} Loaded" << user.username;
    emit ready(user.username);
}

Client::user_t Client::getMe() {
    return me;
}

void Client::create_guild(QJsonObject json_guild) {
    Client::guild_t guild;
    guild.id = json_guild["id"].toString();
    guild.name = json_guild["name"].toString();
    guild.icon = json_guild["icon"].toString().isNull() ? "" : json_guild["icon"].toString();
    guild.splash = json_guild["splash"].toString().isNull() ? "" : json_guild["splash"].toString();
    guild.owner_id = json_guild["owner_id"].toString();
    guild.region = json_guild["region"].toString();
    guild.verification_level = json_guild["verification_level"].toInt();
    guild.explicit_content_filter = json_guild["explicit_content_filter"].toInt();
    guild.mfa_level = json_guild["mfa_level"].toInt();
    guild.large = json_guild["large"].toBool();
    guild.member_count = json_guild["member_count"].toInt(); // get members
    guild.max_members = json_guild["max_members"].toInt();
    guild.description = json_guild["description"].toString().isNull() ? "" : json_guild["description"].toString();



    QList<Client::roles_t> roles;
    foreach(const QJsonValue &value, json_guild["roles"].toArray()) {
        QJsonObject json_role = value.toObject();
        Client::roles_t role;
        role.id = json_role["id"].toString();
        role.name = json_role["name"].toString();
        role.colour = json_role["color"].toInt();
        role.position = json_role["position"].toInt();
        role.permissions = json_role["permissions"].toString();
        role.managed = json_role["managed"].toBool();
        role.mentionable = json_role["mentionable"].toBool();
        roles.push_back(role);
    }
    guild.roles = roles;


    QJsonArray member_array = json_guild["members"].toArray();              // Members in the json
    QHash<QString, member_t> current_members = guilds[guild.id].members;    // Members in the guild

    // If no members exist currently (on update)
    if (current_members.size() <= 1) {
        // If no members exist in the json
        if (member_array.size() <= 1) {
            if (guild.member_count < 1000) {
                qDebug() << "Pulling x<1000 members from" << guild.id;
                QNetworkAccessManager NAManager;
                QNetworkRequest request(QUrl(QString("https://discord.com/api/guilds/%1/members?limit=999").arg(guild.id)));
                request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
                request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
                QNetworkReply *reply = NAManager.get(request);
                QEventLoop eventLoop;
                QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit())); // 48- bug
                eventLoop.exec();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
                QJsonArray payload = jsonResponse.array();
                NAManager.deleteLater();

                foreach(const QJsonValue &value, payload) {
                    member_array.push_back(value.toObject());
                }
            } else {
                // Figure out how to work with the "highest snowflake"
            }
        }
    }

    foreach(const QJsonValue &value, member_array) {
        QJsonObject json_member = value.toObject();
        QList<Client::roles_t> roles;
        Client::member_t member;
        Client::user_t user;

        user.avatar = json_member["user"].toObject()["avatar"].toString();
        user.bot = json_member["user"].toObject()["bot"].toBool();
        user.discriminator = json_member["user"].toObject()["discriminator"].toString();
        user.flags = json_member["user"].toObject()["flags"].toInt();
        user.id = json_member["user"].toObject()["id"].toString();
        user.mfa_enabled = json_member["user"].toObject()["mfa_enabled"].toBool();
        user.username = json_member["user"].toObject()["username"].toString();
        user.verified = json_member["user"].toObject()["verified"].toBool();
        users[user.id] = user;
        member.user = user;

        member.nick = json_member["nick"].toString().isNull() ? "" : json_member["nick"].toString();

        QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
        foreach(const QJsonValue &roleValue, json_member["roles"].toArray()) {
            Client::roles_t role;

            for ( ; it != guild.roles.constEnd(); ++it ) {
                const Client::roles_t &roleIT= *it;
                if (roleIT.id == roleValue.toString()) {
                    role = roleIT;
                    roles.push_back(role);
                    break;
                }
            }
        }
        member.roles = roles;
        member.joined_at = QDateTime::fromString(json_member["joined_at"].toString(), Qt::ISODate);
        member.premium_since = QDateTime::fromString(json_member["premium_since"].toString(), Qt::ISODate);
        member.permissions = json_member["permissions"].toString();

        guild.members[user.id] = member;
        guild.member_count = guild.members.size();
    }


    QJsonArray json_channels = json_guild["channels"].toArray();

    if (json_channels.size() == 0) {
        qDebug() << "Pulling channels from" << guild.id;
        QNetworkAccessManager NAManager;
        QNetworkRequest request(QUrl(QString("https://discord.com/api/guilds/%1/channels").arg(guild.id)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
        QNetworkReply *reply = NAManager.get(request);
        QEventLoop eventLoop;
        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonArray payload = jsonResponse.array();
        NAManager.deleteLater();

        foreach(const QJsonValue &value, payload) {
            json_channels.push_back(value.toObject());
        }
    }

    QList<Client::channel_t> channels;
    foreach(const QJsonValue &value, json_channels) {
        QJsonObject json_channel = value.toObject();
        Client::channel_t channel;

        channel.id = json_channel["id"].toString();
        channel.type = json_channel["type"].toInt();
        channel.guild_id = guild.id;
        channel.position = json_channel["position"].toInt();

        QList<permission_overwrite_t> permission_overwrites;
        foreach(const QJsonValue &value, json_guild["roles"].toArray()) {
            QJsonObject json_permission_overwrite = value.toObject();
            Client::permission_overwrite_t permission_overwrite;
            permission_overwrite.id = json_permission_overwrite["id"].toString();
            permission_overwrite.type = json_permission_overwrite["type"].toString();
            permission_overwrite.allow = json_permission_overwrite["allow_new"].toString();
            permission_overwrite.deny = json_permission_overwrite["deny_new"].toString();
            permission_overwrites.push_back(permission_overwrite);
        }
        channel.permission_overwrites = permission_overwrites;


        channel.name = json_channel["name"].toString();
        channel.topic = json_channel["topic"].toString().isNull() ? "" : json_guild["topic"].toString();
        channel.nsfw = json_channel["nsfw"].toBool();
        channel.last_message_id = json_channel["last_message_id"].toString();
        channel.bitrate = json_channel["bitrate"].toInt();
        channel.user_limit = json_channel["user_limit"].toInt();
        channel.parent_id = json_channel["parent_id"].toString();


        channels.push_back(channel);
    }

    guild.channels = channels;

    guilds[guild.id] = guild;
}

void Client::GUILD_CREATE(QJsonObject json_guild) {
    this->create_guild(json_guild);
}
void Client::GUILD_UPDATE(QJsonObject json_guild) {
    this->create_guild(json_guild);
}

Client::guild_t Client::getGuild(QString guild_id) {
    if (guilds.contains(guild_id)) {
        return guilds[guild_id];
    } else {
        qDebug() << "Pulling guild" << guild_id;
        QNetworkAccessManager NAManager;
        QNetworkRequest request(QUrl(QString("https://discord.com/api/guilds/%1?with_counts=true").arg(guild_id)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
        QNetworkReply *reply = NAManager.get(request);
        QEventLoop eventLoop;
        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject json_guild = jsonResponse.object();

        json_guild["member_count"] = json_guild["approximate_member_count"].toInt();

        this->create_guild(json_guild);

        return guilds[guild_id];
    }
}

Client::user_t Client::getUser(QString user_id) {
    if (users.contains(user_id)) {
        return users[user_id];
    } else {
        qDebug() << "Pulling user" << user_id;
        QNetworkAccessManager NAManager;
        QNetworkRequest request(QUrl(QString("https://discord.com/api/users/%1").arg(user_id)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
        QNetworkReply *reply = NAManager.get(request);
        QEventLoop eventLoop;
        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject payload = jsonResponse.object();
        NAManager.deleteLater();
        Client::user_t user;
        user.avatar = payload["avatar"].toString();
        user.discriminator = payload["discriminator"].toString();
        user.id = payload["id"].toString();
        user.flags = payload["flags"].toInt();
        user.username = payload["username"].toString();
        user.bot = payload["bot"].toBool();
        users[user.id] = user;
        return user;
    }
}

Client::member_t Client::getMember(QString user_id, QString guild_id) {
    Client::guild_t guild = this->getGuild(guild_id);
    if (guild.members.contains(user_id)) {
        return guild.members[user_id];
    } else {
        qDebug() << "Pulling member" << user_id << "from" << guild_id;
        Client::user_t user = this->getUser(user_id);

        QNetworkAccessManager NAManager;
        QNetworkRequest request(QUrl(QString("https://discord.com/api/guilds/%1/members/%2").arg(guild_id, user_id)));
        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
        QNetworkReply *reply = NAManager.get(request);
        QEventLoop eventLoop;
        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
        QJsonObject payload = jsonResponse.object();
        NAManager.deleteLater();

        Client::member_t member;
        member.user = user;
        member.nick = payload["nick"].toString();

        QList<Client::roles_t> roles;
        foreach(const QJsonValue &value, payload["roles"].toArray()) {
            QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
            for ( ; it != guild.roles.constEnd(); ++it ) {
                const Client::roles_t &role = *it;

                if (value.toString() == role.id) {
                    roles.push_back(role);
                }
            }
        }
        member.roles = roles;
        member.joined_at = QDateTime::fromString(payload["joined_at"].toString(), Qt::ISODate);
        member.premium_since = QDateTime::fromString(payload["premium_since"].toString(), Qt::ISODate);

        guild.members[user.id] = member;
        guild.member_count = guild.members.size();
        guilds[guild.id] = guild;
        return member;
    }
}

void Client::MESSAGE_CREATE(QJsonObject json_message) {
    Client::message_t message;
    message.id = json_message["id"].toString();
    message.channel_id = json_message["channel_id"].toString();
    message.guild_id = json_message["guild_id"].toString();
    message.content = json_message["content"].toString();
    message.timestamp = QDateTime::fromString(json_message["timestamp"].toString(), Qt::ISODate);
    message.edited_time = QDateTime::fromString(json_message["edited_time"].toString(), Qt::ISODate);
    message.tts = json_message["tts"].toBool();
    message.mention_everyone = json_message["mention_everyone"].toBool();
    message.nonce = json_message["nonce"].toString();
    message.pinned = json_message["pinned"].toBool();
    message.webhook_id = json_message["webhook_id"].toString();
    message.type = json_message["type"].toInt();


    Client::guild_t guild = getGuild(message.guild_id);

    message.member = this ->getMember(json_message["author"].toObject()["id"].toString(), message.guild_id);
    message.author = this ->getUser(json_message["author"].toObject()["id"].toString());

    QList<Client::user_t> mentions;
    foreach(const QJsonValue &value, json_message["mentions"].toArray()) {
        QJsonObject json_mention = value.toObject();
        mentions.push_back(this->getUser(json_mention["id"].toString()));
    }
    message.mentions = mentions;

    QList<Client::roles_t> mentioned_roles;
    foreach(const QJsonValue &value, json_message["mention_roles"].toArray()) {
        QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
        for ( ; it != guild.roles.constEnd(); ++it ) {
            const Client::roles_t &role = *it;

            if (value.toString() == role.id) {
                mentioned_roles.push_back(role);
            }
        }
    }
    message.mention_roles = mentioned_roles;


    QList<Client::attachment_t> attachments;
    foreach(const QJsonValue &value, json_message["attachments"].toArray()) {
        QJsonObject json_attachment = value.toObject();
        Client::attachment_t attachment;
        attachment.filename = json_attachment["filename"].toString();
        attachment.height = json_attachment["height"].toInt();
        attachment.id = json_attachment["id"].toString();
        attachment.size = json_attachment["size"].toInt();
        attachment.url = json_attachment["url"].toString();
        attachment.width = json_attachment["width"].toInt();
        attachments.push_back(attachment);
    }
    message.attachments = attachments;

    QList<Client::embed_t> embeds;
    foreach(const QJsonValue &value, json_message["embeds"].toArray()) {
        QJsonObject json_embed = value.toObject();
        Client::embed_t embed;
        embed.title = json_embed["title"].toString();
        embed.description = json_embed["description"].toString();
        embed.url = json_embed["url"].toString();
        embed.timestamp = QDateTime::fromString(json_embed["timestamp"].toString(), Qt::ISODate);
        embed.colour = json_embed["color"].toInt();
        embed.footer_icon_url = json_embed["footer"].toObject()["icon_url"].toString();
        embed.footer_text = json_embed["footer"].toObject()["text"].toString();
        embed.image_url = json_embed["image"].toObject()["url"].toString();
        embed.thumbnail_url = json_embed["thumbnail"].toObject()["url"].toString();

        QList<Client::embed_field_t> fields;
        foreach(const QJsonValue &value, json_embed["fields"].toArray()) {
            QJsonObject json_field = value.toObject();
            Client::embed_field_t field;
            field.is_inline = json_field["inline"].toBool();
            field.name = json_field["name"].toString();
            field.value = json_field["value"].toString();
            fields.push_back(field);
        }
        embed.fields = fields;
        embeds.push_back(embed);
    }
    message.embeds = embeds;

    messages[message.id] = message;

    emit message_create(&message);
}

void Client::INTERACTION_CREATE(QJsonObject json_interaction) {
    Client::interaction_t interaction;
    interaction.channel_id = json_interaction["channel_id"].toString();
    interaction.guild_id = json_interaction["guild_id"].toString();
    interaction.id = json_interaction["id"].toString();
    interaction.token = json_interaction["token"].toString();
    interaction.type = json_interaction["type"].toInt();

    Client::guild_t guild = getGuild(interaction.guild_id);

    interaction.command = json_interaction["data"].toObject()["name"].toString();

    foreach(const QJsonValue &value, json_interaction["data"].toObject()["options"].toArray()) {
        QJsonObject json_option = value.toObject();
        if (json_option["value"].type() == QJsonValue::Bool) {
            interaction.options[json_option["name"].toString()] = json_option["value"].toBool() ? "1" : "0";
        } else if (json_option["value"].type() == QJsonValue::String) {
            interaction.options[json_option["name"].toString()] = json_option["value"].toString();
        } else if (json_option["value"].type() == QJsonValue::Double) {
            interaction.options[json_option["name"].toString()] = QString::number(json_option["value"].toDouble());
        } else if (json_option["options"].type() == QJsonValue::Array) { // sub command

            interaction.sub_group = json_option["name"].toString();
            interaction.sub_option = json_option["options"].toArray().at(0).toObject()["name"].toString();
            if (json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["value"].type() == QJsonValue::Bool) {
                interaction.sub_options[json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["name"].toString()] =
                        json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["value"].toBool() ? "1" : "0";
            } else if (json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["value"].type() == QJsonValue::String) {
                interaction.sub_options[json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["name"].toString()] =
                        json_option["options"].toArray().at(0).toObject()["options"].toArray().at(0).toObject()["value"].toString();
            }

        }
    }

    QJsonObject json_member = json_interaction["member"].toObject();
    QList<Client::roles_t> roles;
    Client::member_t member;
    Client::user_t user;

    user.avatar = json_member["user"].toObject()["avatar"].toString();
    user.bot = json_member["user"].toObject()["bot"].toBool();
    user.discriminator = json_member["user"].toObject()["discriminator"].toString();
    user.flags = json_member["user"].toObject()["flags"].toInt();
    user.id = json_member["user"].toObject()["id"].toString();
    user.mfa_enabled = json_member["user"].toObject()["mfa_enabled"].toBool();
    user.username = json_member["user"].toObject()["username"].toString();
    user.verified = json_member["user"].toObject()["verified"].toBool();
    users[user.id] = user;
    member.user = user;

    member.nick = json_member["nick"].toString().isNull() ? "" : json_member["nick"].toString();

    QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
    foreach(const QJsonValue &roleValue, json_member["roles"].toArray()) {
        QJsonObject json_role = roleValue.toObject();
        Client::roles_t role;

        for ( ; it != guild.roles.constEnd(); ++it ) {
            const Client::roles_t &roleIT= *it;
            if (roleIT.id == json_role["id"].toString()) {
                role = roleIT;
                roles.push_back(role);
                break;
            }
        }
    }
    member.roles = roles;
    member.joined_at = QDateTime::fromString(json_member["joined_at"].toString(), Qt::ISODate);
    member.premium_since = QDateTime::fromString(json_member["premium_since"].toString(), Qt::ISODate);
    member.permissions = json_member["permissions"].toString();

    interaction.member = member;
    emit interaction_create(&interaction);
}


// Logging / Event updates



// GUILD_MEMBER_ADD
void Client::GUILD_MEMBER_ADD(QJsonObject json_member) {
    Client::member_t new_member;
    Client::user_t user;

    user.avatar = json_member["user"].toObject()["avatar"].toString();
    user.discriminator = json_member["user"].toObject()["discriminator"].toString();
    user.id = json_member["user"].toObject()["id"].toString();
    user.flags = json_member["user"].toObject()["public_flags"].toInt();
    user.username = json_member["user"].toObject()["username"].toString();
    user.bot = json_member["user"].toObject()["bot"].toBool();
    users[user.id] = user;
    new_member.user = user;
    new_member.nick = json_member["nick"].toString();

    QList<Client::roles_t> roles;
    foreach(const QJsonValue &value, json_member["roles"].toArray()) {
        QList<Client::roles_t>::ConstIterator it = this->getGuild(json_member["guild_id"].toString()).roles.constBegin();
        for ( ; it != this->getGuild(json_member["guild_id"].toString()).roles.constEnd(); ++it ) {
            const Client::roles_t &role = *it;

            if (value.toString() == role.id) {
                roles.push_back(role);
                break;
            }
        }
    }
    new_member.roles = roles;
    new_member.joined_at = QDateTime::fromString(json_member["joined_at"].toString(), Qt::ISODate);
    new_member.premium_since = QDateTime::fromString(json_member["premium_since"].toString(), Qt::ISODate);

    Client::guild_t guild = this->getGuild(json_member["guild_id"].toString());
    guild.members[user.id] = new_member;
    guild.member_count = guild.members.size();
    guilds[guild.id] = guild;

    QString channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "member_add");
    if (channel_id == "" || enabled == "0" || enabled == "" || (user.bot == true && dbmanager->setting_get(guild.id, "logging", "member_add-bots") == "0")) {
        return;
    }

    Client::embed_t embed;

    embed.title = QString("Logging - Member Add");

    QString description = QString("<@%1> joined as the %2%3 member.").arg(user.id, QString::number(guild.member_count),
                                                                          (QString::number(guild.member_count).endsWith("1") && guild.member_count != 11) ? "st" :
                                                                                                                                                            (QString::number(guild.member_count).endsWith("2") && guild.member_count != 12) ? "nd" :
                                                                                                                                                                                                                                              (QString::number(guild.member_count).endsWith("3") && guild.member_count != 13) ? "rd" : "th");
    // End
    quint64 snowflake = QString(user.id).toULongLong();

    QDateTime then;
    then.setMSecsSinceEpoch((snowflake >> 22) + 1420070400000);
    QDateTime now;
    now.setMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());

    description += QString("\nAccount created %1 ago").arg(this->getAge(then.date(), now.date())); // Date

    embed.description = description;

    embed.colour = -1; // FFB400 - Orange

    embed.author.name = user.username;
    if (user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(user.id, user.avatar);
    }
    this->send_message(channel_id, "", embed);
}

// GUILD_MEMBER_UPDATE - CHANGE TO LOG
void Client::GUILD_MEMBER_UPDATE(QJsonObject json_member) {
    Client::member_t new_member;
    Client::user_t user;

    user.avatar = json_member["user"].toObject()["avatar"].toString();
    user.discriminator = json_member["user"].toObject()["discriminator"].toString();
    user.id = json_member["user"].toObject()["id"].toString();
    user.flags = json_member["user"].toObject()["public_flags"].toInt();
    user.username = json_member["user"].toObject()["username"].toString();
    user.bot = json_member["user"].toObject()["bot"].toBool();
    users[user.id] = user;
    new_member.user = user;
    new_member.nick = json_member["nick"].toString();

    QList<Client::roles_t> roles;
    foreach(const QJsonValue &value, json_member["roles"].toArray()) {
        QList<Client::roles_t>::ConstIterator it = this->getGuild(json_member["guild_id"].toString()).roles.constBegin();
        for ( ; it != this->getGuild(json_member["guild_id"].toString()).roles.constEnd(); ++it ) {
            const Client::roles_t &role = *it;

            if (value.toString() == role.id) {
                roles.push_back(role);
                break;
            }
        }
    }
    new_member.roles = roles;
    new_member.joined_at = QDateTime::fromString(json_member["joined_at"].toString(), Qt::ISODate);
    new_member.premium_since = QDateTime::fromString(json_member["premium_since"].toString(), Qt::ISODate);

    Client::member_t old_member = this->getMember(user.id, json_member["guild_id"].toString());

    Client::guild_t guild = this->getGuild(json_member["guild_id"].toString());
    guild.members[user.id] = new_member;
    guild.member_count = guild.members.size();
    guilds[guild.id] = guild;

    QString channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "member_update");
    if (channel_id == "" || enabled == "0" || enabled == "" || (user.bot == true && dbmanager->setting_get(guild.id, "logging", "member_add-bots") == "0")) {
        return;
    }

    Client::embed_t embed;

    embed.title = QString("Logging - Member Update");

    QList<Client::embed_field_t> fields;

    if (new_member.nick != old_member.nick) {
        Client::embed_field_t field;
        field.name = "Nickname";
        field.value = QString("Before: %1\n+After: %1").arg(old_member.nick, new_member.nick);
        field.is_inline = true;
        fields.push_back(field);
    }

    if (old_member.roles.size() != new_member.roles.size()) {
        Client::embed_field_t field;
        field.name = QString("%1 Role").arg(old_member.roles.size() < new_member.roles.size() ? "New" : "Removed");

        QStringList oldRoles;
        foreach(const Client::roles_t role, old_member.roles) {
            oldRoles.push_back(role.name);
        }
        QStringList newRoles;
        foreach(const Client::roles_t role, new_member.roles) {
            newRoles.push_back(role.name);
        }
        QSet<QString> subtraction;
        QList<Client::roles_t> roles;
        if (old_member.roles.size() < new_member.roles.size()) {
            subtraction = newRoles.toSet().subtract(oldRoles.toSet());
            roles = new_member.roles;
        } else {
            subtraction = oldRoles.toSet().subtract(newRoles.toSet());
            roles = old_member.roles;
        }

        QString field_value;
        foreach (const QString &newRoleName, subtraction) {
            foreach(const Client::roles_t role, roles) {
                if (role.name == newRoleName) {
                    field_value += QString("<@&%1>").arg(role.id);
                }
            }
        }
        field.value = field_value;
        field.is_inline = true;
        fields.push_back(field);
    }

    embed.fields = fields;

    embed.colour = -1; // FFB400 - Orange

    embed.author.name = user.username;
    if (user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(user.id, user.avatar);
    }
    this->send_message(channel_id, "", embed);
    emit guild_member_update(&old_member, &new_member);
}

// GUILD_MEMBER_REMOVE
void Client::GUILD_MEMBER_REMOVE(QJsonObject json_member) {

}


// MESSAGE_CREATE - NOT USED

// MESSAGE_UPDATE
void Client::MESSAGE_UPDATE(QJsonObject json_message) {
    Client::message_t old_message = messages[json_message["id"].toString()];
    QString new_message_content = json_message["content"].toString();
    QString channel_id = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "bind");
    QString enabled = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_update");
    if (channel_id == "" || enabled == "0" || enabled == "" || this->getUser(json_message["author"].toObject()["id"].toString()).bot == true || old_message.content == "" || dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_update") == "0") {
        return;
    }

    Client::embed_t embed;

    embed.title = QString("Logging - Message Update");

    embed.description = QString("Before: %1\n+After: %2").arg(old_message.content, new_message_content);
    embed.colour = -1; // FFB400 - Orange

    embed.author.name = old_message.author.username;
    if (old_message.author.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(old_message.author.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(old_message.author.id, old_message.author.avatar);
    }
    this->send_message(channel_id, "", embed);
}

// MESSAGE_DELETE
void Client::MESSAGE_DELETE(QJsonObject json_message) {
    Client::message_t old_message = messages[json_message["id"].toString()];
    QString channel_id = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "bind");
    QString enabled = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_delete");
    if (channel_id == "" || enabled == "0" || enabled == "" || this->getUser(json_message["author"].toObject()["id"].toString()).bot == true || (old_message.content == "" && old_message.attachments.size() == 0)) {
        return;
    }

    Client::embed_t embed;
    embed.description = QString("Content: %2").arg(old_message.content);

    QList<Client::attachment_t> attachments = old_message.attachments;

    QList<Client::embed_field_t> fields;

    if (attachments.size() > 0) {
        foreach(const Client::attachment_t attachment, attachments) {
            Client::embed_field_t field;
            field.name = attachment.filename;
            field.value = attachment.url;
            field.is_inline = true;
            fields.push_back(field);
        }
    }

    embed.fields = fields;

    embed.colour = -1; // FFB400 - Orange

    embed.title = QString("Logging - Message Delete");


    embed.author.name = old_message.author.username;
    if (old_message.author.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(old_message.author.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(old_message.author.id, old_message.author.avatar);
    }
    this->send_message(channel_id, "", embed);
}


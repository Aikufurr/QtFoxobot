#include "client.h"
#include "websocket.h"

Client::Client(QString _application_id) {
    application_id = _application_id;
}

void Client::login(QString _token) {
    token = _token;

    websocket = new Websocket(token);

    connect(websocket, SIGNAL(READY(QJsonObject)), this, SLOT(READY(QJsonObject)));
    connect(websocket, SIGNAL(GUILD_CREATE(QJsonObject)), this, SLOT(GUILD_CREATE(QJsonObject)));
    connect(websocket, SIGNAL(MESSAGE_CREATE(QJsonObject)), this, SLOT(MESSAGE_CREATE(QJsonObject)));
    connect(websocket, SIGNAL(INTERACTION_CREATE(QJsonObject)), this, SLOT(INTERACTION_CREATE(QJsonObject)));

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
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

    QString urlString = QString("https://discord.com/api/v8/applications/%1").arg(application_id);

    if (guild_id.isEmpty()) {
        urlString += QString("/commands");
    } else {
        urlString += QString("/guilds/%1/commands").arg(guild_id);
    }

    const QUrl url(urlString);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());

    QJsonDocument doc(command);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = mgr->post(request, data);

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
    });
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
    });
}


void Client::send_message(QString channel_id, QString content) {
    embed_t embed;
    this->send_message(channel_id, content, embed);
}

void Client::send_message(QString channel_id, QString content, embed_t embed) {
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
    QByteArray data = doc.toJson();

    QNetworkReply *reply = mgr->post(request, data);

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            //            QString contents = QString::fromUtf8(reply->readAll());
            //            qDebug() << contents;
            qDebug() << "Message Sent";
        }
        else{
            QString err = reply->errorString();
            qDebug() << err;
        }
        reply->deleteLater();
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
    users[user.id] = user;
    qDebug() << "{Client::ready} Loaded";
    emit ready(user.username);
}

void Client::GUILD_CREATE(QJsonObject json_guild) {
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
    guild.member_count = json_guild["member_count"].toInt();
    guild.max_members = json_guild["max_members"].toInt();
    guild.description = json_guild["description"].toString().isNull() ? "" : json_guild["description"].toString();


    {
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
    }
    //    {
    //        QList<Client::member_t> members;
    //        foreach(const QJsonValue &value, json_guild["members"].toArray()) {
    //            QJsonObject json_member = value.toObject();
    //            QList<Client::roles_t> roles;
    //            Client::member_t member;
    //            Client::user_t user;

    //            user.avatar = json_guild["user"].toObject()["avatar"].toString();
    //            user.bot = json_guild["user"].toObject()["bot"].toBool();
    //            user.discriminator = json_guild["user"].toObject()["discriminator"].toString();
    //            user.flags = json_guild["user"].toObject()["flags"].toInt();
    //            user.id = json_guild["user"].toObject()["id"].toString();
    //            user.mfa_enabled = json_guild["user"].toObject()["mfa_enabled"].toBool();
    //            user.username = json_guild["user"].toObject()["username"].toString();
    //            user.verified = json_guild["user"].toObject()["verified"].toBool();
    //            users[user.id] = user;
    //            member.user = user;

    //            member.nick = json_guild["nick"].toString().isNull() ? "" : json_guild["nick"].toString();

    //            QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
    //            foreach(const QJsonValue &roleValue, json_guild["roles"].toArray()) {
    //                QJsonObject json_role = roleValue.toObject();
    //                Client::roles_t role;

    //                for ( ; it != guild.roles.constEnd(); ++it ) {
    //                    const Client::roles_t &roleIT= *it;
    //                    if (roleIT.id == json_role["id"].toString()) {
    //                        role = roleIT;
    //                        roles.push_back(role);
    //                        break;
    //                    }
    //                }
    //            }
    //            member.roles = roles;
    //            member.joined_at = QDateTime::fromString(json_guild["joined_at"].toString(), Qt::ISODate);
    //            member.premium_since = QDateTime::fromString(json_guild["premium_since"].toString(), Qt::ISODate);
    //            member.permissions = json_guild["permissions"].toString();

    //            members.push_back(member);
    //        }
    //        guild.members = members;
    //    }
    {
        foreach(const QJsonValue &value, json_guild["members"].toArray()) {
            QJsonObject json_member = value.toObject();
            QList<Client::roles_t> roles;
            Client::member_t member;
            Client::user_t user;

            user.avatar = json_guild["user"].toObject()["avatar"].toString();
            user.bot = json_guild["user"].toObject()["bot"].toBool();
            user.discriminator = json_guild["user"].toObject()["discriminator"].toString();
            user.flags = json_guild["user"].toObject()["flags"].toInt();
            user.id = json_guild["user"].toObject()["id"].toString();
            user.mfa_enabled = json_guild["user"].toObject()["mfa_enabled"].toBool();
            user.username = json_guild["user"].toObject()["username"].toString();
            user.verified = json_guild["user"].toObject()["verified"].toBool();
            users[user.id] = user;
            member.user = user;

            member.nick = json_guild["nick"].toString().isNull() ? "" : json_guild["nick"].toString();

            QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
            foreach(const QJsonValue &roleValue, json_guild["roles"].toArray()) {
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
            member.joined_at = QDateTime::fromString(json_guild["joined_at"].toString(), Qt::ISODate);
            member.premium_since = QDateTime::fromString(json_guild["premium_since"].toString(), Qt::ISODate);
            member.permissions = json_guild["permissions"].toString();

            guild.members[user.id] = member;
        }
    }
    {
        QList<Client::channel_t> channels;
        foreach(const QJsonValue &value, json_guild["members"].toArray()) {
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
    }

    guilds[guild.id] = guild;
}

Client::guild_t Client::getGuild(QString guild_id) {
    return guilds[guild_id];

    // if not exist try GET
    //    Client::guild_t guild;
    //    return guild;
}
Client::user_t Client::getUser(QString user_id) {
    if (users.contains(user_id)) {
        return users[user_id];
    } else {
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
    if (members.contains(user_id)) {
        return members[user_id];
    } else {
        Client::guild_t guild = this->getGuild(guild_id);
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
        members[user.id] = member;
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

    //    if (!users.contains(json_message["author"].toObject()["id"].toString())) {
    //        QNetworkAccessManager NAManager;
    //        QNetworkRequest request(QUrl(QString("https://discord.com/api/users/%1").arg(json_message["author"].toObject()["id"].toString())));
    //        request.setHeader(QNetworkRequest::UserAgentHeader, "DiscordBot/1.0 (windows)");
    //        request.setRawHeader("Authorization", QString("Bot %1").arg(token).toUtf8());
    //        QNetworkReply *reply = NAManager.get(request);
    //        QEventLoop eventLoop;
    //        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    //        eventLoop.exec();
    //        QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
    //        QJsonObject payload = jsonResponse.object();
    //        qDebug() << "######!!!!!!!!!!!!@@@@@@@@@" << payload;
    //        Client::user_t user;
    //        user.avatar = payload["avatar"].toString();
    //        user.discriminator = payload["discriminator"].toString();
    //        user.id = payload["id"].toString();
    //        user.flags = payload["flags"].toInt();
    //        user.username = payload["username"].toString();
    //        user.bot = payload["bot"].toBool();
    //        users[json_message["author"].toObject()["id"].toString()] = user;
    //    }
    //    message.author = users[json_message["author"].toObject()["id"].toString()];


    Client::user_t user;
    user.avatar = json_message["author"].toObject()["avatar"].toString();
    user.discriminator = json_message["author"].toObject()["discriminator"].toString();
    user.id = json_message["author"].toObject()["id"].toString();
    user.flags = json_message["author"].toObject()["flags"].toInt();
    user.username = json_message["author"].toObject()["username"].toString();
    user.bot = json_message["author"].toObject()["bot"].toBool();
    users[user.id] = user;
    message.author = user;

    Client::member_t member;
    member.user = user;
    member.nick = json_message["member"].toObject()["nick"].toString();

    QList<Client::roles_t> roles;
    foreach(const QJsonValue &value, json_message["member"].toObject()["roles"].toArray()) {
        QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
        for ( ; it != guild.roles.constEnd(); ++it ) {
            const Client::roles_t &role = *it;

            if (value.toString() == role.id) {
                roles.push_back(role);
            }
        }
    }
    member.roles = roles;
    member.joined_at = QDateTime::fromString(json_message["member"].toObject()["joined_at"].toString(), Qt::ISODate);
    member.premium_since = QDateTime::fromString(json_message["member"].toObject()["premium_since"].toString(), Qt::ISODate);

    message.member = member;

    QList<Client::mention_t> mentions;
    foreach(const QJsonValue &value, json_message["mentions"].toArray()) {
        QJsonObject json_mention = value.toObject();
        Client::mention_t mention;

        Client::user_t mentioned_user;
        mentioned_user.avatar = json_mention["avatar"].toString();
        mentioned_user.bot = json_mention["bot"].toBool();
        mentioned_user.discriminator = json_mention["discriminator"].toString();
        mentioned_user.username = json_mention["username"].toString();

        mention.user = mentioned_user;
    }
    message.mentions = mentions;

    QList<Client::roles_t> mentioned_roles;
    foreach(const QJsonValue &value, json_message["mention_roles"].toArray()) {
        QList<Client::roles_t>::ConstIterator it = guild.roles.constBegin();
        for ( ; it != guild.roles.constEnd(); ++it ) {
            const Client::roles_t &role = *it;

            if (value.toString() == role.id) {
                roles.push_back(role);
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
        attachment.proxy_url = json_attachment["proxy_url"].toString();
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

    emit message_create(message);
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

    //    QList<Client::interaction_option_t> interaction_options;
    //    foreach(const QJsonValue &value, json_interaction["data"].toObject()["options"].toArray()) {
    //        QJsonObject json_option = value.toObject();
    //        Client::interaction_option_t interaction_option;
    //        interaction_option.option[json_option["name"].toString()] = json_option["value"].toString();
    //        if (interaction_option.option[json_option["name"].toString()].isEmpty()) {
    //            interaction_option.option[json_option["name"].toString()] = QString::number(json_option["value"].toInt());
    //        }
    //        interaction_options.push_back(interaction_option);
    //    }
    //    interaction.options = interaction_options;

    foreach(const QJsonValue &value, json_interaction["data"].toObject()["options"].toArray()) {
        QJsonObject json_option = value.toObject();
        interaction.options[json_option["name"].toString()] = json_option["value"].toString();
        if (interaction.options[json_option["name"].toString()].isEmpty()) {
            interaction.options[json_option["name"].toString()] = QString::number(json_option["value"].toInt());
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

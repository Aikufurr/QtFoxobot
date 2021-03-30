#include "client.h"

Client::Client(QString _application_id, DbManager *_dbmanager) {
    application_id = _application_id;
    dbmanager = _dbmanager;
}

void Client::channel_bulk_delete(QString channel_id, QHash<QString, Client::message_t> messages_to_delete, Client::interaction_t *interaction) {
    QJsonArray message_ids;
    foreach(const Client::message_t message, messages_to_delete) {
        message_ids.push_back(message.id);
    }
    QJsonObject root;
    QJsonObject obj;
    obj.insert("messages", message_ids);
    root.insert("data", obj);
    QJsonObject payload = QJsonDocument::fromJson(rest->request("POST",  QString("/channels/%1/messages/bulk-delete").arg(channel_id), root)).object();

    Client::guild_t guild = this->guild_get(interaction->guild_id);

    QString log_channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "message_purge");
    if (log_channel_id == "" || enabled == "0" || enabled == "") {
        return;
    }

    QList<QString> description;
    int offset = 0;

    foreach(const Client::message_t message, messages_to_delete) {
        QString insert = QString("[%1#%2]: %3\n").arg(message.author.username,
                                                      message.author.discriminator,
                                                      message.content);
        if (description.size() == 0) {
            description.push_back("");
        }
        if ((description[offset].size() + insert.size()) > 1020) {
            offset++;
        }
        if (description.size()-1 == offset-1) {
            description.push_back("");
        }
        description[offset] += insert;
    }
    offset = 0;

    foreach(const QString &desc, description) {
        Client::embed_t embed;
        embed.title = QString("%1 Messages Purged").arg(messages_to_delete.size());
        if (description.size() > 1) {
            embed.title += QString(" (%1/%2)").arg(QString::number(++offset), QString::number(description.size()));
        }

        embed.description = QString("[#%1](%2)\n%3").arg(
                    guild.channels[messages_to_delete[message_ids.at(0).toString()].channel_id].name,
                QString("https://discord.com/channels/%1/%2/%3").arg(interaction->guild_id,
                                                                     messages_to_delete[message_ids.at(0).toString()].channel_id,
                messages_to_delete[message_ids.at(0).toString()].id),
                desc);


        embed.colour = -1; // FFB400 - Orange

        embed.author.name = interaction->member.user.username;
        if (interaction->member.user.avatar.isNull()) {
            embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
        } else {
            embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
        }
        foreach(const Client::message_t message, messages_to_delete) {
            messages.remove(message.id); // or .erase idk which one to use
        }
        this->send_message(log_channel_id, "", embed);
    }
}

QHash<QString, Client::message_t> Client::channel_messages(QString channel_id) {
    return this->channel_messages(channel_id, 50);
}
QHash<QString, Client::message_t> Client::channel_messages(QString channel_id, int limit) {
    qDebug() << "Pulling channel messages" << channel_id << "limit" << limit;

    QJsonArray payload = QJsonDocument::fromJson(rest->request("GET", QString("/channels/%1/messages?limit=%2").arg(channel_id, QString::number(limit)))).array();

    QHash<QString, Client::message_t> messages_to_send;

    foreach(const QJsonValue &message_value, payload) {
        Client::message_t message = this->message_create(message_value.toObject());
        messages_to_send[message.id] = message;
    }

    return messages_to_send;
}

int Client::days_in_month(QDate date) {
    QDateTime monthStart(QDate(date.year(), date.month(), 1));
    QDateTime monthEnd(QDate(date.year(), date.month() + 1, 1));
    int monthLength = (monthEnd.toMSecsSinceEpoch() - monthStart.toMSecsSinceEpoch()) / (1000 * 60 * 60 * 24);
    return monthLength;
}

QJsonObject Client::embed_to_json(embed_t embed) {
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
        json_embed.insert("timestamp", embed.timestamp.toString(Qt::ISODate));
    }
    if (!embed.thumbnail_url.isEmpty()) {
        QJsonObject json_thumbnail_url;
        json_thumbnail_url.insert("url", embed.thumbnail_url);
        json_embed.insert("thumbnail", json_thumbnail_url);
    }
    if (!embed.image_url.isEmpty()) {
        QJsonObject json_image_url;
        json_image_url.insert("url", embed.image_url);
        json_embed.insert("image", json_image_url);
    }
    if ((!embed.footer.text.isEmpty()) || (!embed.footer.icon_url.isEmpty())) {
        QJsonObject json_image_url;
        if (!embed.footer.text.isEmpty()) {
            json_image_url.insert("text", embed.footer.text);
        }
        if (!embed.footer.text.isEmpty()) {
            json_image_url.insert("icon_url", embed.footer.icon_url);
        }
        json_embed.insert("footer", json_image_url);
    }
    if (!embed.author.name.isEmpty()) {
        QJsonObject json_embed_author;
        json_embed_author.insert("name", embed.author.name);
        json_embed_author.insert("icon_url", embed.author.icon_url);
        json_embed.insert("author", json_embed_author);
    }
    if (embed.fields.size() > 0) {
        QJsonArray fields;
        foreach(const Client::embed_field_t &field, embed.fields) {
            QJsonObject json_embed_field;
            json_embed_field.insert("name", field.name);
            json_embed_field.insert("value", field.value);
            json_embed_field.insert("inline", field.is_inline);
            fields.push_back(json_embed_field);
        }
        json_embed.insert("fields", fields);
    }
    return json_embed;
}

QString Client::get_age(QDate then, QDate now) {
    //--------------------------------------------------------------
    int days = now.day() - then.day();
    if (days < 0) {
        now.setDate(now.year(), now.month()-1, now.day());
        days += this->days_in_month(now);
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
        output += QString("%1 year%2, ").arg(QString::number(years), years == 1 ? "" : "s");
    }
    if (months != 0) {
        output += QString("%1 month%2, ").arg(QString::number(months), months == 1 ? "" : "s");
    }
    if (days != 0) {
        output += QString("%1 day%2").arg(QString::number(days), days == 1 ? "" : "s");
    }

    return output;
}

QString Client::get_time(QDateTime then, QDateTime now) {
    qint64 seconds = now.toSecsSinceEpoch() - then.toSecsSinceEpoch();
    int h = qFloor(seconds / 3600);
    int m = qFloor(seconds % 36000 / 60);
    int s = qFloor(seconds % 36000 % 60);

    QString output;

    output += h > 0 ? QString::number(h) + (h == 1 ? " hour, " : " hours, ") : "";
    output += m > 0 ? QString::number(m) + (m == 1 ? " minute, " : " minutes, ") : "";
    output += s > 0 ? QString::number(s) + (s == 1 ? " second" : " seconds") : "";

    return output;
}


void Client::guild_create(QJsonObject json_guild) {
//    qDebug() << "Function: " << Q_FUNC_INFO << " :-> ID" << json_guild["id"].toString();
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
    guild.member_count = json_guild["member_count"].toInt(); // subject to change
    guild.approximate_member_count = json_guild["approximate_member_count"].toInt() == 0 ? guild.member_count : json_guild["approximate_member_count"].toInt(); // Static
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

    QJsonArray payload = QJsonDocument::fromJson(rest->request("GET", QString("/guilds/%1/members?limit=50").arg(guild.id))).array();

    foreach(const QJsonValue &value, payload) {
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
    }
    guild.member_count = guild.members.size();


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


        guild.channels[channel.id] = channel;
    }

    guilds[guild.id] = guild;
}

void Client::GUILD_CREATE(QJsonObject json_guild) {
    this->guild_create(json_guild);
}
Client::guild_t Client::guild_get(QString guild_id) {
    if (guild_id == "" || guild_id.isEmpty()) {
        Client::guild_t guild;
        return guild;
    }
    qDebug() << "{Client::guild_get} contains guild" << guilds.contains(guild_id);
    if (guilds.contains(guild_id)) {
        qDebug() << "{Client::guild_get} mem" << guilds[guild_id].member_count << guilds[guild_id].approximate_member_count;
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
        json_guild["approximate_member_count"] = json_guild["approximate_member_count"].toInt();

        this->guild_create(json_guild);

        return guilds[guild_id];
    }
}
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
        QList<Client::roles_t>::ConstIterator it = this->guild_get(json_member["guild_id"].toString()).roles.constBegin();
        for ( ; it != this->guild_get(json_member["guild_id"].toString()).roles.constEnd(); ++it ) {
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

    Client::guild_t guild = this->guild_get(json_member["guild_id"].toString());
    guild.members[user.id] = new_member;
    guild.member_count = guild.members.size();
    guild.approximate_member_count++;
    guilds[guild.id] = guild;

    QString channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "member_add");
    if (channel_id == "" || enabled == "0" || enabled == "" || (user.bot == true && dbmanager->setting_get(guild.id, "logging", "member_add-bots") == "0")) {
        return;
    }

    Client::embed_t embed;

    embed.title = QString("Logging - Member Add");

    QString description = QString("<@%1> joined as the %2%3 member.").arg(user.id, QString::number(guild.approximate_member_count),
                                                                          (QString::number(guild.approximate_member_count).endsWith("1") && guild.approximate_member_count != 11) ? "st" :
                                                                                                                                                                                    (QString::number(guild.approximate_member_count).endsWith("2") && guild.approximate_member_count != 12) ? "nd" :
                                                                                                                                                                                                                                                                                              (QString::number(guild.approximate_member_count).endsWith("3") && guild.approximate_member_count != 13) ? "rd" : "th");
    // End
    quint64 snowflake = QString(user.id).toULongLong();

    QDateTime then;
    then.setMSecsSinceEpoch((snowflake >> 22) + 1420070400000);
    QDateTime now;
    now.setMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());

    if (then.daysTo(now) == 0) {
        description += QString("\nAccount created %1 ago").arg(this->get_time(then, now)); // Time
    } else {
        description += QString("\nAccount created %1 ago").arg(this->get_age(then.date(), now.date())); // Date
    }

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

// GUILD_MEMBER_UPDATE
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
        QList<Client::roles_t>::ConstIterator it = this->guild_get(json_member["guild_id"].toString()).roles.constBegin();
        for ( ; it != this->guild_get(json_member["guild_id"].toString()).roles.constEnd(); ++it ) {
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

    Client::member_t old_member = this->member_get(user.id, json_member["guild_id"].toString());

    Client::guild_t guild = this->guild_get(json_member["guild_id"].toString());
    guild.members[user.id] = new_member;
    guild.member_count = guild.members.size();
    guilds[guild.id] = guild;

    QString channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "member_update");
    if (channel_id == "" || enabled == "0" || enabled == "" || (user.bot == true && dbmanager->setting_get(guild.id, "logging", "member_add-bots") == "0")) {
        return;
    }
    QString roles_enabled = dbmanager->setting_get(guild.id, "logging", "member_update-roles");

    Client::embed_t embed;

    embed.title = QString("Logging - Member Update");

    if (new_member.nick != old_member.nick) {
        Client::embed_field_t field;
        field.name = "Nickname";
        QString value = "Before: ";
        if (old_member.nick.isEmpty()) {
            value += "[None]";
        } else {
            value += old_member.nick;
        }
        value += "\n+After: ";
        if (new_member.nick.isEmpty()) {
            value += "[None]";
        } else {
            value += new_member.nick;
        }
        field.value = value;
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    if ((old_member.roles.size() != new_member.roles.size()) && roles_enabled == "1") {
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
                    field_value += QString("<@&%1> ").arg(role.id);
                }
            }
        }
        field.value = field_value;
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    if (embed.fields.size() == 0) {
        return;
    }

    embed.colour = -1; // FFB400 - Orange

    embed.author.name = user.username;
    if (user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(user.id, user.avatar);
    }
    this->send_message(channel_id, "", embed);
}

// GUILD_MEMBER_REMOVE
void Client::GUILD_MEMBER_REMOVE(QJsonObject json_member) {
    Client::guild_t guild = this->guild_get(json_member["guild_id"].toString());

    if (!guild.members.contains(json_member["user"].toObject()["id"].toString())) {
        return;
    }

    Client::member_t old_member = guild.members[json_member["user"].toObject()["id"].toString()];

    guild.members.remove(old_member.user.id); // or .erase idk which one to use
    guild.member_count = guild.members.size();
    guild.approximate_member_count--;
    guilds[guild.id] = guild;

    QString channel_id = dbmanager->setting_get(guild.id, "logging", "bind");
    QString enabled = dbmanager->setting_get(guild.id, "logging", "member_remove");
    if (channel_id == "" || enabled == "0" || enabled == "" || (old_member.user.bot == true && dbmanager->setting_get(guild.id, "logging", "member_remove-bots") == "0")) {
        return;
    }

    Client::embed_t embed;

    embed.title = QString("Logging - Member Remove");

    {
        Client::embed_field_t field;
        field.name = "Nickname";
        field.value = old_member.nick.isEmpty() ? "[None]" : old_member.nick;
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    {
        Client::embed_field_t field;
        field.name = "Roles";

        QString field_value;
        if (old_member.roles.size() > 0) {
            foreach(const Client::roles_t role, old_member.roles) {
                field_value += QString("<@&%1> ").arg(role.id);
            }
        } else {
            field_value = "[None]";
        }
        field.value = field_value;
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    {
        Client::embed_field_t field;

        QDateTime now;
        now.setMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());

        field.name = "Joined the guild";
        if (old_member.joined_at.daysTo(now) == 0) {
            field.value = QString("%1 ago").arg(this->get_time(old_member.joined_at, now));
        } else {
            field.value = QString("%1 ago").arg(this->get_age(old_member.joined_at.date(), now.date()));
        }
        embed.fields.push_back(field);
    }

    embed.colour = -1; // FFB400 - Orange

    embed.author.name = old_member.user.username;
    if (old_member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(old_member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(old_member.user.id, old_member.user.avatar);
    }
    this->send_message(channel_id, "", embed);
}

void Client::GUILD_UPDATE(QJsonObject json_guild) {
    this->guild_create(json_guild);
}

void Client::INTERACTION_CREATE(QJsonObject json_interaction) {
    //    this->typing(json_interaction["channel_id"].toString());

    /* Ack */
    {
        QJsonObject root;
        QJsonObject obj;
        obj.insert("type", 5);
        QJsonObject data;
        data.insert("content", "Loading...");
        obj.insert("data", data);
        root.insert("data", obj);
        rest->request("POST", QString("/interactions/%1/%2/callback").arg(
                          json_interaction["id"].toString(), json_interaction["token"].toString()), root);
    }

    // Process
    Client::interaction_t interaction;
    interaction.channel_id = json_interaction["channel_id"].toString();
    interaction.guild_id = json_interaction["guild_id"].toString();
    interaction.id = json_interaction["id"].toString();
    interaction.token = json_interaction["token"].toString();
    interaction.type = json_interaction["type"].toInt();


    interaction.command = json_interaction["data"].toObject()["name"].toString();

    foreach(const QJsonValue &value, json_interaction["data"].toObject()["options"].toArray()) {
        QJsonObject json_option = value.toObject();

        interaction.sub_group = json_option["name"].toString();

        if (json_option["value"].type() == QJsonValue::Bool) {
            interaction.options[json_option["name"].toString()] = json_option["value"].toBool() ? "1" : "0";
        } else if (json_option["value"].type() == QJsonValue::String) {
            interaction.options[json_option["name"].toString()] = json_option["value"].toString();
        } else if (json_option["value"].type() == QJsonValue::Double) {
            interaction.options[json_option["name"].toString()] = QString::number(json_option["value"].toDouble());
        } else if (json_option["options"].type() == QJsonValue::Array) { // sub command

            QJsonArray sub_options;
            if (json_option["options"].toArray().at(0).toObject()["options"].toArray().size() == 0) {
                sub_options = json_option["options"].toArray();
            } else {
                interaction.sub_option = json_option["options"].toArray().at(0).toObject()["name"].toString();
                sub_options = json_option["options"].toArray().at(0).toObject()["options"].toArray();
            }

            foreach(const QJsonValue &sub_option, sub_options) {
                QJsonObject json_sub_option = sub_option.toObject();
                if (json_sub_option["value"].type() == QJsonValue::Bool) {
                    interaction.sub_options[json_sub_option["name"].toString()] =
                            json_sub_option["value"].toBool() ? "1" : "0";
                } else if (json_sub_option["value"].type() == QJsonValue::String) {
                    interaction.sub_options[json_sub_option["name"].toString()] =
                            json_sub_option["value"].toString();
                } else if (json_sub_option["value"].type() == QJsonValue::Double) {
                    interaction.sub_options[json_sub_option["name"].toString()] =
                            QString::number(json_sub_option["value"].toDouble());
                }
            }

        }
    }

    QJsonObject json_member = json_interaction["member"].toObject();

    interaction.member = this->member_get(json_member["user"].toObject()["id"].toString(), interaction.guild_id);
    interaction.member.permissions = json_interaction["member"].toObject()["permissions"].toString();

    Client::guild_t guild = guild_get(interaction.guild_id);
    guild.members[interaction.member.user.id] = interaction.member;
    guild.member_count = guild.members.size();
    guilds[guild.id] = guild;

    emit interaction_create(&interaction);

    QString channel_id = dbmanager->setting_get(interaction.guild_id, "logging", "bind");
    QString enabled = dbmanager->setting_get(interaction.guild_id, "logging", "interaction");
    if (channel_id == "" || enabled == "0" || enabled == "" || interaction.member.user.bot == true) {
        return;
    }

    Client::embed_t embed;
    embed.description = QString("Command: %1").arg(interaction.command);

    if (interaction.options.size() > 0 || interaction.sub_options.size() > 0) {

        Client::embed_field_t field;
        field.name = "Arguments";
        QString output;

        if (interaction.options.size() > 0) {
            QHash<QString, QString>::const_iterator i = interaction.options.constBegin();
            while (i != interaction.options.constEnd()) {
                output += QString("%1: %2\n").arg(i.key(), i.value());
                i++;
            }
        }
        if (interaction.sub_options.size() > 0) {
            QHash<QString, QString>::const_iterator i = interaction.sub_options.constBegin();
            while (i != interaction.sub_options.constEnd()) {
                output += QString("%1: %2\n").arg(i.key(), i.value());
                i++;
            }
        }

        field.value = output;
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    embed.colour = -1; // FFB400 - Orange

    embed.title = QString("Logging - Interaction");


    embed.author.name = interaction.member.user.username;
    if (interaction.member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction.member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction.member.user.id, interaction.member.user.avatar);
    }
    this->send_message(channel_id, "", embed);
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
    connect(websocket, SIGNAL(MESSAGE_REACTION_ADD(QJsonObject)), this, SLOT(MESSAGE_REACTION_ADD(QJsonObject)));

    websocket->start();

    rest = new RESTManager("Bot " + token, 60000, 500, 1);
}

void Client::me_change_presence(QString name, int type, QString status) {
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

void Client::me_gateway_get() {
    QByteArray result = rest->request("GET", "/gateway/bot");
    qDebug() << "{Client::me_getewat_get}" << QJsonDocument::fromJson(result).object();
}


Client::user_t Client::me_get() {
    return me;
}

void Client::me_slash_command_create(QJsonObject command, QString guild_id) {
    if (dbmanager->cmd_exists(command["name"].toString())) {

        QJsonDocument jDocument(dbmanager->cmd_get(command["name"].toString()));
        QJsonDocument jCommand(command);

        if (jDocument.toJson(QJsonDocument::Compact) == jCommand.toJson(QJsonDocument::Compact)) {
            qDebug() << "Skipping" << command["name"].toString() << "- Already exists";
            return;
        }
    }

    QString url = QString("/applications/%1").arg(application_id);

    if (command["name"].toString() == "close" && !guild_id.isEmpty()) {
        qDebug() << "Creating slash command" << command["name"].toString() << "locally";
        url += QString("/guilds/%1/commands").arg(guild_id);
    } else {
        if (guild_id.isEmpty()) {
            qDebug() << "Creating slash command" << command["name"].toString() << "globally";
            url += QString("/commands");
        } else {
            qDebug() << "Creating slash command" << command["name"].toString() << "locally";
            url += QString("/guilds/%1/commands").arg(guild_id);
        }
    }


    QJsonObject root;
    root.insert("data", command);
    QJsonObject payload = QJsonDocument::fromJson(rest->request("POST", url, root)).object();

    if (payload.keys().size() == 0) {
        qDebug() << "Payload returned empty";
        return;
    }
    QJsonDocument jDocument(command);
    dbmanager->cmd_set(payload["id"].toString(), payload["name"].toString(), jDocument.toJson(QJsonDocument::Compact));
}

void Client::me_slash_command_delete(QString command_id, QString guild_id) {
    QString url = QString("/applications/%1").arg(application_id);

    if (guild_id.isEmpty()) {
        url += QString("/commands/%1").arg(command_id);
    } else {
        url += QString("/guilds/%1/commands/%1").arg(guild_id, command_id);
    }

    rest->request("DELETE", url);
}


Client::member_t Client::member_get(QString user_id, QString guild_id) {
    if (user_id == "" || user_id.isEmpty() || guild_id == "" || guild_id.isEmpty()) {
        Client::member_t member;
        return member;
    }
    Client::guild_t guild = this->guild_get(guild_id);
    if (guild.members.contains(user_id)) {
        return guild.members[user_id];
    } else {
        qDebug() << "Pulling member" << user_id << "from" << guild_id;
        Client::user_t user = this->user_get(user_id);

        QJsonObject payload = QJsonDocument::fromJson(rest->request("GET", QString("/guilds/%1/members/%2").arg(guild_id, user_id))).object();

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

void Client::message_delete(QString channel_id, QString message_id) {
    messages.remove(message_id);
    rest->request("DELETE", QString("/channels/%1/messages/%2").arg(channel_id, message_id));
}

Client::message_t Client::message_edit(QString channel_id, QString message_id, QString content, embed_t embed) {
    QJsonObject obj;
    if (!content.isEmpty()) {
        obj.insert("content", content);
    }
    if (!embed.title.isEmpty()) {
        obj.insert("embed", this->embed_to_json(embed));
    }

    QJsonObject root;
    root.insert("data", obj);

    return this->message_create(QJsonDocument::fromJson(rest->request("PATCH", QString("/channels/%1/messages/%2").arg(channel_id, message_id), root)).object());
}

// MESSAGE_DELETE
void Client::MESSAGE_DELETE(QJsonObject json_message) {
    Client::message_t old_message = messages[json_message["id"].toString()];
    QString channel_id = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "bind");
    QString enabled = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_delete");
    if (channel_id == "" || enabled == "0" || enabled == "" || this->user_get(json_message["author"].toObject()["id"].toString()).bot == true || (old_message.content == "" && old_message.attachments.size() == 0)) {
        return;
    }

    Client::guild_t guild = this->guild_get(old_message.guild_id);
    Client::embed_t embed;
    embed.description = QString("[#%1](%2)\nContent: %3").arg(guild.channels[old_message.channel_id].name, QString("https://discord.com/channels/%1/%2/%3").arg(old_message.guild_id, old_message.channel_id, old_message.id), old_message.content);

    QList<Client::attachment_t> attachments = old_message.attachments;


    if (attachments.size() > 0) {
        foreach(const Client::attachment_t attachment, attachments) {
            Client::embed_field_t field;
            field.name = attachment.filename;
            field.value = attachment.url;
            field.is_inline = true;
            embed.fields.push_back(field);
        }
    }

    embed.colour = -1; // FFB400 - Orange

    embed.title = QString("Logging - Message Delete");


    embed.author.name = old_message.author.username;
    if (old_message.author.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(old_message.author.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(old_message.author.id, old_message.author.avatar);
    }

    messages.remove(old_message.id); // or .erase idk which one to use
    this->send_message(channel_id, "", embed);
}



void Client::MESSAGE_REACTION_ADD(QJsonObject json_reaction) {
    Client::reaction_t reaction;
    reaction.count = 0;
    reaction.me = json_reaction["member"].toObject()["user"].toObject()["id"].toString() == me.id;
    reaction.user = this->user_get(json_reaction["member"].toObject()["user"].toObject()["id"].toString());
    reaction.emoji.id = json_reaction["emoji"].toObject()["id"].toString();
    reaction.emoji.name = json_reaction["emoji"].toObject()["name"].toString();
    reaction.message_id = json_reaction["message_id"].toString();

    emit message_reaction_add(reaction);
}

Client::message_t Client::message_create(QJsonObject json_message) {
    if (!json_message.contains("guild_id")) {
        if (messages.contains(json_message["id"].toString())) {
            return messages[json_message["id"].toString()];
        } else {
            json_message = QJsonDocument::fromJson(rest->request("GET", QString("/channels/%1/messages/%2").arg(json_message["channel_id"].toString(), json_message["id"].toString()))).object();
        }
    }
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


    Client::guild_t guild = guild_get(message.guild_id);

    message.member = this ->member_get(json_message["author"].toObject()["id"].toString(), message.guild_id);
    message.author = this ->user_get(json_message["author"].toObject()["id"].toString());

    QList<Client::user_t> mentions;
    foreach(const QJsonValue &value, json_message["mentions"].toArray()) {
        QJsonObject json_mention = value.toObject();
        mentions.push_back(this->user_get(json_mention["id"].toString()));
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
        embed.footer.icon_url = json_embed["footer"].toObject()["icon_url"].toString();
        embed.footer.text = json_embed["footer"].toObject()["text"].toString();
        embed.image_url = json_embed["image"].toObject()["url"].toString();
        embed.thumbnail_url = json_embed["thumbnail"].toObject()["url"].toString();

        foreach(const QJsonValue &value, json_embed["fields"].toArray()) {
            QJsonObject json_field = value.toObject();
            Client::embed_field_t field;
            field.is_inline = json_field["inline"].toBool();
            field.name = json_field["name"].toString();
            field.value = json_field["value"].toString();
            embed.fields.push_back(field);
        }
        embeds.push_back(embed);
    }
    message.embeds = embeds;

    messages[message.id] = message;

    return message;
}
void Client::MESSAGE_CREATE(QJsonObject json_message) {
    if (json_message["content"].toString().startsWith("</")) {
        qDebug() << "boogie null";
    } else {
        qDebug() << "no boogies found (yet)";
        Client::message_t msg = this->message_create(json_message);
        emit message(&msg);
    }
}

void Client::message_reaction_create(QString channel_id, QString message_id, QString emoji) {
    rest->request("PUT", QString("/channels/%1/messages/%2/reactions/%3/@me").arg(channel_id, message_id, emoji));
}

void Client::message_reaction_delete(QString channel_id, QString message_id, QString emoji) {
    this->message_reaction_delete(channel_id, message_id, emoji, "");
}

void Client::message_reaction_delete(QString channel_id, QString message_id, QString emoji, QString user_id) {
    QString url = QString("/channels/%1/messages/%2/reactions/%3").arg(channel_id, message_id, emoji);
    if (user_id != "") {
        url += QString("/%1").arg(user_id);
    }
    rest->request("DELETE", url);
}

void Client::message_reaction_delete_all(QString channel_id, QString message_id) {
    rest->request("DELETE", QString("/channels/%1/messages/%2/reactions").arg(channel_id, message_id));
}

// MESSAGE_UPDATE
void Client::MESSAGE_UPDATE(QJsonObject json_message) {
    Client::message_t old_message = messages[json_message["id"].toString()];
    QString new_message_content = json_message["content"].toString();
    QString channel_id = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "bind");
    QString enabled = dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_update");
    if (channel_id == "" || enabled == "0" || enabled == "" || old_message.author.bot == true || (old_message.content == "" && !messages.contains(old_message.id)) || dbmanager->setting_get(json_message["guild_id"].toString(), "logging", "message_update") == "0") {
        return;
    }


    Client::guild_t guild = this->guild_get(old_message.guild_id);

    Client::embed_t embed;

    embed.title = QString("Logging - Message Update");

    QString before;
    if (old_message.content.isEmpty()) {
        before = "[None]";
    } else {
        before = Util::escapeMarkdown(old_message.content);
    }
    QString after;
    if (new_message_content.isEmpty()) {
        return;
    } else {
        after = Util::escapeMarkdown(new_message_content);
    }

    qDebug().noquote() << old_message.content;
    qDebug().noquote() << Util::escapeMarkdown(old_message.content);
    qDebug().noquote() << new_message_content;
    qDebug().noquote() << Util::escapeMarkdown(new_message_content);

    embed.description = QString("[#%1](%2)\nBefore: %3\n+After: %4").arg(guild.channels[old_message.channel_id].name, QString("https://discord.com/channels/%1/%2/%3").arg(old_message.guild_id, old_message.channel_id, old_message.id), before, after);
    embed.colour = -1; // FFB400 - Orange

    embed.author.name = old_message.author.username;
    if (old_message.author.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(old_message.author.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(old_message.author.id, old_message.author.avatar);
    }
    this->send_message(channel_id, "", embed);
}

void Client::READY(QJsonObject response) {
    QJsonObject userResponse = response["user"].toObject();
    me.avatar = userResponse["avatar"].toString();
    me.bot = userResponse["bot"].toBool();
    me.discriminator = userResponse["discriminator"].toString();
    me.flags = userResponse["flags"].toInt();
    me.id = userResponse["id"].toString();
    me.mfa_enabled= userResponse["mfa_enabled"].toBool();
    me.username = userResponse["username"].toString();
    me.verified = userResponse["verified"].toBool();
    users[me.id] = me;
    qDebug() << "{Client::ready} Loaded" << me.username;
    emit ready(me.username);
}

Client::message_t Client::send_file_message(QString channel_id, QByteArray file) {
    embed_t embed;
    return this->send_file_message(channel_id, file, embed);
}

Client::message_t Client::send_file_message(QString channel_id, QByteArray _file, embed_t embed) {
    QJsonObject root;

    QJsonObject obj;
    if (!embed.title.isEmpty()) {
        obj.insert("embed", this->embed_to_json(embed));
    }

    root.insert("data", obj);

    QJsonArray files;
    files.push_back(QString(_file));
    QJsonArray file_names;
    file_names.push_back("rank_chart.png");

    root.insert("files", files);
    root.insert("file_names", file_names);
    return this->message_create(QJsonDocument::fromJson(rest->request("POST", QString("/channels/%1/messages").arg(channel_id), root)).object());
}

Client::message_t Client::send_message(QString channel_id, QString content) {
    embed_t embed;
    return this->send_message(channel_id, content, embed);
}

Client::message_t Client::send_message(QString channel_id, QString content, embed_t embed) {
    QJsonObject root;
    QJsonObject obj;
    if (!content.isEmpty()) {
        obj.insert("content", content);
    }
    if (!embed.title.isEmpty()) {
        obj.insert("embed", this->embed_to_json(embed));
    }

    root.insert("data", obj);
    return this->message_create(QJsonDocument::fromJson(rest->request("POST", QString("/channels/%1/messages").arg(channel_id), root)).object());
}

void Client::typing(QString channel_id) {
    rest->request("POST", QString("/channels/%1/typing").arg(channel_id));
}

Client::user_t Client::user_get(QString user_id) {
    if (user_id == "" || user_id.isEmpty()) {
        Client::user_t user;
        return user;
    }
    if (users.contains(user_id)) {
        return users[user_id];
    } else {
        qDebug() << "Pulling user" << user_id;

        QJsonObject payload = QJsonDocument::fromJson(rest->request("GET", QString("/users/%1").arg(user_id))).object();

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

Client::message_t Client::webhook_edit_message(QString token, QString content) {
    embed_t embed;
    return this->webhook_edit_message(token, content, embed);
}
Client::message_t Client::webhook_edit_message(QString token, QString content, QString id) {
    embed_t embed;
    return this->webhook_edit_message(token, content, embed, id);
}

Client::message_t Client::webhook_edit_message(QString token, QString content, embed_t embed) {
    return this->webhook_edit_message(token, content, embed, "");
}

Client::message_t Client::webhook_edit_message(QString token, QString content, embed_t embed, QString id) {
//    qDebug() << "debug 2";
    QJsonObject obj;
    obj.insert("content", content.isEmpty() ? "​" : content);
    if (!embed.title.isEmpty()) {
        QJsonArray arr;
        arr.push_back(this->embed_to_json(embed));
        obj.insert("embeds", arr);
    }

    QJsonObject root;
    root.insert("data", obj);
//    qDebug() << "debug 3";
    QJsonObject response = QJsonDocument::fromJson(rest->request("PATCH", QString("/webhooks/%1/%2/messages/%3").arg(
                                                                     application_id, token,
                                                                     (id.isEmpty() ? "@original" : id)), root)).object();
//    qDebug() << "debug 21";
    Client::message_t msg = this->message_create(response);
//    qDebug() << "debug 22";
    return msg;
}

Client::message_t Client::webhook_followup_message(QString token, QString content) {
    embed_t embed;
    return this->webhook_followup_message(token, content, embed);
}

Client::message_t Client::webhook_followup_message(QString token, QString content, embed_t embed) {
    QJsonObject obj;
    obj.insert("content", content);
    if (!embed.title.isEmpty()) {
        QJsonArray arr;
        arr.push_back(this->embed_to_json(embed));
        obj.insert("embeds", arr);
    }

    QJsonObject root;
    root.insert("data", obj);
    return this->message_create(QJsonDocument::fromJson(rest->request("POST", QString("/webhooks/%1/%2").arg(
                                                                          application_id, token), root)).object());
}

Client::message_t Client::webhook_followup_message(QString token, QString content, embed_t embed, QByteArray _file) {
    QJsonObject root;

    QJsonObject obj;
    obj.insert("content", content);
    if (!embed.title.isEmpty()) {
        QJsonArray arr;
        arr.push_back(this->embed_to_json(embed));
        obj.insert("embeds", arr);
    }

    root.insert("data", obj);

    QHash<QByteArray, QByteArray> files;
    files["rank_chart.png"] = _file;

    return this->message_create(QJsonDocument::fromJson(rest->request("POST", QString("/webhooks/%1/%2").arg(
                                                                          application_id, token), root, files)).object());
}

Client::message_t Client::webhook_followup_message(QString token, QString content, QByteArray _file) {
    QJsonObject root;

    QJsonObject obj;
    obj.insert("content", content);
    root.insert("data", obj);
    QHash<QByteArray, QByteArray> files;
    files["rank_chart.png"] = _file;

    return this->message_create(QJsonDocument::fromJson(rest->request("POST", QString("/webhooks/%1/%2").arg(
                                                                          application_id, token), root, files)).object());
}


QByteArray Client::custom_request() {
    QJsonObject root;
    QJsonObject obj;
    obj.insert("fuck", "you");
    root.insert("data", obj);
    QByteArray result = rest->custom_request("POST", "http://127.0.0.1:8080/messages", root);
    qDebug() << "Result" << result;
    QJsonDocument::fromJson(result);
    return result;
}
// End

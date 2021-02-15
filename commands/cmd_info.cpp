#include "cmd_info.h"

cmd_info::cmd_info(Client *client, Client::interaction_t *interaction) {
    Client::member_t member;

    if (interaction->options.size() > 0) {
        member = client->getMember(interaction->options["user"], interaction->guild_id);
    } else {
        member = interaction->member;
    }

    Client::embed_t embed;

    embed.title = QString("About %1").arg(member.user.username);
    QString value;
    if (member.nick.isEmpty()) {
        value = "[None]";
    } else {
        value = member.nick;
    }
    embed.description = QString("Nickname: %1\nUsername: %2#%3").arg(value, member.user.username, member.user.discriminator);
    embed.colour = -1;

    embed.footer.text = QString("ID: %1").arg(member.user.id);
    if (interaction->member.user.avatar.isNull()) {
        embed.footer.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(member.user.discriminator.toInt() % 5));
        embed.thumbnail_url = embed.footer.icon_url;
    } else {
        embed.footer.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(member.user.id, member.user.avatar);
        embed.thumbnail_url = embed.footer.icon_url;
    }

    {
        Client::embed_field_t field;
        field.name = "Roles";

        QString field_value;
        if (member.roles.size() > 0) {
            foreach(const Client::roles_t role, member.roles) {
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
        quint64 snowflake = QString(member.user.id).toULongLong();

        QDateTime then;
        then.setMSecsSinceEpoch((snowflake >> 22) + 1420070400000);
        QDateTime now;
        now.setMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());

        field.name = "Account created";
        field.value = then.toString(Qt::ISODate).replace("T", " ");
        if (member.joined_at.daysTo(now) == 0) {
            field.value += QString("\n%1 ago").arg(client->getTime(then, now));
        } else {
            field.value += QString("\n%1 ago").arg(client->getAge(then.date(), now.date()));
        }
        embed.fields.push_back(field);
    }

    {
        Client::embed_field_t field;

        QDateTime now;
        now.setMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch());

        field.name = "Joined the guild";
        field.value = member.joined_at.toString(Qt::ISODate).replace("T", " ");
        if (member.joined_at.daysTo(now) == 0) {
            field.value += QString("\n%1 ago").arg(client->getTime(member.joined_at, now));
        } else {
            field.value += QString("\n%1 ago").arg(client->getAge(member.joined_at.date(), now.date()));
        }
        embed.fields.push_back(field);
    }

    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    client->send_message(interaction->channel_id, "", embed);
}

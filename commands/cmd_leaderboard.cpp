#include "cmd_leaderboard.h"

cmd_leaderboard::cmd_leaderboard(Client *client, Client::interaction_t *interaction, DbManager *dbManager) {
    Client::guild_t guild = client->getGuild(interaction->guild_id);
    int amount = 10;
    if (interaction->options.size() > 0) {
        amount = interaction->options["amount"].toInt();
    }

    QJsonArray leaderboard = dbManager->rank_leaderboard(interaction->guild_id, amount);

    Client::embed_t embed;
    embed.title = QString("%1's Leaderboard").arg(guild.name);
    embed.description = QString("The top %1 members").arg(amount);
    embed.colour = 16757760; // FFB400 - Orange

    QList<Client::embed_field_t> fields;
    foreach(const QJsonValue &value, leaderboard) {
        QJsonObject result = value.toObject();
        Client:: member_t member = client->getMember(result["user_id"].toString(), interaction->guild_id);

        Client::embed_field_t field;
        field.name = QString("Rank %1").arg(QString::number(result["position"].toInt()));
        field.value = QString("**%1** %2 at %3 message%4").arg(member.nick.isEmpty() ? member.user.username : member.nick, member.nick.isEmpty() ? "" : QString("*%1*").arg(member.user.username), QString::number(result["rank"].toInt()), result["rank"].toInt() == 1 ? "" : "s");
        field.is_inline = false;
        fields.push_back(field);

    }
    embed.fields = fields;
    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    if (!guild.icon.isNull()) {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/icons/%1/%2.%3").arg(interaction->guild_id, guild.icon, guild.icon.startsWith("a_") ? "gif" : "png");
    }
    client->send_message(interaction->channel_id, "", embed);
}

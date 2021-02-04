#include "cmd_rank.h"

cmd_rank::cmd_rank(Client *client, Client::interaction_t *interaction, DbManager *dbManager) {
    Client::member_t behind_member;
    Client::member_t member;
    Client::embed_t embed;
    QJsonObject rank;
    if (interaction->options.size() > 0) {
        rank = dbManager->rank_position(interaction->options["user"], interaction->guild_id);
        //            qDebug() << rank;
        member = client->getMember(interaction->options["user"], interaction->guild_id);
    } else {
        rank = dbManager->rank_position(interaction->member.user.id, interaction->guild_id);
        member = interaction->member;
    }
    if (!rank["behind_user_id"].toString().isEmpty()) {
        behind_member = client->getMember(rank["behind_user_id"].toString(), interaction->guild_id);
    }
    embed.title = QString("%1's Rank").arg(member.nick.isEmpty() ? member.user.username : member.nick);
    embed.colour = 16757760; // FFB400 - Orange

    QList<Client::embed_field_t> fields;

    {
        Client::embed_field_t field;
        field.name = "Rank";
        field.value = QString("%1/%2").arg(QString::number(rank["position"].toInt()), QString::number(rank["count"].toInt()));
        field.is_inline = true;
        fields.push_back(field);
    }
    {
        Client::embed_field_t field;
        field.name = "Messages Sent";
        field.value = QString::number(rank["rank"].toInt());
        field.is_inline = true;
        fields.push_back(field);
    }
    if (rank["position"].toInt() == 1) {
        Client::embed_field_t field;
        field.name = QString("%1 are ranked 1st on this server!").arg(member.user.id == interaction->member.user.id ? "Congratulations! You" : "They");
        field.value = QString("%1").arg(member.user.id == interaction->member.user.id ? "Give yourself a pat on the back" : " ‍ ");
        field.is_inline = false;
        fields.push_back(field);
    } else {
        Client::embed_field_t field;
        field.name = QString("To get to %1's rank of %2, %3 need %4 more message%6").arg(behind_member.nick.isEmpty() ? behind_member.user.username : behind_member.nick, QString::number(rank["behind_position"].toInt()), member.user.id == interaction->member.user.id ? "you" : "they", QString::number((rank["behind_rank"].toInt()-rank["rank"].toInt())+1), ((rank["behind_rank"].toInt()-rank["rank"].toInt())+1) != 1 ? "s" : "");
        field.value = " ‍ ";
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
    if (member.user.avatar.isNull()) {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(member.user.discriminator.toInt() % 5));
    } else {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(member.user.id, member.user.avatar);
    }
    client->send_message(interaction->channel_id, "", embed);
}

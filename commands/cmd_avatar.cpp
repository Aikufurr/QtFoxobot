#include "cmd_avatar.h"

cmd_avatar::cmd_avatar(Client *client, Client::interaction_t *interaction)
{
    Client::member_t member;
    if (interaction->options.size() > 0) {
        member = client->getMember(interaction->options["user"], interaction->guild_id);
    } else {
        member = interaction->member;
    }
    Client::embed_t embed;

    embed.title = QString("%1'%2 avatar").arg(member.nick.isEmpty() ? member.user.username : member.nick, (member.nick.isEmpty() ? member.user.username : member.nick).endsWith("s") ? "" : "s");
    embed.colour = -1;

    if (member.user.avatar.isNull()) {
        embed.image_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(member.user.discriminator.toInt() % 5));
    } else {
        embed.image_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(member.user.id, member.user.avatar);
    }

    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    client->send_message(interaction->channel_id, "", embed);
}

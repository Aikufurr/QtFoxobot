#include "cmd_hello.h"

cmd_hello::cmd_hello(Client *client, Client::interaction_t *interaction) {
    Client::embed_t embed;

    embed.title = QString("Hello There %1!").arg(interaction->member.nick.isEmpty() ? interaction->member.user.username : interaction->member.nick);
    embed.colour = -1;

    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    client->send_message(interaction->channel_id, "", embed);
}

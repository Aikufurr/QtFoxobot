#include "cmd_avatar.h"

cmd_avatar::cmd_avatar(Client *client, Client::interaction_t *interaction) {
    Client::member_t member;
    if (interaction->options.size() > 0) {
        member = client->member_get(interaction->options["user"], interaction->guild_id);
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

    client->webhook_edit_message(interaction->token, "", embed);
    emit quit();
}

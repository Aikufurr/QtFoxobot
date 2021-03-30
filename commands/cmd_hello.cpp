#include "cmd_hello.h"

cmd_hello::cmd_hello(Client *client, Client::interaction_t *interaction) {
    Client::embed_t embed;

    embed.title = QString("Hello There %1!").arg(interaction->member.nick.isEmpty() ? interaction->member.user.username : interaction->member.nick);
    embed.colour = -1;

    client->webhook_edit_message(interaction->token, "", embed);
//    client->custom_request();
    emit quit();
}

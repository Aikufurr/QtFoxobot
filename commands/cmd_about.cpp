#include "cmd_about.h"

cmd_about::cmd_about(Client *client, Client::interaction_t *interaction) {
    Client::user_t owner = client->user_get("308681202548604938");
    Client::embed_t embed;

    embed.title = QString("About %1").arg(client->me_get().username);
    embed.colour = -1;

    {
        Client::embed_field_t field;
        field.name = "Image Credits";
        field.value = QString("Krabii");
        field.is_inline = true;
        embed.fields.push_back(field);
    }
    {
        Client::embed_field_t field;
        field.name = "Creator";
        field.value = QString("[%1#%2](https://twitter.com/Aikufurr)").arg(owner.username, owner.discriminator);
        field.is_inline = true;
        embed.fields.push_back(field);
    }
    {
        Client::embed_field_t field;
        field.name = "Quality Assurance";
        field.value = QString("[BaconSandwich#3547](https://twitter.com/BaconSandwichMI)");
        field.is_inline = true;
        embed.fields.push_back(field);
    }

    if (client->me_get().avatar.isNull()) {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(client->me_get().discriminator.toInt() % 5));
    } else {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(client->me_get().id, client->me_get().avatar);
    }

    client->webhook_edit_message(interaction->token, "", embed);
    emit quit();
}

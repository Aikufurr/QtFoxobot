#include "cmd_about.h"

cmd_about::cmd_about(Client *client, Client::interaction_t *interaction)
{
    Client::user_t owner = client->getUser("308681202548604938");
    Client::embed_t embed;

    embed.title = QString("About %1").arg(client->getMe().username);
    embed.colour = -1;
    QString description(R"**(%1 originated as a successor for one of my older bots called FluffsterBot and was created on the 4th of September 2019 by %2#%3. Foxobot's original purpose was to provide the user with pictures of foxes (hence the name) with more bot-oriented features following shortly after.
%1 survived multiple language changes with the latest being written in a C++ custom wrapper I wrote. Foxobot was built upon their community and was maintained through their support, with features and issues coming from the userbase providing the best UX that I could provide.)**");

    embed.description = description.arg(client->getMe().username, owner.username, owner.discriminator);

    {
        Client::embed_field_t field;
        field.name = "Image Credits";
        field.value = QString("[Mojang Studios](https://minecraft.gamepedia.com/Fox)");
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

    if (client->getMe().avatar.isNull()) {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(client->getMe().discriminator.toInt() % 5));
    } else {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(client->getMe().id, client->getMe().avatar);
    }

    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    client->send_message(interaction->channel_id, "", embed);
}

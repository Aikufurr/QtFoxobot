#include "cmd_settings.h"

cmd_settings::cmd_settings(Client *client, Client::interaction_t *interaction, DbManager *dbManager) {
    if ((interaction->member.permissions.toUInt() & 0x8) == 0) {
        client->send_message(interaction->channel_id, QString("<@%1>, Sorry, you need to be an administrator to use this command.").arg(interaction->member.user.id));
        return;
    }

    if (interaction->sub_group == "logging") {
        if (interaction->sub_option == "bind") {
            QString channel_id = "";
            if (interaction->sub_options.size() != 0) {
                channel_id = interaction->sub_options["channel"];
            }

            QString old_channel_id = dbManager->setting_get(interaction->guild_id, "logging", "bind");

            // If no previous and unbounded
            if (old_channel_id == "" && channel_id == "") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently bound to any channel.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "bind", channel_id)) {
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Bind");

                QString description = QString("Successfully %1bound").arg(channel_id == "" ? "un" : "");

                // If has previous and unbounded
                if (old_channel_id != "" && channel_id == "") {
                    description += QString(" from <#%1>").arg(old_channel_id);
                } else if (channel_id != "") {
                    // If there is no previous and bounded
                    description += QString(" to <#%1>").arg(channel_id);

                    if (old_channel_id != "" && channel_id != "") {
                        // If there is previous and bounded
                        description += QString(" replacing <#%1>").arg(old_channel_id);
                    }
                }

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "message_update") {
            QString enabled = interaction->sub_options["enabled"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "message_update");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "message_update", enabled)) {
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Message Update");

                QString description = QString("Successfully %1abled").arg(enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "message_delete") {
            QString enabled = interaction->sub_options["enabled"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "message_delete");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "message_delete", enabled)) {
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Message Delete");

                QString description = QString("Successfully %1abled").arg(enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "message_purge") {
            QString enabled = interaction->sub_options["enabled"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "message_purge");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "message_purge", enabled)) {
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Message Purge");

                QString description = QString("Successfully %1abled").arg(enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "member_add") {
            QString enabled = interaction->sub_options["enabled"];
            QString bots_enabled = interaction->sub_options["bots"].isEmpty() ? "0" : interaction->sub_options["bots"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "member_add");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "member_add", enabled)) {
                dbManager->setting_set(interaction->guild_id, "logging", "member_add-bots", bots_enabled);
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Member Add");

                QString description = QString("Successfully %1abled\nBots %2abled").arg(enabled == "0" ? "dis" : "en",
                                                                                       bots_enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "member_update") {
            QString enabled = interaction->sub_options["enabled"];
            QString bots_enabled = interaction->sub_options["bots"].isEmpty() ? "0" : interaction->sub_options["bots"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "member_update");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "member_update", enabled)) {
                dbManager->setting_set(interaction->guild_id, "logging", "member_update-bots", bots_enabled);
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Member Update");

                QString description = QString("Successfully %1abled\nBots %2abled").arg(enabled == "0" ? "dis" : "en",
                                                                                       bots_enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "member_remove") {
            QString enabled = interaction->sub_options["enabled"];
            QString bots_enabled = interaction->sub_options["bots"].isEmpty() ? "0" : interaction->sub_options["bots"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "member_remove");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "member_remove", enabled)) {
                dbManager->setting_set(interaction->guild_id, "logging", "member_remove-bots", bots_enabled);
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Member Remove");

                QString description = QString("Successfully %1abled\nBots %2abled").arg(enabled == "0" ? "dis" : "en",
                                                                                       bots_enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        } else if (interaction->sub_option == "interaction") {
            QString enabled = interaction->sub_options["enabled"];
            QString old_state = dbManager->setting_get(interaction->guild_id, "logging", "interaction");

            if (old_state == "0" && enabled == "0") {
                client->send_message(interaction->channel_id, QString("<@%1>, Not currently enabled.").arg(interaction->member.user.id));
            } else if (dbManager->setting_set(interaction->guild_id, "logging", "interaction", enabled)) {
                Client::embed_t embed;

                embed.title = QString("Settings - Logging - Interaction");

                QString description = QString("Successfully %1abled").arg(enabled == "0" ? "dis" : "en");

                embed.description = description;
                embed.colour = 16757760; // FFB400 - Orange

                embed.author.name = interaction->member.user.username;
                if (interaction->member.user.avatar.isNull()) {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
                } else {
                    embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
                }
                client->send_message(interaction->channel_id, "", embed); // TODO - Send embed in selected channel showing what events will be logged (if not in that channel already)
            } else {
                client->send_message(interaction->channel_id, QString("<@%1>, Sorry, there was an error is preforming this action. Please try again later or contact the developer.").arg(interaction->member.user.id));
            }
        }
    }
}

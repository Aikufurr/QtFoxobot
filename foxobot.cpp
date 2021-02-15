#include "foxobot.h"
#include <QDebug>

// Commands
#include "commands/cmd_about.h"
#include "commands/cmd_aesthetics.h"
#include "commands/cmd_avatar.h"
#include "commands/cmd_coinflip.h"
#include "commands/cmd_eight_ball.h"
#include "commands/cmd_hello.h"
#include "commands/cmd_info.h"
#include "commands/cmd_leaderboard.h"
#include "commands/cmd_rank.h"
#include "commands/cmd_settings.h"
#include "commands/cmd_mod.h"

foxobot::foxobot() {
    dbManager = new DbManager("./database.db");

    token = dbManager->config_get("token");
    application_id = dbManager->config_get("application_id");
    dev_application_id = dbManager->config_get("dev_application_id");
    dev_guild_id = dbManager->config_get("dev_guild_id");

    if (token.isEmpty() || application_id.isEmpty()) {
        qDebug() << "No Token and or application_id present";
        return;
    }
    client = new Client(application_id, dbManager);

    client->login(token);

    //    client->getGateway();

    connect(client, SIGNAL(ready(QString)), this, SLOT(ready(QString)));
    connect(client, SIGNAL(message_create(Client::message_t *)), this, SLOT(message_create(Client::message_t *)));
    connect(client, SIGNAL(interaction_create(Client::interaction_t *)), this, SLOT(interaction_create(Client::interaction_t *)));
}


void foxobot::ready(QString name) {
    qDebug() << "Logged in as" << name;
    this->create_slash_commands();
}

void foxobot::message_create(Client::message_t *message) {
    qDebug() << "Received message from" << message->author.username << ":" << message->content;
    if (message->author.bot) {
        return;
    }
    dbManager->rank_increment(message->author.id, message->guild_id);
}

void foxobot::interaction_create(Client::interaction_t *interaction) {
    qDebug() << "Interaction message from" << interaction->member.user.username << ":" << interaction->command;

    if (interaction->command == "about") {
        cmd_about *cmd = new cmd_about(client, interaction);
        delete cmd;
    } else if (interaction->command == "aesthetics") {
        cmd_aesthetics *cmd = new cmd_aesthetics(client, interaction);
        delete cmd;
    } else if (interaction->command == "avatar") {
        cmd_avatar *cmd = new cmd_avatar(client, interaction);
        delete cmd;
    } else if (interaction->command == "coinflip") {
        cmd_coinflip *cmd = new cmd_coinflip(client, interaction);
        delete cmd;
    } else if (interaction->command == "8ball") {
        cmd_eight_ball *cmd = new cmd_eight_ball(client, interaction);
        delete cmd;
    } else if (interaction->command == "hello") {
        cmd_hello *cmd = new cmd_hello(client, interaction);
        delete cmd;
    } else if (interaction->command == "info") {
        cmd_info *cmd = new cmd_info(client, interaction);
        delete cmd;
    } else if (interaction->command == "leaderboard") {
        cmd_leaderboard *cmd = new cmd_leaderboard(client, interaction, dbManager);
        delete cmd;
    } else if (interaction->command == "rank") {
        cmd_rank *cmd = new cmd_rank(client, interaction, dbManager);
        delete cmd;
    } else if (interaction->command == "settings") {
        cmd_settings *cmd = new cmd_settings(client, interaction, dbManager);
        delete cmd;
    } else if (interaction->command == "mod") {
        cmd_mod *cmd = new cmd_mod(client, interaction);
        delete cmd;
    } else if (interaction->command == "close") {
        client->send_message(interaction->channel_id, "Goodbye.");
        delete dbManager;
        delete client;
        QCoreApplication::quit();
    }
}

void foxobot::create_slash_commands() {
    qDebug() << "Creating slash commands globally?" << (application_id != dev_application_id);

    {
        QJsonObject command;
        command.insert("name", "close");
        command.insert("description", "Stop the bot");
        client->create_slash_command(command, dev_guild_id);
    }

    {
        QJsonObject command;
        command.insert("name", "about");
        command.insert("description", QString("about %1").arg(client->getMe().username));
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "aesthetics");
        command.insert("description", "Takes your text and makes it 𝒶𝑒𝓈𝓉𝒽𝑒𝓉𝒾𝒸");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "text");
            option.insert("description", "Enter your text to make 𝒶𝑒𝓈𝓉𝒽𝑒𝓉𝒾𝒸");
            option.insert("required", true);
            option.insert("type", applicationCommandOptionType::STRING);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "avatar");
        command.insert("description", "Displays your avatar");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "user");
            option.insert("description", "View the avatar of a user");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::USER);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "coinflip");
        command.insert("description", "Flip a coin");
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "8ball");
        command.insert("description", "Enter your question and let the magical 8ball decide");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "question");
            option.insert("description", "Enter your question");
            option.insert("required", true);
            option.insert("type", applicationCommandOptionType::STRING);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "hello");
        command.insert("description", QString("Say hello to %1").arg(client->getMe().username));
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "info");
        command.insert("description", "Displays basic user information");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "user");
            option.insert("description", "Display for another user");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::USER);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "leaderboard");
        command.insert("description", "View the server's top 10 leaderboard");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "amount");
            option.insert("description", "Choose how many results to show");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::INTEGER);
            QJsonArray choices;
            {
                QJsonObject choice;
                choice.insert("name", "5");
                choice.insert("value", 5);
                choices.push_back(choice);
            }
            {
                QJsonObject choice;
                choice.insert("name", "10");
                choice.insert("value", 10);
                choices.push_back(choice);
            }
            {
                QJsonObject choice;
                choice.insert("name", "15");
                choice.insert("value", 15);
                choices.push_back(choice);
            }
            {
                QJsonObject choice;
                choice.insert("name", "20");
                choice.insert("value", 20);
                choices.push_back(choice);
            }
            option.insert("choices", choices);
            options.push_back(option);
        }
        {
            QJsonObject option;
            option.insert("name", "custom");
            option.insert("description", "Choose yourself how many results to show (1-20)");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::INTEGER);
            options.push_back(option);
        }
        {
            QJsonObject option;
            option.insert("name", "chart");
            option.insert("description", "Display the leaderboard as a chart");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::BOOLEAN);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "rank");
        command.insert("description", "View your rank on the server");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "user");
            option.insert("description", "View the rank of a user");
            option.insert("required", false);
            option.insert("type", applicationCommandOptionType::USER);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "settings");
        command.insert("description", "Adjust the settings for the bot (Admin only)");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "logging");
            option.insert("description", "Adjust the logging settings");
            option.insert("type", applicationCommandOptionType::SUB_COMMAND_GROUP);
            QJsonArray sub_options;
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "bind");
                sub_cmd_options.insert("description", "Binds the logging to a channel. If left blank it will unbind");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "channel");
                    sub_cmd_option.insert("description", "The channel to bind to");
                    sub_cmd_option.insert("required", false);
                    sub_cmd_option.insert("type", applicationCommandOptionType::CHANNEL);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "message_update");
                sub_cmd_options.insert("description", "If a message contents have been edited");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "message_delete");
                sub_cmd_options.insert("description", "If a message has been deleted");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "message_purge");
                sub_cmd_options.insert("description", "When a set of messages are purged");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "member_add");
                sub_cmd_options.insert("description", "If a member has joined the server and display their info");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "bots");
                    sub_cmd_option.insert("description", "Should bots be counted");
                    sub_cmd_option.insert("required", false);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "member_update");
                sub_cmd_options.insert("description", "If a member has been updated on the server");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "bots");
                    sub_cmd_option.insert("description", "Should bots be counted");
                    sub_cmd_option.insert("required", false);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "member_remove");
                sub_cmd_options.insert("description", "If a member has left the server and display their info");
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "bots");
                    sub_cmd_option.insert("description", "Should bots be counted");
                    sub_cmd_option.insert("required", false);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }
            {
                QJsonObject sub_cmd_options;
                sub_cmd_options.insert("name", "interaction");
                sub_cmd_options.insert("description", QString("If a member activated one of %1'%2 commands").arg(client->getMe().username, client->getMe().username.endsWith("s") ? "" : "s"));
                sub_cmd_options.insert("type", applicationCommandOptionType::SUB_COMMAND);
                QJsonArray sub_cmd_options_options;
                {
                    QJsonObject sub_cmd_option;
                    sub_cmd_option.insert("name", "enabled");
                    sub_cmd_option.insert("description", "Enable or disable this log type");
                    sub_cmd_option.insert("required", true);
                    sub_cmd_option.insert("type", applicationCommandOptionType::BOOLEAN);
                    sub_cmd_options_options.push_back(sub_cmd_option);
                }
                sub_cmd_options.insert("options", sub_cmd_options_options);
                sub_options.push_back(sub_cmd_options);
            }

            option.insert("options", sub_options);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
    {
        QJsonObject command;
        command.insert("name", "mod");
        command.insert("description", "Moderation commands (Admin only)");
        QJsonArray options;
        {
            QJsonObject option;
            option.insert("name", "purge");
            option.insert("description", "Bulk delete a certain amount of messages (2-100)");
            option.insert("type", applicationCommandOptionType::SUB_COMMAND);
            QJsonArray sub_options;
            {
                QJsonObject sub_option;
                sub_option.insert("name", "amount");
                sub_option.insert("description", "How many messages to delete (2-100)");
                sub_option.insert("required", true);
                sub_option.insert("type", applicationCommandOptionType::INTEGER);
                sub_options.push_back(sub_option);
            }
            option.insert("options", sub_options);
            options.push_back(option);
        }
        command.insert("options", options);
        client->create_slash_command(command, application_id == dev_application_id ? dev_guild_id : "");
    }
}

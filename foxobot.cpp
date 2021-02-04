#include "foxobot.h"
#include <QDebug>

// Commands
#include "commands/cmd_aesthetics.h"
#include "commands/cmd_eight_ball.h"
#include "commands/cmd_hello.h"
#include "commands/cmd_leaderboard.h"
#include "commands/cmd_rank.h"

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
    client = new Client(application_id);

    client->login(token);
    this->create_slash_commands();
    //    client->getGateway();

    connect(client, SIGNAL(ready(QString)), this, SLOT(ready(QString)));
    connect(client, SIGNAL(message_create(Client::message_t)), this, SLOT(message_create(Client::message_t)));
    connect(client, SIGNAL(interaction_create(Client::interaction_t *)), this, SLOT(interaction_create(Client::interaction_t *)));
}

void foxobot::ready(QString name) {
    qDebug() << "Logged in as" << name;
}

void foxobot::message_create(Client::message_t message) {
    qDebug() << "Received message from" << message.author.username << ":" << message.content;
    if (message.author.bot) {
        return;
    }
    dbManager->rank_increment(message.author.id, message.guild_id);
}

void foxobot::interaction_create(Client::interaction_t *interaction) {
    qDebug() << "Interaction message from" << interaction->member.user.username << ":" << interaction->command;

    if (interaction->command == "aesthetics") {
        new cmd_aesthetics(client, interaction);
    } else if (interaction->command == "8ball") {
        new cmd_eight_ball(client, interaction);
    } else if (interaction->command == "hello") {
        new cmd_hello(client, interaction);
    } else if (interaction->command == "leaderboard") {
        new cmd_leaderboard(client, interaction, dbManager);
    } else if (interaction->command == "rank") {
        new cmd_rank(client, interaction, dbManager);
    }
}

void foxobot::create_slash_commands() {
    qDebug() << "Creating slash commands globally?" << (application_id != dev_application_id);

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
        command.insert("description", "Say hello to the bot");
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
}

#include "cmd_eight_ball.h"

cmd_eight_ball::cmd_eight_ball(Client *client, Client::interaction_t *interaction) {
    int seed = 0;
    QList<QString> words = interaction->options["question"].toLower().split("", QString::SkipEmptyParts);
    qDebug() << words << interaction->options["question"].toLower();
    foreach(const QString &word, words) {
        seed += word.at(0).toLatin1();
    }

    srand(seed);

    long choice = rand() % outcomes.size();
    QString outcome = outcomes.at(choice);
    qDebug() << choice << seed << outcome;
    client->webhook_edit_message(interaction->token, QString("<@%1>, \"%2\" - %3").arg(interaction->member.user.id, interaction->options["question"], outcome));
    emit quit();
}

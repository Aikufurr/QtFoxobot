#include "cmd_eight_ball.h"

cmd_eight_ball::cmd_eight_ball(Client *client, Client::interaction_t *interaction) {
    int seed = 0;
    QList<QString> words = interaction->options["question"].toLower().split("", QString::SkipEmptyParts);
    qDebug() << words << interaction->options["question"].toLower();
    foreach(const QString &word, words) {
        seed += word.at(0).toLatin1();
    }

    srand(seed);

    long choice = rand() % 12;
    QString outcome = outcomes.at(choice);
    qDebug() << choice << seed << outcome;
    client->send_message(interaction->channel_id, QString("<@%1>, %2").arg(interaction->member.user.id, outcome));
}

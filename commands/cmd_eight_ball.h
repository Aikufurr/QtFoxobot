#ifndef CMD_EIGHT_BALL_H
#define CMD_EIGHT_BALL_H

#include "client.h"
#include <QList>
#include <QObject>

class cmd_eight_ball : public QObject {
    Q_OBJECT

public:
    cmd_eight_ball(Client *client, Client::interaction_t *interaction);

private:
    QList<QString> outcomes = {
        "It is certain",
        "It is decidedly so",
        "Without a doubt",
        "Yes - definitely",
        "You may rely on it",
        "As I see it, yes",
        "Most likely",
        "Probably Not",
        "No",
        "Signs point to yes",
        "Don't count on it",
        "My reply is no",
        "Outlook not so good",
        "Very doubtful"
    };

signals:
    void quit();
};

#endif // CMD_EIGHT_BALL_H

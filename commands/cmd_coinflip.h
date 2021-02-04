#ifndef CMD_COINFLIP_H
#define CMD_COINFLIP_H

#include <QObject>
#include "client.h"
#include <QList>

class cmd_coinflip : public QObject
{
    Q_OBJECT
public:
    cmd_coinflip(Client *client, Client::interaction_t *interaction);

private:
    QList<QString> outcomes = {"Heads", "Heads", "Heads", "Heads", "Heads", "Heads",
                               "Tails", "Tails","Tails", "Tails", "Tails", "Tails",
                               "it's side"};
};

#endif // CMD_COINFLIP_H

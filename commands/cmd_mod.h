#ifndef CMD_MOD_H
#define CMD_MOD_H

#include <QObject>
#include "client.h"

class cmd_mod : public QObject
{
    Q_OBJECT
public:
    cmd_mod(Client *client, Client::interaction_t *interaction);
};

#endif // CMD_MOD_H

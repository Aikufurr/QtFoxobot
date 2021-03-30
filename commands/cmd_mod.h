#ifndef CMD_MOD_H
#define CMD_MOD_H

#include "client.h"
#include <QObject>

class cmd_mod : public QObject {
    Q_OBJECT

public:
    cmd_mod(Client *client, Client::interaction_t *interaction);

signals:
    void quit();
};

#endif // CMD_MOD_H

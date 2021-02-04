#ifndef CMD_HELLO_H
#define CMD_HELLO_H

#include <QObject>
#include "client.h"

class cmd_hello : public QObject {
    Q_OBJECT
public:
    cmd_hello(Client *client, Client::interaction_t *interaction);
};

#endif // CMD_HELLO_H

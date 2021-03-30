#ifndef CMD_HELLO_H
#define CMD_HELLO_H

#include "client.h"
#include <QObject>

class cmd_hello : public QObject {
    Q_OBJECT

public:
    cmd_hello(Client *client, Client::interaction_t *interaction);

signals:
    void quit();
};

#endif // CMD_HELLO_H

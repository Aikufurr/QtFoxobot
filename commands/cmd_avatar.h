#ifndef CMD_AVATAR_H
#define CMD_AVATAR_H

#include "client.h"
#include <QObject>

class cmd_avatar : public QObject {
    Q_OBJECT

public:
    cmd_avatar(Client *client, Client::interaction_t *interaction);

signals:
    void quit();
};

#endif // CMD_AVATAR_H

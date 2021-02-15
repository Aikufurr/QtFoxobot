#ifndef CMD_AVATAR_H
#define CMD_AVATAR_H

#include <QObject>
#include "client.h"

class cmd_avatar : public QObject
{
    Q_OBJECT
public:
    cmd_avatar(Client *client, Client::interaction_t *interaction);
};

#endif // CMD_AVATAR_H

#ifndef CMD_ABOUT_H
#define CMD_ABOUT_H

#include <QObject>
#include "client.h"

class cmd_about : public QObject
{
    Q_OBJECT
public:
    cmd_about(Client *client, Client::interaction_t *interaction);
};

#endif // CMD_ABOUT_H

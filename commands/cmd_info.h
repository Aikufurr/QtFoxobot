#ifndef CMD_INFO_H
#define CMD_INFO_H

#include <QObject>
#include "client.h"

class cmd_info : public QObject {
    Q_OBJECT
public:
    cmd_info(Client *client, Client::interaction_t *interaction);
};

#endif // CMD_INFO_H

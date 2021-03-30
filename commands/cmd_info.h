#ifndef CMD_INFO_H
#define CMD_INFO_H

#include "client.h"
#include <QObject>

class cmd_info : public QObject {
    Q_OBJECT

public:
    cmd_info(Client *client, Client::interaction_t *interaction);

signals:
    void quit();
};

#endif // CMD_INFO_H

#ifndef CMD_ABOUT_H
#define CMD_ABOUT_H

#include "client.h"
#include <QObject>

class cmd_about : public QObject {
    Q_OBJECT

public:
    cmd_about(Client *client, Client::interaction_t *interaction);

signals:
    void quit();
};

#endif // CMD_ABOUT_H

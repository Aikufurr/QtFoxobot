#ifndef CMD_SETTINGS_H
#define CMD_SETTINGS_H

#include "client.h"
#include "dbmanager.h"
#include <QObject>

class cmd_settings : public QObject {
    Q_OBJECT

public:
    cmd_settings(Client *client, Client::interaction_t *interaction, DbManager *dbManager);

signals:
    void quit();
};

#endif // CMD_SETTINGS_H

#ifndef CMD_SETTINGS_H
#define CMD_SETTINGS_H

#include <QObject>
#include "client.h"
#include "dbmanager.h"

class cmd_settings : public QObject {
    Q_OBJECT
public:
    cmd_settings(Client *client, Client::interaction_t *interaction, DbManager *dbManager);
};

#endif // CMD_SETTINGS_H

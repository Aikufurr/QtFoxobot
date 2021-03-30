#ifndef CMD_RANK_H
#define CMD_RANK_H

#include "client.h"
#include "dbmanager.h"
#include <QObject>

class cmd_rank : public QObject {
    Q_OBJECT

public:
    cmd_rank(Client *client, Client::interaction_t *interaction, DbManager *dbManager);

signals:
    void quit();
};

#endif // CMD_RANK_H

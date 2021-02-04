#ifndef CMD_RANK_H
#define CMD_RANK_H

#include <QObject>
#include "client.h"
#include "dbmanager.h"

class cmd_rank : public QObject {
    Q_OBJECT
public:
    cmd_rank(Client *client, Client::interaction_t *interaction, DbManager *dbManager);
};

#endif // CMD_RANK_H

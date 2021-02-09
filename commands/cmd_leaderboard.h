#ifndef CMD_LEADERBOARD_H
#define CMD_LEADERBOARD_H

#include <QObject>
#include "client.h"
#include "dbmanager.h"

#include <QPixmap>
#include <QtCharts>
#include <QDateTimeAxis>

class cmd_leaderboard : public QObject {
    Q_OBJECT
public:
    cmd_leaderboard(Client *client, Client::interaction_t *interaction, DbManager *dbManager);
};

#endif // CMD_LEADERBOARD_H

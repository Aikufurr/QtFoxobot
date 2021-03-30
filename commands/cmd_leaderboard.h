#ifndef CMD_LEADERBOARD_H
#define CMD_LEADERBOARD_H

#include "client.h"
#include "dbmanager.h"
#include <QDateTimeAxis>
#include <QObject>
#include <QPixmap>
#include <QtCharts>

class cmd_leaderboard : public QObject {
    Q_OBJECT

public:
    cmd_leaderboard(Client *client, Client::interaction_t *interaction, DbManager *dbManager);

signals:
    void quit();
};

#endif // CMD_LEADERBOARD_H

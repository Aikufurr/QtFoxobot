#ifndef CMD_TICTACTOE_H
#define CMD_TICTACTOE_H

#include "client.h"
#include <QObject>
#include <QtMath>
#include <limits>

class cmd_tictactoe : public QObject {
    Q_OBJECT

public:
    cmd_tictactoe(Client *client, Client::interaction_t *interaction);

private:
    Client *client;
    Client::interaction_t interaction;
    Client::message_t msg;
    QTimer *timeout;
    QVector<QStringList> matrix;
    QVector<QStringList> reactions;
    QString player;
    QString ai;

    bool finished_reactions = false;
    void handle_quit();
    bool equals3(QString a, QString b, QString c);
    QString checkWinner();
    QPair<int, int> nextTurn();
    int minimax(bool maximizing_player, int depth);
    QPair<int, int> bestMove(int depth);

private slots:
    void message_reaction_add(Client::reaction_t reaction);

signals:
    void quit();
};

#endif // CMD_TICTACTOE_H

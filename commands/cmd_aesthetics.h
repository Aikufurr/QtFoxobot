#ifndef CMD_AESTHETICS_H
#define CMD_AESTHETICS_H

#include "client.h"
#include <QHash>
#include <QList>
#include <QObject>

class cmd_aesthetics : public QObject {
    Q_OBJECT

public:
    cmd_aesthetics(Client *client, Client::interaction_t *interaction);

private:
    QHash<QChar, QString> characterMap;

signals:
    void quit();

};

#endif // CMD_AESTHETICS_H

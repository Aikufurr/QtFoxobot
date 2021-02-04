#ifndef CMD_AESTHETICS_H
#define CMD_AESTHETICS_H

#include <QObject>
#include "client.h"
#include <QList>
#include <QHash>

class cmd_aesthetics : public QObject
{
    Q_OBJECT
public:
    cmd_aesthetics(Client *client, Client::interaction_t *interaction);

private:
    QHash<QChar, QString> characterMap;

};

#endif // CMD_AESTHETICS_H

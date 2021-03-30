#ifndef RANKRESET_H
#define RANKRESET_H

#include "dbmanager.h"
#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QTimer>

class RankReset : public QObject {
    Q_OBJECT

public:
    RankReset(DbManager *dbManager);
    void start();

private:
    QTimer timer;
    DbManager *dbManager;

private slots:
    void reset();

};

#endif // RANKRESET_H

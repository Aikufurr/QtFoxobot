#include "rankreset.h"

RankReset::RankReset(DbManager *dbManager) {
    this->dbManager = dbManager;
    QObject::connect(&timer, &QTimer::timeout, this, &RankReset::reset);
    timer.setSingleShot(false);
}

void RankReset::start() {
    QDateTime now = QDateTime::currentDateTimeUtc();
    QDateTime tomorrow = QDateTime::currentDateTimeUtc().addMonths(1);
    tomorrow.setDate(QDate(tomorrow.date().year(), tomorrow.date().month(), 1));
    tomorrow.setTime(QTime(0, 0, 0, 0));

    qDebug() << "Interval set to" << now.msecsTo(tomorrow) << "ms from now [" << now.toMSecsSinceEpoch() << "]. Timeout at " << tomorrow.toMSecsSinceEpoch() << "ms";
    timer.start(now.msecsTo(tomorrow));
}

void RankReset::reset() {
    qDebug() << "Resetting Rank";

    dbManager->rank_reset();

    qDebug() << "Restarting Timer...";
    this->start();
}

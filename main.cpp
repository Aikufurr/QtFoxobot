#include <QApplication>
#include "foxobot.h"

// TODO
/*
 * Monthly (customisable via settings?) guild rank purge
 * pic
 * avatar
 * about (credits)
 */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    new foxobot();

    return a.exec();
}

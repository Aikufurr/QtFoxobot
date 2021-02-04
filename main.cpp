#include <QCoreApplication>
#include "foxobot.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    new foxobot();

    return a.exec();
}

#include <QApplication>
#include "foxobot.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    new foxobot();

    return a.exec();
}

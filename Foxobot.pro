QT += core websockets sql charts network
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        client.cpp \
        commands/cmd_about.cpp \
        commands/cmd_aesthetics.cpp \
        commands/cmd_avatar.cpp \
        commands/cmd_coinflip.cpp \
        commands/cmd_eight_ball.cpp \
        commands/cmd_hello.cpp \
        commands/cmd_info.cpp \
        commands/cmd_leaderboard.cpp \
        commands/cmd_mod.cpp \
        commands/cmd_rank.cpp \
        commands/cmd_settings.cpp \
        commands/cmd_tictactoe.cpp \
        dbmanager.cpp \
        foxobot.cpp \
        http/apirequest.cpp \
        http/asyncqueue.cpp \
        http/requesthandler.cpp \
        http/restmanager.cpp \
        main.cpp \
        util/rankreset.cpp \
        util/util.cpp \
        websocket.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    client.h \
    commands/cmd_about.h \
    commands/cmd_aesthetics.h \
    commands/cmd_avatar.h \
    commands/cmd_coinflip.h \
    commands/cmd_eight_ball.h \
    commands/cmd_hello.h \
    commands/cmd_info.h \
    commands/cmd_leaderboard.h \
    commands/cmd_mod.h \
    commands/cmd_rank.h \
    commands/cmd_settings.h \
    commands/cmd_tictactoe.h \
    dbmanager.h \
    foxobot.h \
    http/apirequest.h \
    http/asyncqueue.h \
    http/requesthandler.h \
    http/restmanager.h \
    util/rankreset.h \
    util/util.h \
    websocket.h

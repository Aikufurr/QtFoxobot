#ifndef CLIENT_H
#define CLIENT_H

#include "dbmanager.h"
#include "http/restmanager.h"
#include "util/util.h"
#include "websocket.h"
#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkReply>
#include <QObject>
#include <QRegularExpression>
#include <QUuid>
#include <QtMath>
#include <QtNetwork/QNetworkAccessManager>

namespace applicationCommandOptionType {
enum ApplicationCommandOptionType {
    SUB_COMMAND         =   1,
    SUB_COMMAND_GROUP   =   2,
    STRING              =   3,
    INTEGER             =   4,
    BOOLEAN             =   5,
    USER                =   6,
    CHANNEL             =   7,
    ROLE                =   8
};
}

namespace channelTypes {
enum ChannelTypes {
    GUILD_TEXT      =   0,
    DM              =   1,
    GUILD_VOICE     =   2,
    GROUP_DM        =   3,
    GUILD_CATEGORY  =   4,
    GUILD_NEWS      =   5,
    GUILD_STORE     =   6
};
}

class Client : public QObject {
    Q_OBJECT

public:
    Client(QString application_id, DbManager *dbmanager);

    // Start of structures
    struct attachment_t {
        QString id;
        QString filename;
        int size;
        QString url;
        int height;
        int width;
    };
    struct permission_overwrite_t {
        QString id;
        QString type;
        QString allow;
        QString deny;
    };
    struct channel_t {
        QString id;
        int type; // channelTypes
        QString guild_id;
        int position;
        QList<permission_overwrite_t> permission_overwrites;
        QString name;
        QString topic;
        bool nsfw;
        QString last_message_id;
        int bitrate;
        int user_limit;
        QString parent_id;
    };
    struct embed_author_t {
        QString name;
        QString icon_url;
    };
    struct embed_field_t {
        QString name;
        QString value;
        bool is_inline;
    };
    struct embed_footer_t {
        QString text;
        QString icon_url;
    };
    struct embed_t {
        QString title;
        QString description;
        QString url;
        QDateTime timestamp;
        int colour;
        embed_author_t author;
        embed_footer_t footer;
        QString image_url;
        QString thumbnail_url;
        QList<embed_field_t> fields;
    };
    struct roles_t {
        QString id;
        QString name;
        int colour;
        int position;
        QString permissions;
        bool managed;
        bool mentionable;
    };
    struct user_t {
        QString avatar;
        bool bot;
        QString discriminator;
        int flags;
        QString id;
        bool mfa_enabled;
        QString username;
        bool verified;
    };
    struct emoji_t {
        QString id;
        QString name;
        QList<roles_t> role;
        user_t user;
        bool require_colons;
        bool managed;
        bool animated;
        bool available;
    };
    struct member_t {
        user_t user;
        QString nick;
        QList<roles_t> roles;
        QDateTime joined_at;
        QDateTime premium_since;
        QString permissions;
    };
    struct interaction_t {
        QString id;         // Interaction ID
        int type;           // Type of interaction 1 - ping | 2 - ApplicationCommand (eg /rank)
        QString guild_id;   // Guild of interaction
        QString channel_id; // Channel of interaction
        member_t member;    // Guild Member of invoker
        QString token;      // Continuation token

        // Data
        QString command;    // data->name           // rank                 mod                     settings
        QHash<QString, QString> options;            // ["user"] => 1234     unused                  unused
        QString sub_group;                          // unused               purge                   logging
        QString sub_option;                         // unused               unused                  bind
        QHash<QString, QString> sub_options;        // unused               ["amount"] => 2-100     ["channel"] => 1234 or .size() == 0 for nothing
    };
    struct guild_t {
        QString id;
        QString name;
        QString icon;
        QString splash;
        QString owner_id;
        QString region;
        int verification_level;
        int explicit_content_filter;
        QList<roles_t> roles;       // STRUCT
        int mfa_level;
        QDateTime joined_at;
        bool large;
        int member_count;
        int approximate_member_count;
        QHash<QString, member_t> members;    // STRUCT
        QHash<QString, channel_t> channels;  // STRUCT
        int max_members;
        QString description;
    };
    struct mention_t {
        user_t user;
    };
    struct reaction_t {
        int count;
        bool me;
        QString message_id;
        user_t user;
        emoji_t emoji;
    };
    struct message_t {
        QString id;
        QString channel_id;
        QString guild_id;
        user_t author;
        member_t member;
        QString content;
        QDateTime timestamp;
        QDateTime edited_time;
        bool tts;
        bool mention_everyone;
        QList<user_t> mentions;
        QList<roles_t> mention_roles;
        QList<attachment_t> attachments;
        QList<embed_t> embeds;
        QList<reaction_t> reactions;
        QString nonce;
        bool pinned;
        QString webhook_id;
        int type;
    };
    // End of structures


    // Start of variables
    QString token;
    QString application_id;
    DbManager *dbmanager;
    // End of variables


    // Start of functions
    void channel_bulk_delete(QString channel_id, QHash<QString, Client::message_t> message_ids, interaction_t *interaction);
    QHash<QString, message_t> channel_messages(QString channel_id);
    QHash<QString, message_t> channel_messages(QString channel_id, int limit);

    int days_in_month(QDate date);

    QJsonObject embed_to_json(embed_t embed);

    QString get_age(QDate then, QDate now);
    QString get_time(QDateTime then, QDateTime now);

    guild_t guild_get(QString guild_id);

    void login(QString token);

    void me_change_presence(QString name, int type, QString status);
    void me_gateway_get();
    user_t me_get();
    void me_slash_command_create(QJsonObject command, QString guild_id);
    void me_slash_command_delete(QString command_id, QString guild_id);

    member_t member_get(QString user_id, QString guild_id);

    void message_delete(QString channel_id, QString message_id);
    message_t message_edit(QString channel_id, QString message_id, QString content, embed_t embed);
    void message_reaction_create(QString channel_id, QString message_id, QString emoji);
    void message_reaction_delete(QString channel_id, QString message_id, QString emoji);
    void message_reaction_delete(QString channel_id, QString message_id, QString emoji, QString user_id);
    void message_reaction_delete_all(QString channel_id, QString message_id);

    message_t send_file_message(QString channel_id, QByteArray file);
    message_t send_file_message(QString channel_id, QByteArray file, embed_t embed);
    message_t send_message(QString channel_id, QString content);
    message_t send_message(QString channel_id, QString content, embed_t embed);

    void typing(QString channel_id);

    user_t user_get(QString user_id);

    message_t webhook_edit_message(QString token, QString content);
    message_t webhook_edit_message(QString token, QString content, QString id);
    message_t webhook_edit_message(QString token, QString content, embed_t embed);
    message_t webhook_edit_message(QString token, QString content, embed_t embed, QString id);
    message_t webhook_followup_message(QString token, QString content);
    message_t webhook_followup_message(QString token, QString content, embed_t embed);
    message_t webhook_followup_message(QString token, QString content, embed_t embed, QByteArray _file);
    message_t webhook_followup_message(QString token, QString content, QByteArray _file);


    QByteArray custom_request();
    // End of functions



private:
    Websocket *websocket;
    QHash<QString, guild_t> guilds;
    QHash<QString, user_t> users;
    QHash<QString, message_t> messages;
    void guild_create(QJsonObject json_guild);
    Client::message_t message_create(QJsonObject json_message);
    user_t me;
    RESTManager *rest;

private slots:
    void READY(QJsonObject response);

    void INTERACTION_CREATE(QJsonObject interaction);

    void GUILD_CREATE(QJsonObject guild);
    void GUILD_UPDATE(QJsonObject guild);

    void GUILD_MEMBER_ADD(QJsonObject member);
    void GUILD_MEMBER_UPDATE(QJsonObject member);
    void GUILD_MEMBER_REMOVE(QJsonObject member);

    void MESSAGE_CREATE(QJsonObject message);
    void MESSAGE_UPDATE(QJsonObject message);
    void MESSAGE_DELETE(QJsonObject message);
    void MESSAGE_REACTION_ADD(QJsonObject reaction);
    //    void replyFinished(QNetworkReply *);

signals:
    void ready(QString);
    void message(Client::message_t *message);
    void interaction_create(Client::interaction_t *interaction);

    void message_reaction_add(Client::reaction_t reaction);
};

#endif // CLIENT_H

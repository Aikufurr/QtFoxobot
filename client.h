#ifndef CLIENT_H
#define CLIENT_H

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QList>
#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>


#include <QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

#include "websocket.h"

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

class Client : public QObject {
    Q_OBJECT

public:
    Client(QString application_id);
    struct user_t {
        QString avatar;
        bool bot;
        QString discriminator;
        int flags;
        QString id;
        bool mfa_enabled;
        QString username;
        bool verified;
    } user;

    struct embed_field_t {
        QString name;
        QString value;
        bool is_inline;
    };

    struct embed_author_t {
        QString name;
        QString icon_url;
    };

    struct embed_t {
        QString title;
        QString description;
        QString url;
        QDateTime timestamp;
        int colour;
        embed_author_t author;
        QString footer_icon_url;
        QString footer_text;
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
    struct member_t {
        user_t user;
        QString nick;
        QList<roles_t> roles;
        QDateTime joined_at;
        QDateTime premium_since;
        QString permissions;
    };


    struct mention_t {
        user_t user;
    };
    struct attachment_t {
        QString id;
        QString filename;
        int size;
        QString url;
        QString proxy_url;
        int height;
        int width;
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
    struct reaction_t {
        int count;
        bool me;
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
        QList<mention_t> mentions;
        QList<roles_t> mention_roles;
        QList<attachment_t> attachments;
        QList<embed_t> embeds;
        QList<reaction_t> reactions;
        QString nonce;
        bool pinned;
        QString webhook_id;
        int type;
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
        QHash<QString, member_t> members;    // STRUCT
        QList<channel_t> channels;    // STRUCT
        int max_members;
        QString description;
    };

//    struct interaction_option_t {
//        QHash<QString, QString> option;
//    };
    struct interaction_t {
        QString id;         // Interaction ID
        int type;           // Type of interaction 1 - ping | 2 - ApplicationCommand (eg /rank)
        QString guild_id;   // Guild of interaction
        QString channel_id; // Channel of interaction
        member_t member;    // Guild Member of invoker
        QString token;      // Continuation token

        // Data
        QString command;    // data->name
        QHash<QString, QString> options;
    };

    QString token;
    QString application_id;

    void login(QString token);
    void change_presence(QString name, int type, QString status);
    void send_message(QString channel_id, QString content);
    void send_message(QString channel_id, QString content, embed_t embed);
    void create_slash_command(QJsonObject command, QString guild_id);
    guild_t getGuild(QString guild_id);
    user_t getUser(QString user_id);
    member_t getMember(QString user_id, QString guild_id);
    void getGateway();

private:
    Websocket *websocket = new Websocket("");
    QHash<QString, guild_t> guilds;
    QHash<QString, user_t> users;
    QHash<QString, member_t> members;

private slots:
    void READY(QJsonObject response);
    void GUILD_CREATE(QJsonObject guild);
    void MESSAGE_CREATE(QJsonObject message);
    void INTERACTION_CREATE(QJsonObject interaction);
    void GUILD_MEMBER_UPDATE(QJsonObject member);
//    void replyFinished(QNetworkReply *);

signals:
    void ready(QString);
    void message_create(Client::message_t);
    void interaction_create(Client::interaction_t *);
};

#endif // CLIENT_H

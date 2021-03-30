#include "cmd_mod.h"

cmd_mod::cmd_mod(Client *client, Client::interaction_t *interaction) {
    if ((interaction->member.permissions.toLongLong() & 0x8) == 0) {
        client->send_message(interaction->channel_id, QString("<@%1>, Sorry, you need to be an administrator to use this command.").arg(interaction->member.user.id));
        emit quit();
        return;
    }

    if (interaction->sub_group == "purge") {
        int amount = interaction->sub_options["amount"].toInt();
        if (amount < 2 || amount > 98) {
            client->send_message(interaction->channel_id, QString("<@%1>, The allowed range is 2-98, you entered %2.").arg(interaction->member.user.id, interaction->sub_options["amount"]));
            return;
        }

        QHash<QString, Client::message_t> messages_to_delete = client->channel_messages(interaction->channel_id, amount+2);

        client->channel_bulk_delete(interaction->channel_id, messages_to_delete, interaction);
    }

    emit quit();
}

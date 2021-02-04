#include "cmd_coinflip.h"

cmd_coinflip::cmd_coinflip(Client *client, Client::interaction_t *interaction) {
    srand(time(0));
    int index = rand() % outcomes.size();

    client->send_message(interaction->channel_id, QString("<@%1>, The coin landed on %2!").arg(interaction->member.user.id, outcomes.at(index)));
}

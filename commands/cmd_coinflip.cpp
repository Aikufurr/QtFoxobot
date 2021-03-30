#include "cmd_coinflip.h"

cmd_coinflip::cmd_coinflip(Client *client, Client::interaction_t *interaction) {
    srand(time(0));
    int index = rand() % outcomes.size();

    client->webhook_edit_message(interaction->token, QString("<@%1>, The coin landed on %2!").arg(interaction->member.user.id, outcomes.at(index)));
    emit quit();
}

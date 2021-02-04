#include "cmd_aesthetics.h"

cmd_aesthetics::cmd_aesthetics(Client *client, Client::interaction_t *interaction)
{
    characterMap['a'] = "𝒶";
    characterMap['b'] = "𝒷";
    characterMap['c'] = "𝒸";
    characterMap['d'] = "𝒹";
    characterMap['e'] = "𝑒";
    characterMap['f'] = "𝒻";
    characterMap['g'] = "𝑔";
    characterMap['h'] = "𝒽";
    characterMap['i'] = "𝒾";
    characterMap['j'] = "𝒿";
    characterMap['k'] = "𝓀";
    characterMap['l'] = "𝓁";
    characterMap['m'] = "𝓂";
    characterMap['n'] = "𝓃";
    characterMap['o'] = "𝑜";
    characterMap['p'] = "𝓅";
    characterMap['q'] = "𝓆";
    characterMap['r'] = "𝓇";
    characterMap['s'] = "𝓈";
    characterMap['t'] = "𝓉";
    characterMap['u'] = "𝓊";
    characterMap['v'] = "𝓋";
    characterMap['w'] = "𝓌";
    characterMap['x'] = "𝓍";
    characterMap['y'] = "𝓎";
    characterMap['z'] = "𝓏";

    QList<QString> words = interaction->options["text"].split("", QString::SkipEmptyParts);
    QString output = QString("<@%1>, ").arg(interaction->member.user.id);
    foreach(const QString &word, words) {
        output += characterMap[word.at(0)];
    }
    client->send_message(interaction->channel_id, output);
}

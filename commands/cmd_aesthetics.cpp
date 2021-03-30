#include "cmd_aesthetics.h"

cmd_aesthetics::cmd_aesthetics(Client *client, Client::interaction_t *interaction) {
    characterMap['a'] = "𝒶";
    characterMap['A'] = "𝒜";
    characterMap['b'] = "𝒷";
    characterMap['B'] = "ℬ";
    characterMap['c'] = "𝒸";
    characterMap['C'] = "𝒞";
    characterMap['d'] = "𝒹";
    characterMap['D'] = "𝒟";
    characterMap['e'] = "ℯ";
    characterMap['E'] = "ℰ";
    characterMap['f'] = "𝒻";
    characterMap['F'] = "ℱ";
    characterMap['g'] = "ℊ";
    characterMap['G'] = "𝒢";
    characterMap['h'] = "𝒽";
    characterMap['H'] = "ℋ";
    characterMap['i'] = "𝒾";
    characterMap['I'] = "ℐ";
    characterMap['j'] = "𝒿";
    characterMap['J'] = "𝒥";
    characterMap['k'] = "𝓀";
    characterMap['K'] = "𝒦";
    characterMap['l'] = "𝓁";
    characterMap['L'] = "ℒ";
    characterMap['m'] = "𝓂";
    characterMap['M'] = "ℳ";
    characterMap['n'] = "𝓃";
    characterMap['N'] = "𝒩";
    characterMap['o'] = "ℴ";
    characterMap['O'] = "𝒪";
    characterMap['p'] = "𝓅";
    characterMap['P'] = "𝒫";
    characterMap['q'] = "𝓆";
    characterMap['Q'] = "𝒬";
    characterMap['r'] = "𝓇";
    characterMap['R'] = "ℛ";
    characterMap['s'] = "𝓈";
    characterMap['S'] = "𝒮";
    characterMap['t'] = "𝓉";
    characterMap['T'] = "𝒯";
    characterMap['u'] = "𝓊";
    characterMap['U'] = "𝒰";
    characterMap['v'] = "𝓋";
    characterMap['V'] = "𝒱";
    characterMap['w'] = "𝓌";
    characterMap['W'] = "𝒲";
    characterMap['x'] = "𝓍";
    characterMap['X'] = "𝒳";
    characterMap['y'] = "𝓎";
    characterMap['Y'] = "𝒴";
    characterMap['z'] = "𝓏";
    characterMap['Z'] = "𝒵";
    characterMap[' '] = " ";

    QList<QString> words = interaction->options["text"].split("", QString::SkipEmptyParts);
    QString output = QString("<@%1>, ").arg(interaction->member.user.id);
    foreach(const QString &word, words) {
        output += characterMap[word.at(0)];
    }
    client->webhook_edit_message(interaction->token, output);
    emit quit();
}

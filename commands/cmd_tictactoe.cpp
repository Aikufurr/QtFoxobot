#include "cmd_tictactoe.h"

bool cmd_tictactoe::equals3(QString a, QString b, QString c) {
    return (a == b && b == c && !a.contains(":"));
}

QString cmd_tictactoe::checkWinner() {
    QString winner;

    // horizontal
    for (int i = 0; i < 3; i++) {
        if (equals3(matrix[i][0], matrix[i][1], matrix[i][2])) {
            winner = matrix[i][0];
        }
    }

    // Vertical
    for (int i = 0; i < 3; i++) {
        if (equals3(matrix[0][i], matrix[1][i], matrix[2][i])) {
            winner = matrix[0][i];
        }
    }

    // Diagonal
    if (equals3(matrix[0][0], matrix[1][1], matrix[2][2])) {
        winner = matrix[0][0];
    }
    if (equals3(matrix[2][0], matrix[1][1], matrix[0][2])) {
        winner = matrix[2][0];
    }
    int openSpots = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (matrix[i][j].contains(":")) {
                openSpots++;
            }
        }
    }
    if (winner == "" && openSpots == 0) {
        return "Tie";
    } else {
        return winner;
    }
}

QPair<int, int> cmd_tictactoe::nextTurn() {
    QVector<QVector<int>> available;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (matrix[i][j].contains(":")) {
                QVector<int> a {i, j};
                available.push_back(a);
            }
        }
    }
    if (available.size() == 0) {
        return QPair<int, int>(-1, -1);
    }
    int index = qFloor(rand() % available.size());
    QVector<int> spot = available[index];
    matrix[spot.at(0)][spot.at(1)] = ai;
    return QPair<int, int>(spot.at(0), spot.at(1));
}

int cmd_tictactoe::minimax(bool maximizing_player, int depth) {
    QString result = checkWinner();
    if (result != "" || depth == 0) {
        return result == player ? -10 : result == ai ? 10 : 0;
    }

    if (maximizing_player) {
        int bestScore = std::numeric_limits<int>::min();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (matrix[i][j].contains(":")) {
                    QString prev = matrix[i][j];
                    matrix[i][j] = ai;
                    int score = minimax(false, depth-1);
                    matrix[i][j] = prev;
                    bestScore = qMax(score, bestScore);
                }
            }
        }
        return bestScore;
    } else {
        int bestScore = std::numeric_limits<int>::max();
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (matrix[i][j].contains(":")) {
                    QString prev = matrix[i][j];
                    matrix[i][j] = player;
                    int score = minimax(true, depth-1);
                    matrix[i][j] = prev;
                    bestScore = qMin(score, bestScore);
                }
            }
        }
        return bestScore;
    }
}

QPair<int, int> cmd_tictactoe::bestMove(int depth) {
    int openSpots = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (matrix[i][j].contains(":")) {
                openSpots++;
            }
        }
    }
    if (openSpots == 0) {
        return QPair<int, int>(-1, -1);
    }
    int bestScore = std::numeric_limits<int>::min();
    QPair<int, int> move;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (matrix[i][j].contains(":")) {
                QString prev = matrix[i][j];
                matrix[i][j] = ai;
                int score = minimax(false, depth);
                matrix[i][j] = prev;
                if (score > bestScore) {
                    bestScore = score;
                    move.first = i;
                    move.second = j;
                }
            }
        }
    }
    matrix[move.first][move.second] = ai;
    return move;
}


cmd_tictactoe::cmd_tictactoe(Client *_client, Client::interaction_t *_interaction) {
    qRegisterMetaType<Client::reaction_t>("Client::reaction_t");
    client = _client;
    interaction = *_interaction;
    srand(time(0));

    connect(client, SIGNAL(message_reaction_add(Client::reaction_t)), this, SLOT(message_reaction_add(Client::reaction_t)));

    // add argument
    player  = QString("❌");
    ai      = QString("⭕");

    matrix = {
        {":one:", ":two:", ":three:"},
        {":four:", ":five:", ":six:"},
        {":seven:", ":eight:", ":nine:"}
    };
    reactions = {
        {QString("1️⃣"), QString("2️⃣"), QString("3️⃣")},
        {QString("4️⃣"), QString("5️⃣"), QString("6️⃣")},
        {QString("7️⃣"), QString("8️⃣"), QString("9️⃣")}
    };

    Client::embed_t embed;

    embed.title = QString("%1'%2 %3 Tic Tac Toe Game").arg(interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick, (interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick).endsWith("s") ? "" : "s", QString("%1%2").arg(interaction.sub_group[0].toUpper(), interaction.sub_group.mid(1)));
    embed.colour = -1;

    embed.description = QString("%1%2%3").arg(matrix[0][0], matrix[0][1], matrix[0][2]);
    embed.description += QString("\n%1%2%3").arg(matrix[1][0], matrix[1][1], matrix[1][2]);
    embed.description += QString("\n%1%2%3").arg(matrix[2][0], matrix[2][1], matrix[2][2]);

    embed.footer.text = "Game will end in 60s of no activity";

    qDebug() << "debug 1";
    msg = client->webhook_edit_message(interaction.token, "", embed);

    foreach(const QStringList &reaction_list, reactions) {
        foreach(const QString &reaction, reaction_list) {
            client->message_reaction_create(interaction.channel_id, msg.id, reaction);
        }
    }
    finished_reactions = true;

    timeout = new QTimer(this);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, [=](){
        Client::embed_t embed;

        embed.title = QString("%1'%2 %3 Tic Tac Toe Game").arg(interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick, (interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick).endsWith("s") ? "" : "s", QString("%1%2").arg(interaction.sub_group[0].toUpper(), interaction.sub_group.mid(1)));
        embed.colour = -1;

        embed.description = QString("%1%2%3").arg(matrix[0][0], matrix[0][1], matrix[0][2]);
        embed.description += QString("\n%1%2%3").arg(matrix[1][0], matrix[1][1], matrix[1][2]);
        embed.description += QString("\n%1%2%3").arg(matrix[2][0], matrix[2][1], matrix[2][2]);

        embed.footer.text = "Game ended due to 60s of no activity";
        msg = client->message_edit(interaction.channel_id, msg.id, "", embed);
        client->message_reaction_delete_all(interaction.channel_id, msg.id);
        emit quit();
    });

    timeout->start(60000);

}

void cmd_tictactoe::message_reaction_add(Client::reaction_t reaction) {
    if (reaction.me || reaction.message_id != msg.id || !finished_reactions || reaction.user.id != interaction.member.user.id) {
        return;
    }
    bool made_move = false;

    // X--
    // ---
    // ---
    if (reaction.emoji.name == QString("1️⃣")) {
        if (matrix[0][0] == ":one:") {
            matrix[0][0] = player;
            made_move = true;
        }
    }
    // -X-
    // ---
    // ---
    else if (reaction.emoji.name == QString("2️⃣")) {
        if (matrix[0][1] == ":two:") {
            matrix[0][1] = player;
            made_move = true;
        }
    }
    // --X
    // ---
    // ---
    else if (reaction.emoji.name == QString("3️⃣")) {
        if (matrix[0][2] == ":three:") {
            matrix[0][2] = player;
            made_move = true;
        }
    }

    // ---
    // X--
    // ---
    else if (reaction.emoji.name == QString("4️⃣")) {
        if (matrix[1][0] == ":four:") {
            matrix[1][0] = player;
            made_move = true;
        }
    }
    // ---
    // -X-
    // ---
    else if (reaction.emoji.name == QString("5️⃣")) {
        if (matrix[1][1] == ":five:") {
            matrix[1][1] = player;
            made_move = true;
        }
    }
    // ---
    // --X
    // ---
    else if (reaction.emoji.name == QString("6️⃣")) {
        if (matrix[1][2] == ":six:") {
            matrix[1][2] = player;
            made_move = true;
        }
    }

    // ---
    // ---
    // X--
    else if (reaction.emoji.name == QString("7️⃣")) {
        if (matrix[2][0] == ":seven:") {
            matrix[2][0] = player;
            made_move = true;
        }
    }
    // ---
    // ---
    // -X-
    else if (reaction.emoji.name == QString("8️⃣")) {
        if (matrix[2][1] == ":eight:") {
            matrix[2][1] = player;
            made_move = true;
        }
    }
    // ---
    // ---
    // --X
    else if (reaction.emoji.name == QString("9️⃣")) {
        if (matrix[2][2] == ":nine:") {
            matrix[2][2] = player;
            made_move = true;
        }
    }

    if (reaction.emoji.id.isEmpty()) {
        client->message_reaction_delete(interaction.channel_id, msg.id, reaction.emoji.name);
    } else {
        client->message_reaction_delete(interaction.channel_id, msg.id, QString(":%1:%2").arg(reaction.emoji.name, reaction.emoji.id));
    }

    if (made_move) {
        Client::embed_t embed;

        bool won = false;
        {
            QString result = checkWinner();
            if (result == "") {
                embed.footer.text = "Game will end in 60s of no activity";
            } else {
                timeout->stop();
                won = true;
                if (result == player) {
                    embed.footer.text = QString("%1 Wins!").arg(interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick);
                } else if (result == ai) {
                    embed.footer.text = QString("%1 Wins!").arg(client->member_get(client->me_get().id, interaction.guild_id).nick.isEmpty() ? client->member_get(client->me_get().id, interaction.guild_id).user.username : client->member_get(client->me_get().id, interaction.guild_id).nick);
                } else if (result == "Tie") {
                    embed.footer.text = "Tie!";
                } else {
                    embed.footer.text = "Unknown game state";
                }
                client->message_reaction_delete_all(interaction.channel_id, msg.id);
                emit quit();
            }
        }
        if (!won) {
            QPair<int, int> move;
            if (interaction.sub_group == "easy") {
                move = nextTurn();
            }
            if (interaction.sub_group == "medium") {
                move = bestMove(1);
            }
            if (interaction.sub_group == "hard") {
                move = bestMove(2);
            }
            if (interaction.sub_group == "expert") {
                move = bestMove(-1);
            }
            if (!(move.first == -1 || move.second == -1)) {
                client->message_reaction_delete(interaction.channel_id, msg.id, reactions[move.first][move.second]);
            }
            QString result = checkWinner();
            if (result == "") {
                embed.footer.text = "Game will end in 60s of no activity";
            } else {
                timeout->stop();
                if (result == player) {
                    embed.footer.text = QString("%1 Wins!").arg(interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick);
                } else if (result == ai) {
                    embed.footer.text = QString("%1 Wins!").arg(client->member_get(client->me_get().id, interaction.guild_id).nick.isEmpty() ? client->member_get(client->me_get().id, interaction.guild_id).user.username : client->member_get(client->me_get().id, interaction.guild_id).nick);
                } else if (result == "Tie") {
                    embed.footer.text = "Tie!";
                } else {
                    embed.footer.text = "Unknown game state";
                }
                client->message_reaction_delete_all(interaction.channel_id, msg.id);
                emit quit();
            }
        }

        embed.title = QString("%1'%2 %3 Tic Tac Toe Game").arg(interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick, (interaction.member.nick.isEmpty() ? interaction.member.user.username : interaction.member.nick).endsWith("s") ? "" : "s", QString("%1%2").arg(interaction.sub_group[0].toUpper(), interaction.sub_group.mid(1)));
        embed.colour = -1;

        embed.description = QString("%1%2%3").arg(matrix[0][0], matrix[0][1], matrix[0][2]);
        embed.description += QString("\n%1%2%3").arg(matrix[1][0], matrix[1][1], matrix[1][2]);
        embed.description += QString("\n%1%2%3").arg(matrix[2][0], matrix[2][1], matrix[2][2]);

        msg = client->webhook_edit_message(interaction.token, "", embed, msg.id);
        timeout->start(60000);
    }
}

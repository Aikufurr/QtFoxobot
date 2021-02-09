#include "cmd_leaderboard.h"

cmd_leaderboard::cmd_leaderboard(Client *client, Client::interaction_t *interaction, DbManager *dbManager) {
    Client::guild_t guild = client->getGuild(interaction->guild_id);
    int amount = 10;
    bool isChart = false;
    if (interaction->options.size() > 0) {
        if (interaction->options.contains("custom")) {
            amount = interaction->options["custom"].toInt();
            if (amount < 1) {
                amount = 1;
            }
            if (amount > 20) {
                amount = 20;
            }
        } else if (interaction->options.contains("amount")) {
            amount = interaction->options["amount"].toInt();
        }

        if (interaction->options.contains("chart")) {
            isChart = interaction->options["chart"].toInt() == 1 ? true : false;
        }
    }

    QJsonArray leaderboard = dbManager->rank_leaderboard(interaction->guild_id, amount);


    Client::embed_t embed;
    embed.title = QString("%1'%2 Leaderboard").arg(guild.name, guild.name.endsWith("s") ? "" : "s");
    embed.description = QString("The top %1 member%2").arg(QString::number(amount), amount != 1 ? "s" : "");
    embed.colour = 16757760; // FFB400 - Orange

    QList<Client::embed_field_t> fields;
    foreach(const QJsonValue &value, leaderboard) {
        QJsonObject result = value.toObject();
        Client:: member_t member = client->getMember(result["user_id"].toString(), interaction->guild_id);

        Client::embed_field_t field;
        field.name = QString("Rank %1").arg(QString::number(result["position"].toInt()));
        field.value = QString("**%1** %2 at %3 message%4").arg(member.nick.isEmpty() ? member.user.username : member.nick, member.nick.isEmpty() ? "" : QString("*%1*").arg(member.user.username), QString::number(result["rank"].toInt()), result["rank"].toInt() == 1 ? "" : "s");
        field.is_inline = false;
        fields.push_back(field);

    }
    embed.fields = fields;
    embed.author.name = QString("%1").arg(interaction->member.user.username);
    if (interaction->member.user.avatar.isNull()) {
        embed.author.icon_url = QString("https://cdn.discordapp.com/embed/avatars/%1.png").arg(QString::number(interaction->member.user.discriminator.toInt() % 5));
    } else {
        embed.author.icon_url = QString("https://cdn.discordapp.com/avatars/%1/%2.png").arg(interaction->member.user.id, interaction->member.user.avatar);
    }
    if (!guild.icon.isNull()) {
        embed.thumbnail_url = QString("https://cdn.discordapp.com/icons/%1/%2.%3").arg(interaction->guild_id, guild.icon, guild.icon.startsWith("a_") ? "gif" : "png");
    }

    if (!isChart) {
        client->send_message(interaction->channel_id, "", embed);
    } else {
        QLineSeries *series = new QLineSeries();
        series->setPointLabelsVisible(true); // Argument
        series->setPointLabelsColor(Qt::black);
        //        series->setPointLabelsFormat("@yPoint");
        series->setPointLabelsClipping(false);
        series->setPointLabelsColor(Qt::white);

        foreach(const QJsonValue &value, leaderboard) {
            QJsonObject result = value.toObject();
            series->append(result["position"].toInt(), result["rank"].toInt());
        }

        QChart *chart = new QChart();
        chart->legend()->hide();
        chart->addSeries(series);
        QValueAxis *axisX = new QValueAxis;
        axisX->setTitleText("Rank");
        axisX->setReverse(true);
        axisX->setLabelFormat("%.0f");
        axisX->setLabelsColor(Qt::white);
        axisX->setTitleBrush(QBrush(Qt::white));
        chart->addAxis(axisX, Qt::AlignBottom);
        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText("Messages");
        axisY->applyNiceNumbers();
        axisY->setLabelsColor(Qt::white);
        axisY->setTitleBrush(QBrush(Qt::white));
        chart->addAxis(axisY, Qt::AlignLeft);

        series->attachAxis(axisX);
        series->attachAxis(axisY);

        chart->layout()->setContentsMargins(0, 0, 0, 0);
        chart->setBackgroundBrush(Qt::transparent);
        QString g_name = client->getGuild(interaction->guild_id).name;
        chart->setTitle(QString("<b>%1'%2 Leaderboard</b>").arg(g_name, g_name.endsWith("s") ? "" : "s"));
        chart->setTitleBrush(QBrush(Qt::white));

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        chartView->resize(600, 400);
        //            chartView->resize(400, 200);

        chartView->grab();
        QPixmap pix(chartView->size());
        pix.fill(Qt::transparent);//fill with transparent color  -- possible argument?
        QPainter painter(&pix);

        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        chartView->render(&painter);
        painter.end();

        QByteArray file;
        QBuffer buffer(&file);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "PNG");

        client->send_file_message(interaction->channel_id, file, embed);
    }
}

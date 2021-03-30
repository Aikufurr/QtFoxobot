#include "util.h"

Util::Util() {

}

int _delayFor(const int ms) {
    QEventLoop loop;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.setSingleShot(true);
    timer.start(ms);
    loop.exec();
    loop.deleteLater();
    timer.deleteLater();
    return 0;
};

QFuture<int> Util::delayFor(int ms) {
    return QtConcurrent::run(_delayFor, ms);
}

QString Util::escapeMarkdown(QString text,
                             bool escapeCodeBlock,
                             bool escapeInlineCode,
                             bool escapeBold,
                             bool escapeItalic,
                             bool escapeUnderline,
                             bool escapeStrikethrough,
                             bool escapeSpoiler,
                             bool escapeCodeBlockContent,
                             bool escapeInlineCodeConent) {
    if (!escapeCodeBlockContent) {
        QString escaped;
        QList<QString> splitText = text.split("```");
        for (int i = 0; i < splitText.size(); i++) {
            if (i % 2 && i != splitText.size() - 1) {
                splitText.replace(i, splitText.at(i));
            }
            splitText.replace(i, Util::escapeMarkdown(splitText.at(i),
                                                      escapeCodeBlock,
                                                      escapeInlineCode,
                                                      escapeBold,
                                                      escapeItalic,
                                                      escapeUnderline,
                                                      escapeStrikethrough,
                                                      escapeSpoiler,
                                                      escapeCodeBlockContent,
                                                      escapeInlineCodeConent));
        }
        return splitText.join(escapeCodeBlock ? "\\`\\`\\`" : "```");
    }
    if (!escapeInlineCodeConent) {
        QString escaped;
        QList<QString> splitText = text.split(QRegularExpression(R"**((?<=^|[^`])`(?=[^`]|$))**"));
        for (int i = 0; i < splitText.size(); i++) {
            if (i % 2 && i != splitText.size() - 1) {
                splitText.replace(i, splitText.at(i));
            }
            splitText.replace(i, Util::escapeMarkdown(splitText.at(i),
                                                      escapeCodeBlock,
                                                      escapeInlineCode,
                                                      escapeBold,
                                                      escapeItalic,
                                                      escapeUnderline,
                                                      escapeStrikethrough,
                                                      escapeSpoiler,
                                                      escapeCodeBlockContent,
                                                      escapeInlineCodeConent));
        }
        return splitText.join(escapeCodeBlock ? "\\`" : "`");
    }
    if (escapeInlineCode) {
        text = Util::escapeInlineCode(text);
    }
    if (escapeCodeBlock) {
        text = Util::escapeCodeBlock(text);
    }
    if (escapeItalic) {
        text = Util::escapeItalic(text);
    }
    if (escapeBold) {
        text = Util::escapeBold(text);
    }
    if (escapeUnderline) {
        text = Util::escapeUnderline(text);
    }
    if (escapeStrikethrough) {
        text = Util::escapeStrikethrough(text);
    }
    if (escapeSpoiler) {
        text = Util::escapeSpoiler(text);
    }
    return text;
}

/**
  * Escapes code block markdown in a string.
  * @param {QString} text Content to escape
  * @returns {QString}
  */
QString Util::escapeCodeBlock(QString text) {
    return text.replace(QRegularExpression(R"**(```)**"), "\\`\\`\\`");
}

/**
  * Escapes inline code markdown in a string.
  * @param {QString} text Content to escape
  * @returns {QString}
  */
QString Util::escapeInlineCode(QString text) {
    return text.replace(QRegularExpression(R"**(`)**"), "\\`");
}

/**
  * Escapes italic markdown in a string.
  * @param {QString} text Content to escape
  * @returns {QString}
  */
QString Util::escapeItalic(QString text) {
    int i = 0;
    QRegularExpression rx(R"**((?<=^|[^*])\*([^*]|\*\*|$))**");
    QString processedText = text;
    int offset = 0;

    QRegularExpressionMatchIterator iterator = rx.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        if (match.hasMatch()) {
            if (match.captured(0) == "**") {
                processedText.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), (++i % 2 ? QString("\\%1").arg(match.captured(0)) : QString("%1\\").arg(match.captured(0))));
            } else {
                processedText.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), QString("\\%1").arg(match.captured(0)));
            }
            offset += (match.capturedEnd()-match.capturedStart())-1;
        }
    }
    i = 0;
    offset = 0;
    QString processedText2 = processedText;
    QRegularExpressionMatchIterator iterator2 = rx.globalMatch(processedText);
    while (iterator2.hasNext()) {
        QRegularExpressionMatch match = iterator2.next();
        if (match.hasMatch()) {
            if (match.captured(0) == "__") {
                processedText2.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), (++i % 2 ? QString("\\%1").arg(match.captured(0)) : QString("%1\\").arg(match.captured(0))));
            } else {
                processedText2.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), QString("\\%1").arg(match.captured(0)));
            }
            offset += (match.capturedEnd()-match.capturedStart())-1;
        }
    }

    return processedText2;
}

/**
 * Escapes bold markdown in a string.
 * @param {QString} text Content to escape
 * @returns {QString}
 */
QString Util::escapeBold(QString text) {
    QRegularExpression rx(R"**(\*\*(\*)?)**");
    QString processedText = text;
    int offset = 0;

    QRegularExpressionMatchIterator iterator = rx.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        if (match.hasMatch()) {
            if (!match.captured(0).isEmpty()) {
                processedText.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), QString("\\*\\*"));
            }
            offset += (match.capturedEnd()-match.capturedStart());
        }
    }
    return processedText;
}

/**
 * Escapes underline markdown in a string.
 * @param {QString} text Content to escape
 * @returns {QString}
 */
QString Util::escapeUnderline(QString text) {
    QRegularExpression rx(R"**(__(_)?)**");
    QString processedText = text;
    int offset = 0;

    QRegularExpressionMatchIterator iterator = rx.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        if (match.hasMatch()) {
            if (!match.captured(0).isEmpty()) {
                processedText.replace(match.capturedStart() + offset, ((match.capturedEnd()+offset)-(match.capturedStart()+offset)), QString("\\_\\_"));
            }
            offset += (match.capturedEnd()-match.capturedStart());
        }
    }
    return processedText;
}

/**
  * Escapes strikethrough markdown in a string.
  * @param {QString} text Content to escape
  * @returns {QString}
  */
QString Util::escapeStrikethrough(QString text) {
    return text.replace(QRegularExpression(R"**(~~)**"), "\\~\\~");
}

/**
  * Escapes spoiler markdown in a string.
  * @param {QString} text Content to escape
  * @returns {QString}
  */
QString Util::escapeSpoiler(QString text) {
    return text.replace(QRegularExpression(R"**(\|\|)**"), "\\|\\|");
}


/**
   * Splits a byte array into multiple chunks at a designated character that do not exceed a specific length.
   * @param {QString} text Content to split
   * @param {int} maxLength Max length of text
   * @param {QString} splitChar Character to split at
   * @param {QString} prepend What to prepend on the start
   * @param {QString} append What to append on the end
   * @returns {QList<QByteArray>}
   */
QList<QString> Util::splitText(QString text, int maxLength, QString splitChar, QString prepend, QString append) {
    QList<QString> texts;
    if (text.size() <= maxLength) {
        texts.push_back(text);
        return texts;
    }
    const QList<QString> splitText = text.split(splitChar);

    QString txt = "";
    foreach (const QString chunk, splitText) {
        if (!txt.isEmpty() && (txt + splitChar + chunk + append).size() > maxLength) {
            texts.push_back(txt + append);
            txt = prepend;
        }
        txt += (!txt.isEmpty() && txt != prepend ? splitChar : "") + chunk;
    }
    texts.push_back(txt);
    return texts;
}

/**
 * Breaks user, role and everyone/here mentions by adding a zero width space after every @ character
 * @param {QString} text The string to sanitize
 * @returns {QString}
 */
QString Util::removeMentions(QString text) {
    return text.replace(QRegularExpression(R"**(@)**"), "@\u200b");
}

/**
   * The content to put in a codeblock with all codeblock fences replaced by the equivalent backticks.
   * @param {QString} text The string to be converted
   * @returns {QString}
   */
QString Util::cleanCodeBlockContent(QString text) {
    return text.replace(QRegularExpression(R"**(```)**"), "`\u200b``");
}

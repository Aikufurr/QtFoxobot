#ifndef UTIL_H
#define UTIL_H

#include <QEventLoop>
#include <QFuture>
#include <QObject>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

class Util : public QObject {
    Q_OBJECT

public:
    Util();


    static QFuture<int> delayFor(int ms);
	

    static QString escapeCodeBlock(QString text);
    static QString escapeInlineCode(QString text);
    static QString escapeItalic(QString text);
    static QString escapeBold(QString text);
    static QString escapeUnderline(QString text);
    static QString escapeStrikethrough(QString text);
    static QString escapeSpoiler(QString text);
	
    static QString escapeMarkdown(QString text,
                           bool escapeCodeBlock = true,
                           bool escapeInlineCode = true,
                           bool escapeBold = true,
                           bool escapeItalic = true,
                           bool escapeUnderline = true,
                           bool escapeStrikethrough = true,
                           bool escapeSpoiler = true,
                           bool escapeCodeBlockContent = true,
                           bool escapeInlineCodeConent = true);

    static QList<QString> splitText(QString text, int maxLength = 2000, QString splitChar = "", QString prepend = "", QString append = "");

    static QString removeMentions(QString str);

    static QString cleanCodeBlockContent(QString text);

};

#endif // UTIL_H

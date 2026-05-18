#include "spell_highlighter.h"

spell_highlighter::spell_highlighter(const spell_checker& checker, QTextDocument* parent)
    : QSyntaxHighlighter(parent)
    , checker_(checker)
    , word_re_(QStringLiteral("[A-Za-z]+"))
{
    misspelled_fmt_.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    misspelled_fmt_.setUnderlineColor(Qt::red);
}

void spell_highlighter::highlightBlock(const QString& text)
{
    QRegularExpressionMatchIterator it = word_re_.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        if (!checker_.check_word(match.captured())) {
            setFormat(match.capturedStart(),
                      match.capturedLength(),
                      misspelled_fmt_);
        }
    }
}
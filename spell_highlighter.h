#ifndef SIMPLE_NOTEPAD_PROJECT_SPELL_HIGHLIGHTER_H
#define SIMPLE_NOTEPAD_PROJECT_SPELL_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class spell_highlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit spell_highlighter(const spell_checker& checker, QTextDocument* parent);

protected:
    void highlightBlock(const QString& text) override;

private:
    const spell_checker& checker_;
    QTextCharFormat      misspelled_fmt_;
    QRegularExpression   word_re_;
};

#endif // SIMPLE_NOTEPAD_PROJECT_SPELL_HIGHLIGHTER_H
#ifndef SIMPLE_NOTEPAD_PROJECT_SPELL_CHECKER_H
#define SIMPLE_NOTEPAD_PROJECT_SPELL_CHECKER_H

#include <QString>
#include <set>
#include <string>
#include <vector>

class spell_checker {
public:
    bool load_dictionary(const QString& path);
    bool check_word(const QString& word) const;
    std::vector<QString> suggest(const QString& word, int max_suggestions = 5) const;

private:
    static std::string normalise(const QString& word);

    std::set<std::string>        dict_;
    std::vector<std::string>     word_list_;
};

#endif //SIMPLE_NOTEPAD_PROJECT_SPELL_CHECKER_H
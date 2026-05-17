#include "spell_checker.h"

#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <unordered_set>

bool spell_checker::load_dictionary(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const std::string w = in.readLine().trimmed().toLower().toStdString();
        if (!w.empty() && dict_.insert(w).second)
            word_list_.push_back(w);
    }
    return true;
}

std::string spell_checker::normalise(const QString& word)
{
    std::string out;
    out.reserve(static_cast<std::size_t>(word.size()));
    for (const QChar ch : word) {
        if (ch.isLetter())
            out += static_cast<char>(ch.toLower().toLatin1());
    }
    return out;
}

bool spell_checker::check_word(const QString& word) const
{
    if (dict_.empty()) return true;
    const std::string norm = normalise(word);
    if (norm.empty()) return true;
    return dict_.count(norm) > 0;
}

std::vector<QString> spell_checker::suggest(const QString& word, const int max_suggestions) const
{
    const std::string w = normalise(word);
    if (w.empty() || dict_.empty())
        return {};

    std::unordered_set<std::string> candidate_set;
    const std::size_t n = w.size();
    static constexpr std::string_view alpha = "abcdefghijklmnopqrstuvwxyz";

    for (std::size_t i = 0; i < n; ++i) {
        std::string c = w.substr(0, i) + w.substr(i + 1);
        if (dict_.count(c)) candidate_set.insert(std::move(c));
    }

    for (std::size_t i = 0; i < n; ++i) {
        for (const char ch : alpha) {
            if (ch == w[i]) continue;
            std::string c = w;
            c[i] = ch;
            if (dict_.count(c)) candidate_set.insert(std::move(c));
        }
    }

    for (std::size_t i = 0; i <= n; ++i) {
        for (const char ch : alpha) {
            std::string c = w.substr(0, i) + ch + w.substr(i);
            if (dict_.count(c)) candidate_set.insert(std::move(c));
        }
    }

    for (std::size_t i = 0; i + 1 < n; ++i) {
        std::string c = w;
        std::swap(c[i], c[i + 1]);
        if (dict_.count(c)) candidate_set.insert(std::move(c));
    }

    if (candidate_set.empty())
        return {};

    std::vector<std::pair<std::size_t, std::string>> ranked;
    ranked.reserve(candidate_set.size());

    std::unordered_set<std::string> remaining = candidate_set;
    for (std::size_t idx = 0; idx < word_list_.size() && !remaining.empty(); ++idx) {
        auto it = remaining.find(word_list_[idx]);
        if (it != remaining.end()) {
            ranked.emplace_back(idx, *it);
            remaining.erase(it);
        }
    }

    std::sort(ranked.begin(), ranked.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    std::vector<QString> result;
    const int limit = std::min(max_suggestions, static_cast<int>(ranked.size()));
    result.reserve(static_cast<std::size_t>(limit));
    for (int i = 0; i < limit; ++i)
        result.push_back(QString::fromStdString(ranked[static_cast<std::size_t>(i)].second));

    return result;
}
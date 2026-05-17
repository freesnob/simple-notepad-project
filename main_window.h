#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include "spell_checker.h"
#include "spell_highlighter.h"
#include "text_transform.h"


#include <QLabel>
#include <QDialog>
#include <QMainWindow>
#include <QString>
#include <QTextEdit>
#include <memory>
#include <vector>

namespace Ui {
    class find_replace_dialog;
    class word_frequency_dialog;
}

class main_window : public QMainWindow {
public:
    main_window();
    ~main_window() override;

private:
    void setup_file_menu();
    void setup_edit_menu();
    void setup_format_menu();
    void setup_format_toolbar();
    void setup_search_menu();
    void setup_tools_menu();
    void run_spell_check();
    void on_editor_context_menu(const QPoint& pos);
    void setup_view_menu();
    void setup_status_bar();
    void update_cursor_position();
    QLabel* cursor_position_label { nullptr };


    void open_file();
    void save_file();
    void save_file_as();
    void update_title();

    void apply_transform(const text_transform& transform) const;

    void show_find_replace_dialog();
    void find_next(const QString& term, QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;
    void replace_current(const QString& term, const QString& replacement,
        QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;
    void replace_all(const QString& term, const QString& replacement,
        QTextDocument::FindFlags flags = QTextDocument::FindFlags()) const;

    void show_word_frequency();

    QTextEdit* editor { nullptr };
    QString current_file;
    std::vector<std::unique_ptr<text_transform>> transforms;

    QDialog* find_replace_dlg { nullptr };
    std::unique_ptr<Ui::find_replace_dialog> find_replace_ui;

    spell_checker      spell_checker_;
    spell_highlighter* highlighter_ { nullptr };
};

#endif // MAIN_WINDOW_H
#include "main_window.h"
#include "spell_checker.h"
#include "notepad_exception.h"
#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"
#include <QPrintDialog>
#include <QPrinter>
#include <QFontDialog>
#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QFont>
#include <QHeaderView>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextStream>
#include <QToolBar>
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

main_window::main_window()
{
    setWindowTitle("Notepad");
    resize(800, 600);

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    transforms.push_back(std::make_unique<uppercase_transform>());
    transforms.push_back(std::make_unique<lowercase_transform>());
    transforms.push_back(std::make_unique<capitalize_transform>());
    transforms.push_back(std::make_unique<sentence_case_transform>());
    transforms.push_back(std::make_unique<swap_case_transform>());

    setup_file_menu();
    setup_edit_menu();
    setup_format_menu();
    setup_format_toolbar();
    setup_search_menu();
    setup_tools_menu();
    setup_view_menu();
    setup_status_bar();

    spell_checker_.load_dictionary("data/words.txt");
    highlighter_ = new spell_highlighter(spell_checker_, editor->document());

    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editor, &QWidget::customContextMenuRequested,
            this, &main_window::on_editor_context_menu);
}

main_window::~main_window() = default;

void main_window::setup_file_menu()
{
    auto* file_menu = menuBar()->addMenu("File");

    const auto* action_new = file_menu->addAction("New");
    connect(action_new, &QAction::triggered, this, [this] {
        editor->clear();
        current_file.clear();
        update_title();
    });

    file_menu->addSeparator();

    const auto* action_open = file_menu->addAction("Open...");
    connect(action_open, &QAction::triggered, this, [this] {
        open_file();
    });

    const auto* action_save = file_menu->addAction("Save");
    connect(action_save, &QAction::triggered, this, [this] {
        save_file();
    });

    const auto* action_save_as = file_menu->addAction("Save As...");
    connect(action_save_as, &QAction::triggered, this, [this] {
        save_file_as();
    });

    file_menu->addSeparator();
    file_menu->addSeparator();

    auto* action_print = file_menu->addAction("Print...");
    action_print->setShortcut(QKeySequence::Print);
    connect(action_print, &QAction::triggered, this, [this] {
        QPrinter printer(QPrinter::HighResolution);
        QPrintDialog dialog(&printer, this);
        dialog.setWindowTitle("Print");
        if (dialog.exec() == QDialog::Accepted)
            editor->print(&printer);
    });

    file_menu->addSeparator();

    const auto* action_exit = file_menu->addAction("Exit");
    connect(action_exit, &QAction::triggered, this, [] {
        QApplication::quit();
    });
}

void main_window::setup_edit_menu()
{
    auto* edit_menu = menuBar()->addMenu("Edit");

    auto* action_undo = edit_menu->addAction("Undo");
    action_undo->setShortcut(QKeySequence::Undo);
    connect(action_undo, &QAction::triggered, editor, &QTextEdit::undo);

    auto* action_redo = edit_menu->addAction("Redo");
    action_redo->setShortcut(QKeySequence::Redo);
    connect(action_redo, &QAction::triggered, editor, &QTextEdit::redo);

    edit_menu->addSeparator();

    auto* action_cut = edit_menu->addAction("Cut");
    action_cut->setShortcut(QKeySequence::Cut);
    connect(action_cut, &QAction::triggered, editor, &QTextEdit::cut);

    auto* action_copy = edit_menu->addAction("Copy");
    action_copy->setShortcut(QKeySequence::Copy);
    connect(action_copy, &QAction::triggered, editor, &QTextEdit::copy);

    auto* action_paste = edit_menu->addAction("Paste");
    action_paste->setShortcut(QKeySequence::Paste);
    connect(action_paste, &QAction::triggered, editor, &QTextEdit::paste);

    edit_menu->addSeparator();

    auto* action_select_all = edit_menu->addAction("Select All");
    action_select_all->setShortcut(QKeySequence::SelectAll);
    connect(action_select_all, &QAction::triggered, editor, &QTextEdit::selectAll);
}

void main_window::setup_format_menu()
{
    auto* format_menu = menuBar()->addMenu("Format");
    auto* text_case_menu = format_menu->addMenu("Text Case");

    for (const auto& transform : transforms) {
        const auto* action = text_case_menu->addAction(QString::fromStdString(transform->name()));
        const text_transform* t = transform.get();
        connect(action, &QAction::triggered, this, [this, t] {
            apply_transform(*t);
        });
    }

    format_menu->addSeparator();

    auto* action_font = format_menu->addAction("Font...");
    connect(action_font, &QAction::triggered, this, [this] {
        bool ok = false;
        const QFont current = editor->currentFont();
        const QFont selected = QFontDialog::getFont(&ok, current, this, "Font");
        if (!ok)
            return;

        QTextCursor cursor = editor->textCursor();
        if (cursor.hasSelection()) {
            QTextCharFormat fmt;
            fmt.setFont(selected);
            cursor.mergeCharFormat(fmt);
            editor->setTextCursor(cursor);
        } else {
            editor->setFont(selected);
        }
    });
}



void main_window::setup_format_toolbar()
{
    auto* toolbar = addToolBar("Format");
    toolbar->setIconSize(QSize(16, 16));

    auto* action_bold = toolbar->addAction(QIcon("data/images/bold.svg"), "B");
    action_bold->setCheckable(true);
    action_bold->setShortcut(QKeySequence("Ctrl+B"));
    connect(action_bold, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        editor->mergeCurrentCharFormat(fmt);
    });

    auto* action_italic = toolbar->addAction(QIcon("data/images/italic.svg"), "I");
    action_italic->setCheckable(true);
    action_italic->setShortcut(QKeySequence("Ctrl+I"));
    connect(action_italic, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontItalic(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    auto* action_underline = toolbar->addAction(QIcon("data/images/underline.svg"), "U");
    action_underline->setCheckable(true);
    action_underline->setShortcut(QKeySequence("Ctrl+U"));
    connect(action_underline, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontUnderline(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    connect(editor, &QTextEdit::currentCharFormatChanged,
        this, [action_bold, action_italic, action_underline](const QTextCharFormat& fmt) {
            action_bold->setChecked(fmt.fontWeight() == QFont::Bold);
            action_italic->setChecked(fmt.fontItalic());
            action_underline->setChecked(fmt.fontUnderline());
        });
}

void main_window::setup_search_menu()
{
    auto* search_menu = menuBar()->addMenu("Search");

    auto* action_find_replace = search_menu->addAction("Find / Replace...");
    action_find_replace->setShortcut(QKeySequence::Find);
    connect(action_find_replace, &QAction::triggered, this, [this] {
        show_find_replace_dialog();
    });
}

void main_window::setup_tools_menu()
{
    auto* tools_menu = menuBar()->addMenu("Tools");

    const auto* action_word_freq = tools_menu->addAction("Word Frequency...");
    connect(action_word_freq, &QAction::triggered, this, [this] {
        show_word_frequency();
    });

    tools_menu->addSeparator();

    const auto* action_spell_check = tools_menu->addAction("Check Spelling...");
    connect(action_spell_check, &QAction::triggered, this, [this] {
        run_spell_check();
    });
}

void main_window::apply_transform(const text_transform& transform) const
{
    auto cursor = editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
    }
    const int start = cursor.selectionStart();
    const QString selected = cursor.selectedText().replace(QChar::ParagraphSeparator, '\n');
    const std::string original = selected.toStdString();
    const auto result = transform.apply(original);

    cursor.beginEditBlock();
    for (std::size_t i = 0; i < result.size(); ++i) {
        if (original[i] != result[i]) {
            cursor.setPosition(start + static_cast<int>(i));
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            cursor.insertText(QString(QChar(result[i])), cursor.charFormat());
        }
    }
    cursor.endEditBlock();
}

void main_window::open_file()
{
    const auto path = QFileDialog::getOpenFileName(this, "Open File");
    if (path.isEmpty()) {
        return;
    }

    try {
        QFile file(path);
        if (!file.exists()) {
            throw file_not_found_exception(path.toStdString());
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw file_read_exception(path.toStdString());
        }
        QTextStream in(&file);
        editor->setPlainText(in.readAll());
        current_file = path;
        update_title();
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", QString::fromStdString(ex.what()));
    }
}

void main_window::save_file()
{
    if (current_file.isEmpty()) {
        save_file_as();
        return;
    }

    try {
        QFile file(current_file);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw file_write_exception(current_file.toStdString());
        }
        QTextStream out(&file);
        out << editor->toPlainText();
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", QString::fromStdString(ex.what()));
    }
}

void main_window::save_file_as()
{
    const auto path = QFileDialog::getSaveFileName(this, "Save File As");
    if (path.isEmpty()) {
        return;
    }
    current_file = path;
    save_file();
    update_title();
}

void main_window::update_title()
{
    if (current_file.isEmpty()) {
        setWindowTitle("Notepad");
    } else {
        setWindowTitle("Notepad: " + current_file);
    }
}

void main_window::show_find_replace_dialog()
{
    if (!find_replace_dlg) {
        find_replace_dlg = new QDialog(this);
        find_replace_ui = std::make_unique<Ui::find_replace_dialog>();
        find_replace_ui->setupUi(find_replace_dlg);

        auto current_flags = [this] {
            auto flags = QTextDocument::FindFlags();
            if (find_replace_ui->case_sensitive_check->isChecked()) {
                flags |= QTextDocument::FindCaseSensitively;
            }
            return flags;
        };

        connect(find_replace_ui->find_next_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                find_next(find_replace_ui->find_input->text(), current_flags());
            });
        connect(find_replace_ui->replace_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                replace_current(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), current_flags());
            });
        connect(find_replace_ui->replace_all_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                replace_all(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), current_flags());
            });
        connect(find_replace_ui->close_button, &QPushButton::clicked,
            find_replace_dlg, [this] { find_replace_dlg->hide(); });
    }

    find_replace_dlg->show();
    find_replace_dlg->raise();
    find_replace_dlg->activateWindow();
}

void main_window::find_next(const QString& term, const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto found = editor->document()->find(term, editor->textCursor(), flags);
    if (found.isNull()) {
        auto from_start = editor->textCursor();
        from_start.movePosition(QTextCursor::Start);
        found = editor->document()->find(term, from_start, flags);
    }
    if (!found.isNull()) {
        editor->setTextCursor(found);
    }
}

void main_window::replace_current(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (auto cursor = editor->textCursor(); cursor.hasSelection()) {
        cursor.insertText(replacement);
        editor->setTextCursor(cursor);
    }
    find_next(term, flags);
}

void main_window::replace_all(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto start_cursor = editor->textCursor();
    start_cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(start_cursor);

    while (true) {
        const auto found = editor->document()->find(term, editor->textCursor(), flags);
        if (found.isNull()) {
            break;
        }
        editor->setTextCursor(found);
        auto c = editor->textCursor();
        c.insertText(replacement);
        editor->setTextCursor(c);
    }
}

void main_window::show_word_frequency()
{
    const auto text = editor->toPlainText().toLower().toStdString();

    std::map<std::string, int> freq;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        std::erase_if(word, [](const unsigned char c) {
            return !std::isalpha(c);
        });
        if (!word.empty()) {
            ++freq[word];
        }
    }

    std::vector<std::pair<std::string, int>> sorted_freq(freq.begin(), freq.end());
    std::sort(sorted_freq.begin(), sorted_freq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    auto* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    Ui::word_frequency_dialog ui;
    ui.setupUi(dialog);

    ui.frequency_table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui.frequency_table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ui.frequency_table->setRowCount(static_cast<int>(sorted_freq.size()));
    for (int i = 0; i < static_cast<int>(sorted_freq.size()); ++i) {
        const auto& [w, count] = sorted_freq[i];
        ui.frequency_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(w)));
        auto* count_item = new QTableWidgetItem(QString::number(count));
        count_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.frequency_table->setItem(i, 1, count_item);
    }
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    dialog->exec();
}

void main_window::run_spell_check()
{
    highlighter_->setDocument(nullptr);
    highlighter_->setDocument(editor->document());
}

void main_window::on_editor_context_menu(const QPoint& pos)
{
    QMenu* menu = editor->createStandardContextMenu();

    QTextCursor click_cursor = editor->cursorForPosition(pos);
    click_cursor.select(QTextCursor::WordUnderCursor);
    const QString word = click_cursor.selectedText();

    if (!word.isEmpty() && !spell_checker_.check_word(word)) {
        const std::vector<QString> suggestions =
            spell_checker_.suggest(word, /*max_suggestions=*/5);

        if (!suggestions.empty()) {
            menu->insertSeparator(menu->actions().first());

            QAction* insert_after = menu->actions().first();
            for (const QString& suggestion : suggestions) {
                auto* act = new QAction(suggestion, menu);
                connect(act, &QAction::triggered, this,
                    [this, click_cursor, suggestion]() mutable {
                        click_cursor.insertText(suggestion);
                        editor->setTextCursor(click_cursor);
                    });
                menu->insertAction(insert_after, act);
                insert_after = act;
            }

            menu->insertSeparator(suggestions.empty()
                ? menu->actions().first()
                : menu->actions().at(static_cast<int>(suggestions.size())));
        } else {
            auto* no_suggestions = new QAction("(No suggestions)", menu);
            no_suggestions->setEnabled(false);
            menu->insertAction(menu->actions().first(), no_suggestions);
            menu->insertSeparator(menu->actions().at(1));
        }
    }

    menu->exec(editor->viewport()->mapToGlobal(pos));
    delete menu;
}

void main_window::setup_view_menu()
{
    auto* view_menu = menuBar()->addMenu("View");

    auto* action_zoom_in = view_menu->addAction("Zoom In");
    action_zoom_in->setShortcut(QKeySequence("Ctrl++"));
    connect(action_zoom_in, &QAction::triggered, this, [this] {
        editor->zoomIn(1);
    });

    auto* action_zoom_out = view_menu->addAction("Zoom Out");
    action_zoom_out->setShortcut(QKeySequence("Ctrl+-"));
    connect(action_zoom_out, &QAction::triggered, this, [this] {
        editor->zoomOut(1);
    });

    auto* action_zoom_reset = view_menu->addAction("Reset Zoom");
    action_zoom_reset->setShortcut(QKeySequence("Ctrl+0"));
    connect(action_zoom_reset, &QAction::triggered, this, [this] {
        // Reset to default font size (10pt)
        QFont font = editor->font();
        font.setPointSize(10);
        editor->setFont(font);
    });
}

void main_window::setup_status_bar()
{
    cursor_position_label = new QLabel("Line: 1, Col: 1", this);
    statusBar()->addPermanentWidget(cursor_position_label);

    connect(editor, &QTextEdit::cursorPositionChanged, this, [this] {
        update_cursor_position();
    });
}

void main_window::update_cursor_position()
{
    const QTextCursor cursor = editor->textCursor();
    const int line = cursor.blockNumber() + 1;
    const int col = cursor.columnNumber() + 1;
    cursor_position_label->setText(
        QString("Line: %1, Col: %2").arg(line).arg(col));
}
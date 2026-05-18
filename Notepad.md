# Simple Notepad

<img alt="Notepad" src="https://i.postimg.cc/g0rNY99t/Snimok-ekrana-2026-05-18-v-02-20-20.png"/>

---

## Description

**Student: Ramazan Busurmankulov**

Simple Notepad is a desktop text editor written in C++20 using the Qt6 framework.
It implements core notepad functionality with modern software engineering practices.
The editor supports file management, text transformation, spell checking, and document analysis.
The goal is to provide a clean, functional writing environment with real-time feedback.
The project features exception handling, a spell checker, word frequency analysis, and various formatting tools.

---

## Features

* **File Management**: The editor supports creating new documents, opening existing files, saving, and saving to a new location. File errors such as missing or unreadable files are caught and shown to the user as descriptive error dialogs.

* **Text Transforms**: The editor offers five case transformations — Uppercase, Lowercase, Capitalize Words, Sentence Case, and Swap Case — applied to selected text or the whole document.

* **Find & Replace**: The editor provides a Find / Replace dialog with case-sensitive search, Find Next, Replace, and Replace All functionality.

* **Spell Checker**: A 370,105-word dictionary is loaded at startup. Misspelled words are underlined in red in real time as the user types. Right-clicking a misspelled word shows up to 5 suggestions. Tools → Check Spelling forces a full re-pass over the document.

<img alt="Notepad" src="https://i.postimg.cc/mZny3HNY/Snimok-ekrana-2026-05-18-v-11-26-45.png"/>

* **Word Frequency**: The editor analyzes the document and displays a ranked table of every word by occurrence count.

---

## Additional Features

* **Print Option**: The current document can be printed directly from the File menu using the system print dialog.

* **Font Dialog**: The user can change the font family and size for selected text or the whole document through a font picker dialog.

* **Zoom**: The View menu provides Zoom In, Zoom Out, and Reset Zoom with keyboard shortcuts.

* **Cursor Line / Column Indicator**: A live Line and Column display in the status bar updates as the cursor moves through the document.

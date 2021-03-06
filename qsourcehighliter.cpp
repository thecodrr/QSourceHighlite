/*
 * Copyright (c) 2019 Waqar Ahmed -- <waqar.17a@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */
#include "qsourcehighliter.h"
#include "languagedata.h"

#include <QDebug>
#include <QTextDocument>

QSourceHighliter::QSourceHighliter(QTextDocument *doc)
    : QSyntaxHighlighter(doc)
{
    initFormats();
}

void QSourceHighliter::initFormats() {
    /****************************************
     * Formats for syntax highlighting
     ***************************************/

    QTextCharFormat format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    _formats[CodeBlock] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#F92672"));
    _formats[CodeKeyWord] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#a39b4e"));
    _formats[CodeString] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#75715E"));
    _formats[CodeComment] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#54aebf"));
    _formats[CodeType] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#db8744"));
    _formats[CodeOther] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#AE81FF"));
    _formats[CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#018a0f"));
    _formats[CodeBuiltIn] = format;
}

void QSourceHighliter::setCurrentLanguage(Language language) {
    if (language != !_language)
        _language = language;
}

QSourceHighliter::Language QSourceHighliter::currentLanguage() {
    return _language;
}

void QSourceHighliter::highlightBlock(const QString &text)
{
    if (currentBlock() == document()->firstBlock()) {
        setCurrentBlockState(_language);
    } else {
        previousBlockState() == _language ?
                    setCurrentBlockState(_language) :
                    setCurrentBlockState(_language + 1);
    }
    highlightSyntax(text);
}

/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void QSourceHighliter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    const auto textLen = text.length();

    QChar comment;
    bool isCSS = false;
    bool isYAML = false;

    QMultiHash<char, QLatin1String> keywords{};
    QMultiHash<char, QLatin1String> others{};
    QMultiHash<char, QLatin1String> types{};
    QMultiHash<char, QLatin1String> builtin{};
    QMultiHash<char, QLatin1String> literals{};

    QList<QLatin1String> wordList;

    switch (currentBlockState()) {
        case CodeCpp :
        case CodeCppComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case CodeJs :
        case CodeJsComment :
            loadJSData(types, keywords, builtin, literals, others);
            break;
        case CodeC :
        case CodeCComment :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case CodeBash :
            loadShellData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodePHP :
        case CodePHPComment :
            loadPHPData(types, keywords, builtin, literals, others);
            break;
        case CodeQML :
        case CodeQMLComment :
            loadQMLData(types, keywords, builtin, literals, others);
            break;
        case CodePython :
            loadPythonData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeRust :
        case CodeRustComment :
            loadRustData(types, keywords, builtin, literals, others);
            break;
        case CodeJava :
        case CodeJavaComment :
            loadJavaData(types, keywords, builtin, literals, others);
            break;
        case CodeCSharp :
        case CodeCSharpComment :
            loadCSharpData(types, keywords, builtin, literals, others);
            break;
        case CodeGo :
        case CodeGoComment :
            loadGoData(types, keywords, builtin, literals, others);
            break;
        case CodeV :
        case CodeVComment :
            loadVData(types, keywords, builtin, literals, others);
            break;
        case CodeSQL :
            loadSQLData(types, keywords, builtin, literals, others);
            break;
        case CodeJSON :
            loadJSONData(types, keywords, builtin, literals, others);
            break;
        case CodeXML :
            xmlHighlighter(text);
            return;
        case CodeCSS :
        case CodeCSSComment :
            isCSS = true;
            loadCSSData(types, keywords, builtin, literals, others);
            break;
        case CodeTypeScript:
        case CodeTypeScriptComment:
            loadTypescriptData(types, keywords, builtin, literals, others);
            break;
        case CodeYAML:
            isYAML = true;
            loadYAMLData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case CodeINI:
            comment = QLatin1Char('#');
            break;
    default:
        break;
    }

    // keep the default code block format
    // this statement is very slow
    // TODO: do this formatting when necessary instead of
    // applying it to the whole block in the beginning
    setFormat(0, textLen, _formats[CodeBlock]);

    auto applyCodeFormat = [this, &wordList](int i, const QMultiHash<char, QLatin1String> &data,
                        const QString &text, const QTextCharFormat &fmt) -> int {
        // check if we are at the beginning OR if this is the start of a word
        // AND the current char is present in the data structure
        if ( ( i == 0 || !text[i-1].isLetter()) && data.contains(text[i].toLatin1())) {
            wordList = data.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size())) {
                    //check if we are at the end of text OR if we have a complete word
                    if ( i + word.size() == text.length() ||
                         !text.at(i + word.size()).isLetter()) {
                        setFormat(i, word.size(), fmt);
                        i += word.size();
                    }
                }
            }
        }
        return i;
    };

    const QTextCharFormat &formatType = _formats[CodeType];
    const QTextCharFormat &formatKeyword = _formats[CodeKeyWord];
    const QTextCharFormat &formatComment = _formats[CodeComment];
    const QTextCharFormat &formatNumLit = _formats[CodeNumLiteral];
    const QTextCharFormat &formatBuiltIn = _formats[CodeBuiltIn];
    const QTextCharFormat &formatOther = _formats[CodeOther];

    for (int i=0; i< textLen; ++i) {

        if (currentBlockState() % 2 != 0) goto Comment;

        while (i < textLen && !text[i].isLetter()) {
            if (text[i].isSpace()) {
                ++i;
                //make sure we don't cross the bound
                if (i == textLen) return;
                if (text[i].isLetter()) break;
                else continue;
            }
            //inline comment
            if (comment.isNull() && text[i] == QLatin1Char('/')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                        Comment:
                        int next = text.indexOf(QLatin1String("*/"));
                        if (next == -1) {
                            //we didn't find a comment end.
                            //Check if we are already in a comment block
                            if (currentBlockState() % 2 == 0)
                                setCurrentBlockState(currentBlockState() + 1);
                            setFormat(i, textLen,  formatComment);
                            return;
                        } else {
                            //we found a comment end
                            //mark this block as code if it was previously comment
                            //first check if the comment ended on the same line
                            //if modulo 2 is not equal to zero, it means we are in a comment
                            //-1 will set this block's state as language
                            if (currentBlockState() % 2 != 0) {
                                setCurrentBlockState(currentBlockState() - 1);
                            }
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= textLen) return;
                        }
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, textLen, formatComment);
                i = textLen;
            //integer literal
            } else if (text[i].isNumber()) {
               i = highlightIntegerLiterals(text, i);
            //string literals
            } else if (text[i] == QLatin1Char('\"')) {
               i = highlightStringLiterals('\"', text, i);
            }  else if (text[i] == QLatin1Char('\'')) {
               i = highlightStringLiterals('\'', text, i);
            }
            if (i >= textLen) {
                break;
            }
            ++i;
        }

        int pos = i;

        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Types */
        i = applyCodeFormat(i, types, text, formatType);
        /************************************************
         next letter is usually a space, in that case
         going forward is useless, so continue;
         We can ++i here and go to the beginning of the next word
         so that the next formatter can check for formatting but this will
         cause problems in case the next word is also of 'Type' or the current
         type(keyword/builtin). We can work around it and reset the value of i
         in the beginning of the loop to the word's first letter but I am not
         sure about its efficiency yet.
         ************************************************/
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Keywords */
        i = applyCodeFormat(i, keywords, text, formatKeyword);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Literals (true/false/NULL,nullptr) */
        i = applyCodeFormat(i, literals, text, formatNumLit);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Builtin library stuff */
        i = applyCodeFormat(i, builtin, text, formatBuiltIn);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight other stuff (preprocessor etc.) */
        if (( i == 0 || !text[i-1].isLetter()) && others.contains(text[i].toLatin1())) {
            wordList = others.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size()).toLatin1()) {
                    if ( i + word.size() == textLen ||
                         !text.at(i + word.size()).isLetter()) {
                        currentBlockState() == CodeCpp ?
                        setFormat(i-1, word.size()+1, formatOther) :
                                    setFormat(i, word.size(), formatOther);
                        i += word.size();
                    }
                }
            }
        }

        //we were unable to find any match, lets skip this word
        if (pos == i) {
            int cnt = i;
            while (cnt < textLen) {
                if (!text[cnt].isLetter()) break;
                ++cnt;
            }
            i = cnt;
        }
    }

    if (isCSS) cssHighlighter(text);
    if (isYAML) ymlHighlighter(text);
}

/**
 * @brief Highlight string literals in code
 * @param strType str type i.e., ' or "
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the string
 */
int QSourceHighliter::highlightStringLiterals(QChar strType, const QString &text, int i) {
    setFormat(i, 1,  _formats[CodeString]);
    ++i;

    while (i < text.length()) {
        //make sure it's not an escape seq
        if (text.at(i) == strType && text.at(i-1) != '\\') {
            setFormat(i, 1,  _formats[CodeString]);
            ++i;
            break;
        }
        //look for escape sequence
        if (text.at(i) == '\\') {
            //look for space
            int spacePos = text.indexOf(' ', i);
            //if space not found, look for the string end
            //this may present problems in very special cases for e.g \"hello\"
            if (spacePos == -1) {
                spacePos = text.indexOf(strType, i);
            }
            setFormat(i, spacePos - i, _formats[CodeNumLiteral]);
            i = spacePos;
        }
        setFormat(i, 1,  _formats[CodeString]);
        ++i;
    }
    return i;
}

/**
 * @brief Highlight number literals in code
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the number
 */
int QSourceHighliter::highlightIntegerLiterals(const QString &text, int i)
{
    bool isPreNum = false;
    if (i == 0) isPreNum = true;
    else {
        switch(text[i - 1].toLatin1()) {
        case '[':
        case '(':
        case '{':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
            isPreNum = true;
            break;
        }
    }
    int start = i;

    if ((i+1) >= text.length()) {
        if (isPreNum) setFormat(i, 1, _formats[CodeNumLiteral]);
        return ++i;
    }

    ++i;
    //hex numbers highlighting (only if there's a preceding zero)
    if (text[i] == 'x' && text[i-1] == '0') ++i;

    if (isPreNum) {
        while (i < text.length()) {
            if (!text[i].isNumber() && text[i] != '.') break;
            ++i;
        }
    } else {
        return i;
    }

    i--;

    bool isPostNum = false;
    if (i+1 == text.length()) isPostNum = true;
    else {
        switch(text[i + 1].toLatin1()) {
        case ']':
        case ')':
        case '}':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>':
        case '<':
        case ';':
            isPostNum = true;
            break;
        case 'u':
        case 'l':
        case 'f':
        case 'U':
        case 'L':
        case 'F':
            isPostNum = true;
            ++i;
            break;
        }
    }
    if (isPostNum) {
        int end = ++i;
        setFormat(start, end - start, _formats[CodeNumLiteral]);
    }
    return i;
}

/**
 * @brief The YAML highlighter
 * @param text
 * @details This function post processes a line after the main syntax
 * highlighter has run for additional highlighting. It does these things
 *
 * 1. Highlight all the words that have a colon after them as 'keyword' except:
 * If the word is a string, skip it.
 * If the colon is in between a path, skip it (C:\)
 *
 * Once the colon is found, the function will skip every character except 'h'
 *
 * 2. If an h letter is found, check the next 4/5 letters for http/https and
 * highlight them as a link (underlined)
 */
void QSourceHighliter::ymlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    bool colonFound = false;

    for (int i = 0; i < textLen; ++i) {
        if (!text[i].isLetter()) continue;

        if (colonFound && text.at(i) != 'h') continue;

        //we found a string literal, skip it
        if (text.at(i-1) == '"') {
            int next = text.indexOf('"', i);
            i = next;
            continue;
        }

        if (text.at(i-1) == '\'') {
            int next = text.indexOf('"', i);
            i = next;
            continue;
        }


        int colon = text.indexOf(':', i);

        //if colon isn't found, we don't have anything more to do
        if (colon == -1) return;

        //colon is found, check if it isn't some path or something else
        if (!colonFound && (colon+1 < textLen) && !(text[colon+1] == '\\')) {
            colonFound = true;
            setFormat(i, colon - i, _formats[CodeKeyWord]);
        }

        //underlined links
        if (text[i] == 'h') {
            if (text.midRef(i, 5) == QStringLiteral("https") ||
                text.midRef(i, 4) == QStringLiteral("http")) {
                int space = text.indexOf(' ', i);
                if (space == -1) space = textLen;
                QTextCharFormat f = _formats[CodeString];
                f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(i, space - i, f);
            }
        }
    }
}

void QSourceHighliter::cssHighlighter(const QString &text)
{
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    for (int i = 0; i<textLen; ++i) {
        if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char('#')) {
            if (i+1 >= textLen) return;
            if (text[i + 1].isSpace() || text[i+1].isNumber()) continue;
            int space = text.indexOf(QLatin1Char(' '), i);
            if (space < 0) {
                space = text.indexOf('{');
                if (space < 0) {
                    space = textLen;
                }
            }
            setFormat(i, space - i, _formats[CodeKeyWord]);
            i = space;
        } else if (text[i] == QLatin1Char('c')) {
            if (text.midRef(i, 5) == QLatin1String("color")) {
                i += 5;
                int colon = text.indexOf(QLatin1Char(':'), i);
                if (colon < 0) continue;
                i = colon;
                i++;
                while(i < textLen) {
                    if (!text[i].isSpace()) break;
                    i++;
                }
                int semicolon = text.indexOf(QLatin1Char(';'));
                if (semicolon < 0) semicolon = textLen;
                QString color = text.mid(i, semicolon-i);
                QTextCharFormat f = _formats[CodeBlock];
                QColor c(color);
                if (color.startsWith(QLatin1String("rgb"))) {
                    int t = text.indexOf('(', i);
                    int rPos = text.indexOf(',', t);
                    int gPos = text.indexOf(',', rPos+1);
                    int bPos = text.indexOf(')', gPos);
                    if (rPos > -1 && gPos > -1 && bPos > -1) {
                        const QStringRef r = text.midRef(t+1, rPos - (t+1));
                        const QStringRef g = text.midRef(rPos+1, gPos - (rPos + 1));
                        const QStringRef b = text.midRef(gPos+1, bPos - (gPos+1));
                        c.setRgb(r.toInt(), g.toInt(), b.toInt());
                    } else {
                        c = _formats[CodeBlock].background().color();
                    }
                }

                if (!c.isValid()) {
                    continue;
                }

                int lightness{};
                QColor foreground;
                //really dark
                if (c.lightness() <= 20) {
                    foreground = Qt::white;
                } else if (c.lightness() > 20 && c.lightness() <= 51){
                    foreground = QColor("#ccc");
                } else if (c.lightness() > 51 && c.lightness() <= 78){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 78 && c.lightness() <= 110){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 127) {
                    lightness = c.lightness() + 100;
                    foreground = c.darker(lightness);
                }
                else {
                    lightness = c.lightness() + 100;
                    foreground = c.lighter(lightness);
                }

                f.setBackground(c);
                f.setForeground(foreground);
                setFormat(i, semicolon - i, QTextCharFormat()); //clear prev format
                setFormat(i, semicolon - i, f);
                i = semicolon;
            }
        }
    }
}


void QSourceHighliter::xmlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    setFormat(0, textLen, _formats[CodeBlock]);

    for (int i = 0; i < textLen; ++i) {
        if (text[i] == QLatin1Char('<') && text[i+1] != QLatin1Char('!')) {

            int found = text.indexOf(QLatin1Char('>'), i);
            if (found > 0) {
                ++i;
                if (text[i] == QLatin1Char('/')) ++i;
                setFormat(i, found - i, _formats[CodeKeyWord]);
            }
        }

        if (text[i] == QLatin1Char('=')) {
            int lastSpace = text.lastIndexOf(QLatin1Char(' '), i);
            if (lastSpace == i-1) lastSpace = text.lastIndexOf(QLatin1Char(' '), i-2);
            if (lastSpace > 0) {
                setFormat(lastSpace, i - lastSpace, _formats[CodeBuiltIn]);
            }
        }

        if (text[i] == QLatin1Char('\"')) {
            int pos = i;
            int cnt = 1;
            ++i;
            //bound check
            if ( (i+1) >= textLen) return;
            while (i < textLen) {
                if (text[i] == QLatin1Char('\"')) {
                    ++cnt;
                    ++i;
                    break;
                }
                ++i; ++cnt;
                //bound check
                if ( (i+1) >= textLen) {
                    ++cnt;
                    break;
                }
            }
            setFormat(pos, cnt, _formats[CodeString]);
        }
    }
}

#ifndef CODEEDITOR
#define CODEEDITOR

#include <QPlainTextEdit>
#include <QPainter>

#include "CodeHighlighter.hpp"

class LineHighlighting;


/**
 * @brief The CodeEditor class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QPlainTextEdit that is optimized for code;
 * with Syntax Highlighting, line numbers and highlighting of
 * the current line.
 */
class CodeEditor : public QPlainTextEdit{
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = 0);
    void lineHighlightingPaintEvent(QPaintEvent *event) noexcept;
    int lineHighlightingWidth() noexcept;
    void highlightErroredLine(int) noexcept;
    void setHighlighting(int highlighting) noexcept;

protected:
    void resizeEvent(QResizeEvent *event) noexcept;
    void keyPressEvent(QKeyEvent *e) noexcept;

private Q_SLOTS:
    void updatelineHighlightingWidth() noexcept;
    void highlightCurrentLine() noexcept;
    void updatelineHighlighting(const QRect &, int) noexcept;

private:
    QWidget *lineHighlighting;
    CodeHighlighter *syntaxEngine;
};



/**
 * @brief The LineHighlighting helper class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A helping subclass of QWidget for the codeEditor that is
 * responsible for highlighting the current line.
 */
class LineHighlighting : public QWidget{
public:
    LineHighlighting(CodeEditor *editor) : QWidget(editor){
        codeEditor = editor;
    }

    virtual QSize sizeHint() const noexcept{
        return QSize(codeEditor->lineHighlightingWidth(), 0);
    }

 protected:
    virtual void paintEvent(QPaintEvent *event) noexcept{
        codeEditor->lineHighlightingPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;

};


#endif

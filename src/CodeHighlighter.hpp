#ifndef CODEHIGHLIGHTER
#define CODEHIGHLIGHTER

#include <QSyntaxHighlighter>
#include <QTextStream>
#include <QFileInfo>

/**
 * @brief The CodeHighlighter class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QSyntaxHighlighter that implements Syntax
 * Highlighting(duh!) for the Code Editor class.
 */
class CodeHighlighter : public QSyntaxHighlighter{
    Q_OBJECT

public:
    CodeHighlighter(QTextDocument *parent = 0);
    void setupHighlighting();

protected:
    void highlightBlock(const QString &text);

private:
    CodeHighlighter& operator=(const CodeHighlighter& rhs);
    CodeHighlighter& operator=(CodeHighlighter&& rhs);

    struct Rule{
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<Rule> Rules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat multiLineCommentNotation;
};

#endif

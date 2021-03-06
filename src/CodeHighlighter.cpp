#include "CodeHighlighter.hpp"

/**
 * @brief CodeHighlighter::CodeHighlighter
 * @param parent
 * @param file
 *
 * The constructor of the syntax highlighter.
 * Needs a highlighting file.
 */
CodeHighlighter::CodeHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent){
    setupHighlighting();
}

/**
 * @brief CodeHighlighter::setupHighlighting
 * @param file
 *
 * Sets up the highlighting rules.
 * TODO: Make it more flexible - File format?
 * TODO: Highlight bracket-pairs
 * TODO: Build quot-pairs ( "bla " bla " bla " -> all is highlighted)
 * TODO: don't let the multi line comment trust preview format (see function highlightBlock(...) )
 */
void CodeHighlighter::setupHighlighting() noexcept{
    QFile highlighting;
    highlighting.setFileName(":/rc/highlighting/glsl");
    highlighting.open(QFile::ReadOnly | QFile::Text);
    Rule rule;
    Rules.clear();

    // Keyword
    rule.format.setForeground(Qt::blue); // rule.format already freshly initialized
//    rule.format.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    auto keywords = QTextStream(&highlighting).readAll();
    for(auto in: keywords.split("\n")){
        if(in != "")
            keywordPatterns << in;
    }
    for(auto &pattern: keywordPatterns){
        rule.pattern = QRegExp(pattern);
        Rules.append(rule);
    }

    // Branch
    rule.format = QTextCharFormat();
    rule.format.setForeground(Qt::blue);

    rule.pattern = QRegExp("\\b(for(each)?|while|if|else)\\s*(?=\\()");
    Rules.append(rule);

    rule.pattern = QRegExp("\\bdo\\s*(?=\\{)");
    Rules.append(rule);


    // Digits
    rule.format = QTextCharFormat();
    rule.format.setForeground(QColor(255, 128, 0));
    rule.pattern = QRegExp("\\b[0-9]+|[0-9]*\\.[0-9]+\\b");
    Rules.append(rule);

    auto filename = QFileInfo(highlighting.fileName()).fileName();

    // Single line comment
    rule.format = QTextCharFormat();
    rule.format.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("//.*$");
    Rules.append(rule);

    // Multi line comment
    // rule.format is already comment
    multiLineCommentNotation = rule.format; // needed for later highlight
    //work in progress; make for more elaborate matching
    //commentStartExpression = QRegExp("[^/(//[^\\r\\n]+)]?/\\*");
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

    // Directive
    rule.format = QTextCharFormat();
    rule.format.setForeground(Qt::darkMagenta);
    //highlight the whole line provided theres a # at the beginning
    rule.pattern = QRegExp("^\\s*#.*$");
    Rules.append(rule);

    // GL
    rule.format = QTextCharFormat();
    //rule.format.setFontWeight(QFont::Bold);
    rule.format.setForeground(Qt::darkCyan);
    rule.pattern = QRegExp("\\b(GL|gl)(_)?[A-Za-z]+\\b");
    Rules.append(rule);

    // Quotation
    rule.format = QTextCharFormat();
    rule.format.setFontItalic(true);
    rule.format.setForeground(Qt::darkRed);
    rule.pattern = QRegExp("\"(\\\"|.)*\"");
    Rules.append(rule);
}

/**
 * @brief CodeHighlighter::highlightBlock
 * @param text
 *
 * Highlights blocks(duh) by evaluating the regexes
 * created in the constructor.
 * TODO: Code and time complexity are suboptimal.
 *       Is there a better way?
 */
void CodeHighlighter::highlightBlock(const QString &text) noexcept{
    /*
     * goes through the text once for each rule and
     * highlights the code according to those
     */
    for(auto &rule: Rules){
        QRegExp regEx(rule.pattern);
        auto i = regEx.indexIn(text);
        while(i >= 0){
            auto length = regEx.matchedLength();
            setFormat(i, length, rule.format);
            i = regEx.indexIn(text, i + length);
        }
    }
    setCurrentBlockState(0);

   /*
    * goes through the text once more for the slightly
    * more complex multiline comment rule: searches for
    * a start of the comment and the accoring end and
    * highlights them and the text between.
    *
    * MAINTAINABILITY NOTES:
    * Requires keeping track of three indices:
    *      si = start index
    *      ei = end index
    *      commentLen = length of the comment(i.e. offset ; needed?)
    */
    auto si = 0;
    if(previousBlockState() != 1)
        si = commentStartExpression.indexIn(text);

    // TODO: don't trust the preview format
    while(si >= 0 && this->format(si) != multiLineCommentNotation){
        auto ei = commentEndExpression.indexIn(text, si);
        int commentLen;
        if(ei == -1){
            setCurrentBlockState(1);
            commentLen = text.length() - si;
        }else
            commentLen = ei - si + commentEndExpression.matchedLength();

        setFormat(si, commentLen, multiLineCommentNotation);
        si = commentStartExpression.indexIn(text, si + commentLen);
    }

}

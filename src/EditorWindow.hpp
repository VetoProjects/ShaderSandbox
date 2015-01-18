#ifndef EDITORWINDOW
#define EDITORWINDOW

#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QInputDialog>

#include "CodeEditor.hpp"
#include "ObjectLoaderDialog.hpp"

/**
 * @brief The EditorWindow class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QMainWindow that makes the CodeEditor more
 * interactive by implementing save/load and open/close features.
 */
class EditorWindow : public QMainWindow{
    Q_OBJECT

public:
    EditorWindow(const QHash<QString, QVariant> &settings, QWidget *parent = 0);
    ~EditorWindow();
    void showResults(const QString &);
    void warningDisplay(const QString &);
    void highlightErroredLine(int);
    void codeStopped();

    QString getVertexSourceCode() const;
    QString getFragmentSourceCode() const;
    QString getTitle() const;

protected:
    virtual void closeEvent(QCloseEvent *);

private Q_SLOTS:
    void newFile();
    void openFile();

    void docModified();
    void runFile();
    bool saveFile();

    void gotOpenHelp();
    void gotOpenSettings();
    void gotCloseAll();

Q_SIGNALS:
    void closing(EditorWindow *);
    void closeAll(EditorWindow *);
    void openSettings(EditorWindow *);
    void openHelp(EditorWindow *);
    void runCode(EditorWindow *);
    void stopCode(EditorWindow *);
    void titleChanged(EditorWindow *);
    void changedSetting(EditorWindow *, const QString &, const QVariant &);
    void changedSettings(EditorWindow *, const QHash<QString, QVariant> &);
    void loadModel(const QString &, const QVector3D &, const QVector3D &, const QVector3D &);

private:
    EditorWindow& operator=(const EditorWindow& rhs);
    EditorWindow& operator=(EditorWindow&& rhs);

    void addActions();
    void addMenus();
    void addToolBars();
    void addStatusBar();
    void applySettings(const QHash<QString, QVariant> &);
    void loadFile(const QString &, bool v = false, bool f = false);
    bool saveDialog();
    void saveSettings();
    void setAsCurrentFile(const QString &vertexFile, const QString &fragmentFile);
    QString stripName(const QString &);

    int templateNum;

    QString vertexName, fragmentName;

    QSplitter *codeEditors;
    CodeEditor *vertexCodeEditor, *fragmentCodeEditor;
    QString currentVertexFile, currentFragmentFile;

    QMenu *fMenu;
    QMenu *eMenu;
    QMenu *hMenu;
    QToolBar *fileBar;
    QToolBar *editBar;
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *exitAction;
    QAction *runAction;
    QAction *settingsAction;
    QAction *helpAction;
    QAction *loadObjectAction;

    QString modelFile;
    QVector3D modelOffset, modelScaling, modelRotation;
    ObjectLoaderDialog *objectLoaderDialog;
};

#endif // EDITORWINDOW

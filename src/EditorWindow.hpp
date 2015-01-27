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
    void showResults(const QString &) noexcept;
    void warningDisplay(const QString &) noexcept;
    void highlightErroredVertexLine(int) noexcept;
    void highlightErroredFragmentLine(int) noexcept;
    void codeStopped() noexcept;

    QString getVertexSourceCode() const noexcept;
    QString getFragmentSourceCode() const noexcept;
    QString getTitle() const noexcept;

protected:
    virtual void closeEvent(QCloseEvent *) noexcept;

private Q_SLOTS:
    void newFile() noexcept;
    void openFile() noexcept;

    void docModified() noexcept;
    void runFile() noexcept;
    bool saveFile() noexcept;

    void gotOpenHelp() noexcept;
    void gotOpenSettings() noexcept;
    void gotCloseAll() noexcept;

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
    EditorWindow& operator=(const EditorWindow& rhs) noexcept;
    EditorWindow& operator=(EditorWindow&& rhs) noexcept;

    void addActions() noexcept;
    void addMenus() noexcept;
    void addToolBars() noexcept;
    void addStatusBar() noexcept;
    void applySettings(const QHash<QString, QVariant> &) noexcept;
    void loadFile(const QString &, bool v = false, bool f = false) noexcept;
    bool saveDialog() noexcept;
    bool saveFile(QString shaderType) noexcept;
    void saveSettings() noexcept;
    void setAsCurrentFile(const QString &vertexFile, const QString &fragmentFile) noexcept;
    QString stripName(const QString &) noexcept;

    int templateNum;

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

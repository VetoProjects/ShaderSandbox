#include "WindowInstance.hpp"

namespace Instances {

/**
 * @brief WindowInstance::WindowInstance
 * @param id
 * @param settings
 * @param parent
 *
 * Constructor of the delegate.
 */
WindowInstance::WindowInstance(int id, const QHash<QString, QVariant> &settings, QObject *parent) : IInstance(id, parent)
{
    window = 0;
    createWindow(settings);
    objectLoaderDialog = new ObjectLoaderDialog();

    connect(objectLoaderDialog, &ObjectLoaderDialog::objectInfo,
        [this](const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation) {
            this->loadModel(this, file, offset, scaling, rotation);
        }
    );

    showObjectLoaderDialog();
}

/**
 * @brief WindowInstance::~WindowInstance
 *
 * Destructor of the delegate.
 */
WindowInstance::~WindowInstance()
{
    delete objectLoaderDialog;
    delete window;
}


/**
 * @brief WindowInstance::sourceCode
 * @return the editor contents as string
 *
 * Gets the Editor Content as a QString.
 */
QString WindowInstance::sourceCode() const
{
    return window->getSourceCode();
}

/**
 * @brief WindowInstance::title
 * @return file name as a QString
 *
 * Gets the file name as a QString.
 */
QString WindowInstance::title() const
{
    return window->getTitle();
}

/**
 * @brief WindowInstance::reportError
 * @param message
 *
 * Displays error message in editor.
 */
void WindowInstance::reportError(const QString &message)
{
    window->warningDisplay(message);
}

/**
 * @brief WindowInstance::reportWarning
 * @param text
 *
 * Displays warning message in editor.
 */
void WindowInstance::reportWarning(const QString &text)
{
    window->showResults(text);
}

/**
 * @brief WindowInstance::highlightErroredLine
 * @param lineno
 *
 * Highlight line number lineno as errored.
 */
void WindowInstance::highlightErroredLine(int lineno)
{
    window->highlightErroredLine(lineno);
}

/**
 * @brief WindowInstance::codeStopped
 *
 * Signals that execution of the editor code has
 * finsihed.
 */
void WindowInstance::codeStopped()
{
    window->codeStopped();
}

/**
 * @brief WindowInstance::close
 * @return whether closing has succeeded
 *
 * Closes the editor window and return a success flag.
 */
bool WindowInstance::close()
{
    disconnect(window, SIGNAL(closing(EditorWindow*)), this, SLOT(gotClosing(EditorWindow*)));
    if(window->close()){
        disconnect(window, SIGNAL(destroyed(QObject*)), this, SLOT(gotDestroying(QObject*))); // Prevent of call gotDestroying
        window->deleteLater();
        window = 0;
        return true;
    }
    connect(window, SIGNAL(closing(EditorWindow*)), this, SLOT(gotClosing(EditorWindow*)));
    return false;
}

/**
 * @brief WindowInstance::gotClosing
 *
 * Signals that the editor requested closing.
 */
void WindowInstance::gotClosing(EditorWindow *)
{
    Q_EMIT closing(this);
}

/**
 * @brief WindowInstance::gotCloseAll
 *
 * Signals that the editor requested closing of all windows.
 */
void WindowInstance::gotCloseAll(EditorWindow *)
{
    Q_EMIT closeAll();
}

/**
 * @brief WindowInstance::gotOpenHelp
 *
 * Signals that the editor requested a help window.
 */
void WindowInstance::gotOpenHelp(EditorWindow *)
{
    Q_EMIT openHelp(this);
}

/**
 * @brief WindowInstance::gotOpenSettings
 *
 * Signals that the editor requested a settings window.
 */
void WindowInstance::gotOpenSettings(EditorWindow *)
{
    Q_EMIT openSettings(this);
}

/**
 * @brief WindowInstance::gotChangedSettings
 * @param settings
 *
 * Signals that the editors' settings have changed(completely).
 */
void WindowInstance::gotChangedSettings(EditorWindow *, const QHash<QString, QVariant> &settings)
{
    Q_EMIT changeSettings(this, settings);
}

/**
 * @brief WindowInstance::gotChangedSetting
 * @param key
 * @param value
 *
 * Signals that the editors' settings have changed(singla value).
 */
void WindowInstance::gotChangedSetting(EditorWindow *, const QString &key, const QVariant &value)
{
    Q_EMIT changeSetting(this, key, value);
}

void WindowInstance::gotDestroying(QObject*)
{
    window = 0;
    createWindow();
}

/**
 * @brief WindowInstance::gotRunCode
 *
 * Signals that the editor requested a run.
 */
void WindowInstance::gotRunCode(EditorWindow *)
{
    Q_EMIT runCode(this);
}

/**
 * @brief WindowInstance::gotStopCode
 *
 * Signals that the editor requested stop of execution.
 */
void WindowInstance::gotStopCode(EditorWindow *)
{
    Q_EMIT stopCode(this);
}

/**
 * @brief WindowInstance::createWindow
 * @param settings
 *
 * Creates a window with settings.
 */
void WindowInstance::createWindow(const QHash<QString,QVariant> &settings)
{
    if(!window){
        window = new EditorWindow(settings);
        connect(window, SIGNAL(destroyed(QObject*))        , this, SLOT(gotDestroying(QObject*)));
        connect(window, SIGNAL(closing(EditorWindow*))     , this, SLOT(gotClosing(EditorWindow*)));
        connect(window, SIGNAL(closeAll(EditorWindow*))    , this, SLOT(gotCloseAll(EditorWindow*)));
        connect(window, SIGNAL(runCode(EditorWindow*))     , this, SLOT(gotRunCode(EditorWindow*)));
        connect(window, SIGNAL(stopCode(EditorWindow*))    , this, SLOT(gotStopCode(EditorWindow*)));
        connect(window, SIGNAL(openHelp(EditorWindow*))    , this, SLOT(gotOpenHelp(EditorWindow*)));
        connect(window, SIGNAL(openSettings(EditorWindow*)), this, SLOT(gotOpenSettings(EditorWindow*)));
        connect(window, SIGNAL(changedSetting(EditorWindow*,QString,QVariant)),         this, SLOT(gotChangedSetting(EditorWindow*,QString,QVariant)));
        connect(window, SIGNAL(changedSettings(EditorWindow*,QHash<QString,QVariant>)), this, SLOT(gotChangedSettings(EditorWindow*,QHash<QString,QVariant>)));
        window->show();
    }
}

bool WindowInstance::showObjectLoaderDialog()
{
    objectLoaderDialog->show();
}

/**
 * @brief WindowInstance::createWindow
 *
 * Creates a window without settings.
 */
void WindowInstance::createWindow()
{
    if(!window){
        QHash<QString, QVariant> settings;
        Q_EMIT getSettings(this, settings);
        createWindow(settings);
    }
}

}

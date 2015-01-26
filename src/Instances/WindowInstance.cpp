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
}

/**
 * @brief WindowInstance::~WindowInstance
 *
 * Destructor of the delegate.
 */
WindowInstance::~WindowInstance()
{
    delete window;
}


/**
 * @brief WindowInstance::sourceCode
 * @return the editor contents as string
 *
 * Gets the Editor Content as a QString.
 */
QString WindowInstance::vertexSourceCode() const
{
    return window->getVertexSourceCode();
}

QString WindowInstance::fragmentSourceCode() const
{
    return window->getFragmentSourceCode();
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
    disconnect(window, &EditorWindow::closing, this, &WindowInstance::gotClosing);
    if(window->close()){
        disconnect(window, &EditorWindow::destroyed, this, &WindowInstance::gotDestroying); // Prevent of call gotDestroying
        window->deleteLater();
        window = 0;
        return true;
    }
    connect(window, &EditorWindow::closing, this, &WindowInstance::gotClosing);
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
        connect(window, &EditorWindow::destroyed      , this, &WindowInstance::gotDestroying);
        connect(window, &EditorWindow::closing        , this, &WindowInstance::gotClosing);
        connect(window, &EditorWindow::closeAll       , this, &WindowInstance::gotCloseAll);
        connect(window, &EditorWindow::runCode        , this, &WindowInstance::gotRunCode);
        connect(window, &EditorWindow::stopCode       , this, &WindowInstance::gotStopCode);
        connect(window, &EditorWindow::openHelp       , this, &WindowInstance::gotOpenHelp);
        connect(window, &EditorWindow::openSettings   , this, &WindowInstance::gotOpenSettings);
        connect(window, &EditorWindow::changedSetting , this, &WindowInstance::gotChangedSetting);
        connect(window, &EditorWindow::changedSettings, this, &WindowInstance::gotChangedSettings);
        connect(window, &EditorWindow::loadModel, [=](const QString &file, const QVector3D &offset,
                                                      const QVector3D &scaling, const QVector3D &rotation){
            Q_EMIT loadModel(this, file, offset, scaling, rotation);
        });
        window->show();
    }
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

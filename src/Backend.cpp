#include "Backend.hpp"

// TODO:
// - Think about all

using namespace Instances;

/**
 * @brief Backend::Backend
 * @param parent
 *
 * The constructor of the Backend class.
 * Initializes the editor window list and the thread list as well
 * as the settings backend.
 */
Backend::Backend(QObject *parent) : QObject(parent){
    QApplication::setStyle(SettingsBackend::getSettingsFor("Design", "").toString());
}

/**
 * @brief Backend::~Backend
 *
 * The destructor of the Backend class.
 * Eliminates all the threads that were orphaned
 * when all the windows closed.
 */
Backend::~Backend(){
    for(auto &thread : threads.values()){
        if(thread){
            if(thread->isRunning())
                thread->terminate();
            thread.reset();
        }
    }
}

/**
 * @brief Backend::addChild
 * @param instance
 * @param removeSettings
 *
 * Is called by one of the editor window instances;
 * enlists the child and creates an empty thread entry
 * in the list so that the two correlate.
 */
void Backend::addInstance(IInstance *instance, bool removeSettings) noexcept{
    int id = instance->ID;
    if(instances.contains(id))
        return;
    if(removeSettings)
        SettingsBackend::removeSettings(id);
    instances.insert(id, std::shared_ptr<IInstance>(instance));
    connect(instance, &IInstance::closing,        this, &Backend::instanceClosing);
    connect(instance, &IInstance::destroyed,      this, &Backend::instanceDestroyed);
    connect(instance, &IInstance::runCode,        this, &Backend::instanceRunCode);
    connect(instance, &IInstance::stopCode,       this, &Backend::instanceStopCode);
    connect(instance, &IInstance::changeSetting,  this, &Backend::instanceChangedSetting);
    connect(instance, &IInstance::getSetting,     this, &Backend::instanceRequestSetting);
    connect(instance, &IInstance::changeSettings, this, &Backend::instanceChangedSettings);
    connect(instance, &IInstance::getSettings,    this, &Backend::instanceRequestSettings);
    connect(instance, &IInstance::closeAll,       this, &Backend::childSaidCloseAll);
    connect(instance, &IInstance::openSettings,   this, &Backend::settingsWindowRequested);
    connect(instance, &IInstance::openHelp,       this, &Backend::openHelp);
    connect(instance, &IInstance::loadModel,      this, &Backend::instanceLoadModel);
    ids.append(id);
    saveIDs();
}

/**
 * @brief Backend::nextID
 * @return Free to use id
 *
 * Look up the first free ID for a new Instance.
 */
int Backend::nextID() noexcept{
    int id = 0;
    while(ids.contains(id))
        ++id;
    return id;
}

/**
 * @brief Backend::loadIds
 * @return A list of used IDs
 *
 * Return the list of ids for which settings
 * should exist.
 */
QList<int> Backend::loadIds() noexcept
{
    QVariantList ids = SettingsBackend::getSettingsFor(QStringLiteral("Instances"), QVariantList()).toList();
    QList<int> res;
    for(auto id : ids){
        bool ok;
        int i = id.toInt(&ok);
        if(ok)
            res.append(i);
    }
    return res;
}

/**
 * @brief Backend::instanceClosing
 * @param instance
 *
 * Reacts to the closing signal and calls the
 * removeInstance() routine.
 * TODO: Needed?
 */
void Backend::instanceClosing(IInstance *instance) noexcept
{
    removeInstance(instance);
}

/**
 * @brief Backend::instanceDestroyed
 * @param instance
 *
 * Reacts to the destroyed signal and removes
 * the instance from the backends' memory.
 */
void Backend::instanceDestroyed(QObject *instance) noexcept
{
    int id = ((IInstance*)instance)->ID;
    instances.remove(id);
    removeInstance(id, false);
}

/**
 * @brief Backend::removeChild
 * @param child
 *
 * Is called by one of the editor window instances;
 * removes the child from the list and closes the thread.
 * Removes all the settings that belong to the current child.
 * BUG: When the settings of the next children are updated,
 *      the settings window will display the settings of the
 *      killed child. This will result in confusion of the user.
 * TODO: Fix bug!
 */
bool Backend::removeInstance(Instances::IInstance *instance, bool removeSettings) noexcept{
    return removeInstance(instance->ID, removeSettings);
}

bool Backend::removeInstance(int id, bool removeSettings) noexcept{
    if(instances.contains(id)){
        disconnect(instances[id].get(), &IInstance::destroyed, this, &Backend::instanceDestroyed);
        if(!instances[id]->close()){
            connect(instances[id].get(), &IInstance::destroyed, this, &Backend::instanceDestroyed);
            return false;
        }
        instances[id]->deleteLater();
        instances.remove(id);
    }
    terminateThread(id);
    if(removeSettings && ids.size() > 1){
        SettingsBackend::removeSettings(id);
        ids.removeOne(id);
        saveIDs();
    }
    return true;
}

/**
 * @brief Backend::childSaidCloseAll
 *
 * Is called by one of the editor window instances;
 * when a user requests to exit the application, this
 * will tell all the children to terminate.
 */
void Backend::childSaidCloseAll() noexcept{
    QList<int> notRemoved = ids;
    for(auto id : ids){
        disconnect(instances[id].get(), &IInstance::destroyed, this, &Backend::instanceDestroyed);
        if(removeInstance(id, false))
            notRemoved.removeOne(id);
    }
    if(!notRemoved.empty()){
        for(auto id : ids)
            if(!notRemoved.contains(id))
               SettingsBackend::removeSettings(id);
        ids = notRemoved;
    }
    saveIDs();
}

/**
 * @brief Backend::childExited
 * @param child
 * @param file
 *
 * Is called by one of the editor window instances;
 * when the child reacts to the closedAction, it is removed
 * from the list.
 */
void Backend::childExited(IInstance *child, QString file) noexcept{
    Q_UNUSED(child);
    Q_UNUSED(file);
//    saveSettings(child, file);
//    children.removeOne(child);
}

/**
 * @brief Backend::getSettings
 * @param instance
 * @return a list of current settings
 *
 * Gets all settings for a specific window.
 */
QHash<QString, QVariant> Backend::getSettings(IInstance* instance) noexcept
{
    return getSettings(instance->ID);
}

/**
 * @brief Backend::getSettings
 * @param id
 * @return a QHash of all the settings for an id.
 *
 * looks up the settings for an editor window child.
 */
QHash<QString, QVariant> Backend::getSettings(int id) noexcept
{
    return SettingsBackend::getSettings(id);
}

/**
 * @brief Backend::settingsWindowRequested
 * @param instance
 *
 * Creates a settings window instance.
 */
void Backend::settingsWindowRequested(IInstance *instance) noexcept{
    SettingsWindow settingsWin(instance->ID);
    settingsWin.exec();
}

/**
 * @brief Backend::openHelp
 *
 * Opens a help window in HTML.
 */
void Backend::openHelp(IInstance *) noexcept{
    QUrl url(directoryOf("rc").absoluteFilePath("help.html"));
    url.setScheme("file");
    QDesktopServices::openUrl(url);
}

/**
 * @brief Backend::directoryOf
 * @param subdir
 * @return the directory one wants to navigate into
 *
 * Platform independent wrapper to changing the directory.
 */
QDir Backend::directoryOf(const QString &subdir) noexcept{
    QDir dir(QApplication::applicationDirPath());

#if defined(Q_OS_MAC)
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
#elif defined(Q_OS_WIN)
    dir.cdUp();
#endif
    if(dir.cd(subdir))
        return dir;
    else
        return QDir(dir.absolutePath() + subdir);
}

/**
 * @brief Backend::removeSettings
 * @param instance
 *
 * removes the settings for a specific file.
 */
void Backend::removeSettings(IInstance* instance) noexcept{
    SettingsBackend::removeSettings(instance->ID);
}

void Backend::removeSettings(int id) noexcept{
    SettingsBackend::removeSettings(id);
}

/**
 * @brief Backend::isLast
 * @return true if the child is the last one, false otherwise
 *
 * checks whether there is only one or no child in the list.
 */
bool Backend::isLast() noexcept{
    return ids.length() < 2;
}

/**
 * @brief Backend::instanceRunCode
 *
 * reacts to the run SIGNAL by running the code(duh) that is
 * in the editor at the moment.
 */
void Backend::instanceRunCode(IInstance *instance) noexcept
{
    long id = instance->ID;
    if(threads.contains(id)){
        bool worked = threads[id]->updateCode(instance->vertexSourceCode(), instance->fragmentSourceCode());
        if(!worked){
            instances[id]->reportError(tr("Code is faulty."));
        }
    }else{
        runGlFile(instance);
    }
}

/**
 * @brief Backend::instanceStopCode
 * @param instance
 *
 * Reacts to the stopCode signal of an instance.
 * Stops the executing context.
 */
void Backend::instanceStopCode(IInstance *instance) noexcept
{
    terminateThread(instance->ID);
}

/**
 * @brief Backend::instanceChangedSetting
 * @param instance
 * @param key
 * @param value
 *
 * Reacts to the instance changing settings.
 * Saves the new settings.
 */
void Backend::instanceChangedSetting(IInstance *instance, const QString &key,
                                     const QVariant &value) noexcept
{
    SettingsBackend::saveSettingsFor(instance->ID, key, value);
}

/**
 * @brief Backend::instanceRequestSetting
 * @param instance
 * @param key
 * @param value
 *
 * Reacts to the instance requesting its settings for
 * a given key.
 */
void Backend::instanceRequestSetting(IInstance *instance, const QString &key,
                                     QVariant &value) noexcept
{
    value = SettingsBackend::getSettingsFor(key, value, instance->ID);
}

QVariant Backend::getSetting(QString key, QVariant defaultValue) noexcept{
    return SettingsBackend::getSettingsFor(key, defaultValue);
}

/**
 * @brief Backend::instanceChangedSettings
 * @param instance
 * @param set
 *
 * Reacts to the instance changing its settings(as a set).
 * Saves the new settings.
 * TODO: Needed(overloaded call)?
 */
void Backend::instanceChangedSettings(IInstance *instance, const QHash<QString,
                                      QVariant> &set) noexcept
{
    SettingsBackend::saveSettingsFor(instance->ID, set);
}

/**
 * @brief Backend::instanceRequestSettings
 * @param instance
 * @param set
 *
 * TODO: Needed(overloaded call)?
 */
void Backend::instanceRequestSettings(IInstance *instance, QHash<QString,
                                      QVariant> &set) noexcept
{
    set = SettingsBackend::getSettings(instance->ID);
}

/**
 * @brief Backend::instanceLoadModel
 * @param instance
 * @param file
 * @param offset
 * @param scaling
 * @param rotation
 *
 * Triggers the renderer to load a model
 */
void Backend::instanceLoadModel(IInstance *instance, const QString &file, const QVector3D &offset,
                                const QVector3D &scaling, const QVector3D &rotation) noexcept
{
    if(threads.contains(instance->ID))
        threads[instance->ID]->loadModel(file, offset, scaling, rotation);
}

/**
 * @brief Backend::runGlFile
 * @param filename
 * @param instructions
 * @param index
 *
 * Creates a thread that executes GL source code.
 */
void Backend::runGlFile(IInstance *instance) noexcept{
    std::shared_ptr<GlLiveThread> thread(new GlLiveThread(instance->ID, this));
    connect(thread.get(), &GlLiveThread::doneSignal,    this, &Backend::getExecutionResults);
    connect(thread.get(), &GlLiveThread::errorSignal,   this, &Backend::getError);
    connect(thread.get(), &GlLiveThread::vertexError,   this, &Backend::getVertexError);
    connect(thread.get(), &GlLiveThread::fragmentError, this, &Backend::getFragmentError);
    thread->initialize(instance->vertexSourceCode(), instance->fragmentSourceCode());
    thread->start();
    threads.insert(thread->ID, thread);
}

/**
 * @brief Backend::getExecutionResults
 *
 * reacts to the done SIGNAL by terminating the thread and
 * emitting a showResults SIGNAL for the QWidgets to display
 */
void Backend::getExecutionResults(GlLiveThread* thread, QString returnedException) noexcept{
    // Already gone?
    disconnect(thread, &GlLiveThread::doneSignal, this, &Backend::getExecutionResults);
    instances[thread->ID]->reportWarning(returnedException);
    terminateThread(thread->ID);
}

void Backend::getError(GlLiveThread* thread, QString error) noexcept{
    instances[thread->ID]->reportWarning(error);
}

void Backend::getVertexError(GlLiveThread* thread, QString error, int line) noexcept{
    instances[thread->ID]->reportWarning(error);
    if(line >= 0)
        instances[thread->ID]->highlightErroredVertexLine(line);
}

void Backend::getFragmentError(GlLiveThread* thread, QString error, int line) noexcept{
    instances[thread->ID]->reportWarning(error);
    if(line >= 0)
        instances[thread->ID]->highlightErroredFragmentLine(line);
}

/**
 * @brief Backend::terminateThread
 * @param thread
 *
 * terminates a specific thread and deletes it from the list.
 */
void Backend::terminateThread(long id) noexcept{
    if(threads.contains(id)){
        if(threads[id]->isRunning())
            threads[id]->terminate();
        threads[id]->wait();
        threads[id]->deleteLater();
        threads.remove(id);
    }

}

/**
 * @brief Backend::saveIDs
 *
 * saves all the IDs in the settings.
 */
void Backend::saveIDs() noexcept{
    QVariantList vids;
    for(auto i : ids)
        vids.append(i);
    SettingsBackend::addSettings("Instances", vids);
}

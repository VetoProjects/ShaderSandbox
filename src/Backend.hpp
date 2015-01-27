#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <QDesktopServices>
#include <QUrl>

#include "SettingsBackend.hpp"
#include "SettingsWindow.hpp"
#include "LiveThread.hpp"
#include "Instances/IInstance.hpp"

using namespace Instances;


/**
 * @brief The Backend class
 *
 * The heart and soul of the eidtors functionality.
 * Is connected to all the other parts of the application
 * (through SIGNALs as well as references) and keeps track
 * of all the windows and code evaluation threads that are created
 * and deleted.
 */
class Backend : public QObject
{
    Q_OBJECT
public:
    static QDir directoryOf(const QString&) noexcept;
    explicit Backend(QObject *parent = 0);
    ~Backend();
    void addInstance(IInstance *, bool = true) noexcept;
    void childExited(IInstance *, QString) noexcept;
    bool isLast() noexcept;
    bool removeInstance(IInstance*, bool = true) noexcept;
    bool removeInstance(int, bool = true) noexcept;
    void removeSettings(IInstance*) noexcept;
    void removeSettings(int) noexcept;
    void saveAllSettings() noexcept;
    void saveSettings(IInstance *, QString) noexcept;
    QHash<QString, QVariant> getSettings(IInstance *) noexcept;
    QHash<QString, QVariant> getSettings(int id) noexcept;
    int nextID() noexcept;
    QList<int> loadIds() noexcept;
    QVariant getSetting(QString key, QVariant defaultValue = QVariant()) noexcept;

Q_SIGNALS:
    void warningSignal(QWidget*, QString);
    void closeAction();
    void saveAction();
    void showResults(const QString &);
    void childDoSaveSettings();

public Q_SLOTS:
    void settingsWindowRequested(IInstance*) noexcept;
    void openHelp(IInstance *) noexcept;
    void instanceClosing(IInstance *) noexcept;
    void instanceDestroyed(QObject*) noexcept;
    void instanceRunCode(IInstance *) noexcept;
    void instanceStopCode(IInstance *) noexcept;
    void instanceChangedSetting(IInstance *, const QString &key, const QVariant &value) noexcept;
    void instanceRequestSetting(IInstance *, const QString &key, QVariant &value) noexcept;
    void instanceChangedSettings(IInstance *, const QHash<QString, QVariant> &) noexcept;
    void instanceRequestSettings(IInstance *, QHash<QString, QVariant> &) noexcept;
    void instanceLoadModel(IInstance*, const QString &, const QVector3D &, const QVector3D &, const QVector3D &) noexcept;
//    void instanceRemoveID(IInstance *instance);
    void childSaidCloseAll() noexcept;
    void getExecutionResults(GlLiveThread*, QString) noexcept;

    void getError(GlLiveThread*, QString) noexcept;
    void getVertexError(GlLiveThread*, QString, int) noexcept;
    void getFragmentError(GlLiveThread*, QString, int) noexcept;

private:
    void runGlFile(IInstance *) noexcept;
    void terminateThread(long id) noexcept;
    void saveIDs() noexcept;
    QList<int> ids;
    QHash<long, std::shared_ptr<IInstance>> instances;
    QHash<long, std::shared_ptr<LiveThread>> threads;
};

#endif // BACKEND_HPP

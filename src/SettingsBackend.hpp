#ifndef SETTINGSBACKEND_HPP
#define SETTINGSBACKEND_HPP

#include <QSettings>

/**
 * @brief The SettingsBackend class
 *
 * A settings backend. Reads from and writes to the
 * platform independent persistent settings.
 */
class SettingsBackend{
public:
    static QVariant getSettingsFor(QString, const QVariant &) noexcept;
    static QVariant getSettingsFor(const QString, const QVariant, const int) noexcept;
    static QHash<QString, QVariant> getSettings(const int index) noexcept;
    static void addSettings(const QString key, const QVariant value) noexcept;
    static void saveSettingsFor(const int id, const QString &key, const QVariant &value) noexcept;
    static void saveSettingsFor(const int id, const QHash<QString, QVariant> &) noexcept;
    static void removeSettings(const int) noexcept;
private:
    static const QString globalName,
                         globalDir;
};

#endif // SETTINGSBACKEND_HPP

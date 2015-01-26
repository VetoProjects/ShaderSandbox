#include "SettingsBackend.hpp"

// no constexpr, because QString is not a literal type
const QString SettingsBackend::globalName = "ShaderSandbox";
const QString SettingsBackend::globalDir = "Live Code Editor";

/**
 * @brief SettingsBackend::getSettingsFor
 * @param key
 * @param defaultOption
 * @param id
 * @return the settings requested
 *
 * Takes a key, a default option and the index of
 * the requesting child window. Gets the settings
 * for the key or returns defaultOption.
 */
QVariant SettingsBackend::getSettingsFor(const QString key,
                                         const QVariant defaultOption,
                                         const int id) noexcept{
    QString relevantSubdir(globalDir + "/" + QString::number(id));
    QSettings set(globalName, relevantSubdir);
    return set.value(key, defaultOption);

}

/**
 * @brief SettingsBackend::getSettingsFor
 * @param key
 * @param defaultOption
 * @return the settings requested
 *
 * Takes a key and a default option. Gets the
 * settings for the key or returns defaultOption.
 */
QVariant SettingsBackend::getSettingsFor(const QString key, const QVariant &defaultOption) noexcept{
    QSettings settings(globalName, globalDir);
    return settings.value(key, defaultOption);
}

/**
 * @brief SettingsBackend::getSettings
 * @param id
 * @return the settings as a QHash
 *
 * gets the settings for a child window and
 * translates it to a QHash that is returned.
 */
QHash<QString, QVariant> SettingsBackend::getSettings(const int id) noexcept{
    QHash<QString, QVariant> settings;
    QString relevantSubdir(globalDir + "/" + QString::number(id));
    QSettings set(globalName, relevantSubdir);
    for(auto &key: set.childKeys())
        settings.insert(key, set.value(key));
    return settings;
}

/**
 * @brief SettingsBackend::addSettings
 * @param key
 * @param value
 *
 * Adds an entry to the global settings.
 */
void SettingsBackend::addSettings(const QString key, const QVariant value) noexcept{
    QSettings settings(globalName, globalDir);
    settings.setValue(key, value);
}



/**
 * @brief SettingsBackend::saveSettings
 * @param id
 * @param key
 * @param value
 *
 * Saves the settings of a instance.
 */
void SettingsBackend::saveSettingsFor(const int id, const QString &key, const QVariant &value) noexcept{
    QString relevantSubdir(globalDir + "/" + QString::number(id));
    QSettings set(globalName, relevantSubdir);
    set.setValue(key, value);
}

void SettingsBackend::saveSettingsFor(const int id, const QHash<QString, QVariant> &settings) noexcept{
    QString relevantSubdir(globalDir + "/" + QString::number(id));
    QSettings set(globalName, relevantSubdir);
    for(auto &key : settings.keys())
        set.setValue(key, settings[key]);
}

/**
 * @brief SettingsBackend::removeSettings
 * @param id
 *
 * Removes the settings of a child.
 */
void SettingsBackend::removeSettings(const int id) noexcept{
    QString relevantSubdir(globalDir + "/" + QString::number(id));
    QSettings set(globalName, relevantSubdir);
    set.clear();
}

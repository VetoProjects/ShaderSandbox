#ifndef SETTINGTABS
#define SETTINGTABS

#include <QGroupBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QStyleFactory>

/**
 * @brief The SettingsTab class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QWidget that implements a base class(not abstract)
 *
 */
class SettingsTab : public QWidget{
Q_OBJECT
public:
    SettingsTab(QHash<QString,QVariant> *Settings, QWidget* parent = 0);
Q_SIGNALS:
    void contentChanged();
protected:
    QHash<QString, QVariant> *settings;
};

/**
 * @brief The LayoutTab class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of SettingsTab that implements one of the tabs
 * of the SettingsWindow in which all configurations regarding
 * layout can be found.
 */
class LayoutTab : public SettingsTab{
Q_OBJECT
public:
    LayoutTab(QHash<QString, QVariant> *Settings, QWidget* parent = 0);
    ~LayoutTab();
private Q_SLOTS:
    void addLayout() noexcept;
    void designSettings(QString) noexcept;
    void hlSettings(int) noexcept;
    void languageSettings(int) noexcept;
private:
    QGroupBox* design;
    QLabel* designBoxLabel;
    QComboBox* designBox;
    QHBoxLayout* verticalDesign;
    QVBoxLayout* horizontalDesign;
    QGroupBox* highlighting;
    QLabel* hlBoxLabel;
    QComboBox* hlBox;
    QHBoxLayout* verticalHl;
    QVBoxLayout* horizontalHl;
    QGroupBox* language;
    QComboBox* languageBox;
    QLabel* languageBoxLabel;
    QHBoxLayout* verticalLanguage;
    QVBoxLayout* horizontalLanguage;
    QVBoxLayout* main;
};

/**
 * @brief The LayoutTab class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of SettingsTab that implements one of the tabs
 * of the SettingsWindow in which all configurations regarding
 * behaviour can be found.
 */
class BehaviourTab : public SettingsTab{
Q_OBJECT
public:
    BehaviourTab(QHash<QString, QVariant> *Settings, QWidget* parent = 0);
    ~BehaviourTab();
private Q_SLOTS:
    void openSlot(bool) noexcept;
    void sizeSlot(bool) noexcept;
private:
    void addLayout() noexcept;

    QGroupBox* startup;
    QCheckBox* openCheck;
    QCheckBox* sizeCheck;
    QVBoxLayout* startupLayout;
    QVBoxLayout* mainLayout;
};

#endif // SETTINGTABS

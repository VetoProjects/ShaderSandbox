#ifndef SETTINGSWINDOW
#define SETTINGSWINDOW

#include <QApplication>

#include "SettingsTab.hpp"
#include "SettingsBackend.hpp"

/**
 * @brief The SettingsWindow class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QDialog that implements a Settings Window
 * in which various configurations regarding behaviour and
 * layout of the editor can be made.
 */
class SettingsWindow : public QDialog{
Q_OBJECT

public:
    SettingsWindow(int);
    ~SettingsWindow();

private Q_SLOTS:
   void apply() noexcept;
   void applyClose() noexcept;
   void tryClose() noexcept;
   void changedTrue() noexcept;

private:
    void addButtons() noexcept;

    bool changed;
    QTabWidget *tabs;
    LayoutTab *layout;
    BehaviourTab *behaviour;
    QHash<QString,QVariant> settingsDict;
    int subDir;
};

#endif // SETTINGSWINDOW

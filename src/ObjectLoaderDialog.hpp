#ifndef OBJECTLOADERDIALOG
#define OBJECTLOADERDIALOG

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>

/**
 * @brief The SettingsWindow class
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 *
 * A subclass of QDialog that implements a Settings Window
 * in which various configurations regarding behaviour and
 * layout of the editor can be made.
 */
class ObjectLoaderDialog : public QDialog{
Q_OBJECT

public:
    ObjectLoaderDialog(QDialog* parent = 0);

private Q_SLOTS:
   void load();

private:
    void setupLayout();
    QVBoxLayout* setupLoader();
};

#endif // SETTINGSWINDOW

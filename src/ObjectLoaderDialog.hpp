#ifndef OBJECTLOADERDIALOG
#define OBJECTLOADERDIALOG

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QDebug>

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
    void setupCoordinateBoxes();
    QVBoxLayout* setupLoaderLayout();

    QString objectFile;
    double objectOffset[3];
    double objectScaling[3];
    double objectRotation[3];
    QDoubleSpinBox* offsetBoxX;
    QDoubleSpinBox* offsetBoxY;
    QDoubleSpinBox* offsetBoxZ;
    QDoubleSpinBox* scalingBoxX;
    QDoubleSpinBox* scalingBoxY;
    QDoubleSpinBox* scalingBoxZ;
    QDoubleSpinBox* rotationBoxX;
    QDoubleSpinBox* rotationBoxY;
    QDoubleSpinBox* rotationBoxZ;
};

#endif // SETTINGSWINDOW

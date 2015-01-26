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
#include <QFileDialog>
#include <QVector3D>
#include <QKeyEvent>

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
    void setData(const QString &modelFile, const QVector3D &modelOffset,
                 const QVector3D &modelScaling, const QVector3D &modelRotation) noexcept;

public Q_SLOTS:
    void keyPressEvent(QKeyEvent*) noexcept;

private Q_SLOTS:
    void load() noexcept;
    void selectFile() noexcept;

Q_SIGNALS:
    void objectInfo(const QString&, const QVector3D &, const QVector3D &, const QVector3D &);

private:
    void setupLayout() noexcept;
    void setupCoordinateBoxes() noexcept;
    void setupFileChooser() noexcept;
    QVBoxLayout* setupLoaderLayout() noexcept;

    QLineEdit* fileNameBox;
    QPushButton* fileChoosingButton;
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

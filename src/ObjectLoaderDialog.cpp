#include "ObjectLoaderDialog.hpp"

ObjectLoaderDialog::ObjectLoaderDialog(QDialog* parent) : QDialog(parent){
    resize(QSize(250, 300));
    setupLayout();
}

void ObjectLoaderDialog::setupLayout() noexcept{
    QPushButton* closeBut = new QPushButton(tr("Cancel"));
    QPushButton* loadBut = new QPushButton(tr("Load Object"));
    connect(loadBut, &QPushButton::clicked, this, &ObjectLoaderDialog::load);
    connect(closeBut, &QPushButton::clicked, this, &ObjectLoaderDialog::close);

    setupCoordinateBoxes();
    setupFileChooser();
    QVBoxLayout* loader = setupLoaderLayout();

    QHBoxLayout* buttons = new QHBoxLayout;
    buttons->addStretch(1);
    buttons->addWidget(loadBut);
    buttons->addWidget(closeBut);

    QVBoxLayout* main = new QVBoxLayout;
    main->addLayout(loader);
    main->addStretch(1);
    main->addSpacing(12);
    main->addLayout(buttons);

    setLayout(main);
    setWindowTitle(tr("ShaderSandbox | Load an Object"));
}

void ObjectLoaderDialog::setupCoordinateBoxes() noexcept{
    offsetBoxX = new QDoubleSpinBox();
    offsetBoxX->setMinimum(-1000);
    offsetBoxX->setMaximum(1000);
    offsetBoxX->setDecimals(4);
    offsetBoxY = new QDoubleSpinBox();
    offsetBoxY->setMinimum(-1000);
    offsetBoxY->setMaximum(1000);
    offsetBoxY->setDecimals(4);
    offsetBoxZ = new QDoubleSpinBox();
    offsetBoxZ->setMinimum(-1000);
    offsetBoxZ->setMaximum(1000);
    offsetBoxZ->setDecimals(4);
    scalingBoxX = new QDoubleSpinBox();
    scalingBoxX->setValue(1.0);
    scalingBoxX->setMaximum(1000);
    scalingBoxX->setDecimals(4);
    scalingBoxY = new QDoubleSpinBox();
    scalingBoxY->setValue(1.0);
    scalingBoxY->setMaximum(1000);
    scalingBoxY->setDecimals(4);
    scalingBoxZ = new QDoubleSpinBox();
    scalingBoxZ->setValue(1.0);
    scalingBoxZ->setMaximum(1000);
    scalingBoxZ->setDecimals(4);
    rotationBoxX = new QDoubleSpinBox();
    rotationBoxX->setMinimum(-1000);
    rotationBoxX->setMaximum(1000);
    rotationBoxX->setDecimals(4);
    rotationBoxY = new QDoubleSpinBox();
    rotationBoxY->setMinimum(-1000);
    rotationBoxY->setMaximum(1000);
    rotationBoxY->setDecimals(4);
    rotationBoxZ = new QDoubleSpinBox();
    rotationBoxZ->setMinimum(-1000);
    rotationBoxZ->setMaximum(1000);
    rotationBoxZ->setDecimals(4);
}

void ObjectLoaderDialog::setupFileChooser() noexcept{
    fileNameBox = new QLineEdit("");
    fileChoosingButton = new QPushButton(tr("Select File..."));

    connect(fileChoosingButton, &QPushButton::clicked, this, &ObjectLoaderDialog::selectFile);
}

void ObjectLoaderDialog::selectFile() noexcept{
    auto filename = QFileDialog::getOpenFileName(this);
    if(filename != "") fileNameBox->setText(filename);
}

QVBoxLayout* ObjectLoaderDialog::setupLoaderLayout() noexcept{
    auto fileLayout = new QHBoxLayout();

    fileLayout->addWidget(fileNameBox);
    fileLayout->addWidget(fileChoosingButton);

    auto offsetLayout = new QHBoxLayout();
    offsetLayout->addSpacing(5);
    offsetLayout->addWidget(new QLabel("x:"));
    offsetLayout->addWidget(offsetBoxX);
    offsetLayout->addWidget(new QLabel("y:"));
    offsetLayout->addWidget(offsetBoxY);
    offsetLayout->addWidget(new QLabel("z:"));
    offsetLayout->addWidget(offsetBoxZ);

    auto scalingLayout = new QHBoxLayout();
    scalingLayout->addSpacing(5);
    scalingLayout->addWidget(new QLabel("x:"));
    scalingLayout->addWidget(scalingBoxX);
    scalingLayout->addWidget(new QLabel("y:"));
    scalingLayout->addWidget(scalingBoxY);
    scalingLayout->addWidget(new QLabel("z:"));
    scalingLayout->addWidget(scalingBoxZ);

    auto rotationLayout = new QHBoxLayout();
    rotationLayout->addSpacing(5);
    rotationLayout->addWidget(new QLabel("x:"));
    rotationLayout->addWidget(rotationBoxX);
    rotationLayout->addWidget(new QLabel("y:"));
    rotationLayout->addWidget(rotationBoxY);
    rotationLayout->addWidget(new QLabel("z:"));
    rotationLayout->addWidget(rotationBoxZ);

    auto coordinateLayout = new QVBoxLayout();
    coordinateLayout->addWidget(new QLabel("Offset of Object:"));
    coordinateLayout->addLayout(offsetLayout);
    coordinateLayout->addWidget(new QLabel("Scale of Object:"));
    coordinateLayout->addLayout(scalingLayout);
    coordinateLayout->addWidget(new QLabel("Rotation of Object:"));
    coordinateLayout->addLayout(rotationLayout);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Path of object to render:"));
    mainLayout->addSpacing(5);
    mainLayout->addLayout(fileLayout);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(new QLabel("Coordinates of Object:"));
    mainLayout->addSpacing(5);
    mainLayout->addLayout(coordinateLayout);

    return mainLayout;
}

void ObjectLoaderDialog::setData(const QString &modelFile, const QVector3D &modelOffset,
                                 const QVector3D &modelScaling, const QVector3D &modelRotation) noexcept{
    fileNameBox->setText(modelFile);
    offsetBoxX->setValue(modelOffset.x());
    offsetBoxY->setValue(modelOffset.y());
    offsetBoxZ->setValue(modelOffset.z());
    scalingBoxX->setValue(modelScaling.x());
    scalingBoxY->setValue(modelScaling.y());
    scalingBoxZ->setValue(modelScaling.z());
    rotationBoxX->setValue(modelRotation.x());
    rotationBoxY->setValue(modelRotation.y());
    rotationBoxZ->setValue(modelRotation.z());
}

void ObjectLoaderDialog::load() noexcept{
    auto objectFile = fileNameBox->text();
    if(objectFile.isEmpty()){
        QMessageBox::warning(this, tr("ShaderSandbox"), "File name cannot be empty.");
        return;
    }

    if(!QFile(objectFile).exists()){
        QMessageBox::warning(this, tr("ShaderSandbox"), "File name must be of a valid file.");
        return;
    }

    QVector3D objectOffset(offsetBoxX->value(), offsetBoxY->value(), offsetBoxZ->value()),
              objectScaling(scalingBoxX->value(), scalingBoxY->value(), scalingBoxZ->value()),
              objectRotation(rotationBoxX->value(), rotationBoxY->value(), rotationBoxZ->value());

    Q_EMIT objectInfo(objectFile, objectOffset, objectScaling, objectRotation);
    close();
}

void ObjectLoaderDialog::keyPressEvent(QKeyEvent *evt) noexcept{
    if(evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(evt);
}

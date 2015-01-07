#include "ObjectLoaderDialog.hpp"

ObjectLoaderDialog::ObjectLoaderDialog(QDialog* parent) : QDialog(parent){
    resize(QSize(250, 300));
    setupLayout();
}

void ObjectLoaderDialog::setupLayout(){
    QPushButton* loadBut = new QPushButton(tr("Load Object"));
    QPushButton* closeBut = new QPushButton(tr("Cancel"));
    connect(loadBut, SIGNAL(clicked()), this, SLOT(load()));
    connect(closeBut, SIGNAL(clicked()), this, SLOT(close()));

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

void ObjectLoaderDialog::setupCoordinateBoxes(){
    offsetBoxX = new QDoubleSpinBox();
    offsetBoxX->setMinimum(-1000);
    offsetBoxX->setMaximum(1000);
    offsetBoxY = new QDoubleSpinBox();
    offsetBoxY->setMinimum(-1000);
    offsetBoxY->setMaximum(1000);
    offsetBoxZ = new QDoubleSpinBox();
    offsetBoxZ->setMinimum(-1000);
    offsetBoxZ->setMaximum(1000);
    scalingBoxX = new QDoubleSpinBox();
    scalingBoxX->setValue(1.0);
    scalingBoxX->setMaximum(1000);
    scalingBoxY = new QDoubleSpinBox();
    scalingBoxY->setValue(1.0);
    scalingBoxY->setMaximum(1000);
    scalingBoxZ = new QDoubleSpinBox();
    scalingBoxZ->setValue(1.0);
    scalingBoxZ->setMaximum(1000);
    rotationBoxX = new QDoubleSpinBox();
    rotationBoxX->setMinimum(-1000);
    rotationBoxX->setMaximum(1000);
    rotationBoxY = new QDoubleSpinBox();
    rotationBoxY->setMinimum(-1000);
    rotationBoxY->setMaximum(1000);
    rotationBoxZ = new QDoubleSpinBox();
    rotationBoxZ->setMinimum(-1000);
    rotationBoxZ->setMaximum(1000);
}

void ObjectLoaderDialog::setupFileChooser(){
    fileNameBox = new QLineEdit("");
    fileChoosingButton = new QPushButton(tr("Select File..."));

    connect(fileChoosingButton, SIGNAL(clicked()), this, SLOT(selectFile()));
}

void ObjectLoaderDialog::selectFile(){
    fileNameBox->setText(QFileDialog::getOpenFileName(this));
}

QVBoxLayout* ObjectLoaderDialog::setupLoaderLayout(){
    QHBoxLayout* fileLayout = new QHBoxLayout();

    fileLayout->addWidget(fileNameBox);
    fileLayout->addWidget(fileChoosingButton);

    QHBoxLayout* offsetLayout = new QHBoxLayout();
    offsetLayout->addSpacing(5);
    offsetLayout->addWidget(new QLabel("x:"));
    offsetLayout->addWidget(offsetBoxX);
    offsetLayout->addWidget(new QLabel("y:"));
    offsetLayout->addWidget(offsetBoxY);
    offsetLayout->addWidget(new QLabel("z:"));
    offsetLayout->addWidget(offsetBoxZ);

    QHBoxLayout* scalingLayout = new QHBoxLayout();
    scalingLayout->addSpacing(5);
    scalingLayout->addWidget(new QLabel("x:"));
    scalingLayout->addWidget(scalingBoxX);
    scalingLayout->addWidget(new QLabel("y:"));
    scalingLayout->addWidget(scalingBoxY);
    scalingLayout->addWidget(new QLabel("z:"));
    scalingLayout->addWidget(scalingBoxZ);

    QHBoxLayout* rotationLayout = new QHBoxLayout();
    rotationLayout->addSpacing(5);
    rotationLayout->addWidget(new QLabel("x:"));
    rotationLayout->addWidget(rotationBoxX);
    rotationLayout->addWidget(new QLabel("y:"));
    rotationLayout->addWidget(rotationBoxY);
    rotationLayout->addWidget(new QLabel("z:"));
    rotationLayout->addWidget(rotationBoxZ);

    QVBoxLayout* coordinateLayout = new QVBoxLayout();
    coordinateLayout->addWidget(new QLabel("Offset of Object:"));
    coordinateLayout->addLayout(offsetLayout);
    coordinateLayout->addWidget(new QLabel("Scale of Object:"));
    coordinateLayout->addLayout(scalingLayout);
    coordinateLayout->addWidget(new QLabel("Rotation of Object:"));
    coordinateLayout->addLayout(rotationLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(new QLabel("Path of object to render:"));
    mainLayout->addSpacing(5);
    mainLayout->addLayout(fileLayout);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(new QLabel("Coordinates of Object:"));
    mainLayout->addSpacing(5);
    mainLayout->addLayout(coordinateLayout);

    return mainLayout;
}

void ObjectLoaderDialog::load(){
    QString objectFile = fileNameBox->text();
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

    qDebug() << objectFile << objectOffset << objectScaling << objectRotation;
    Q_EMIT objectInfo(objectFile, objectOffset, objectScaling, objectRotation);
    close();
}

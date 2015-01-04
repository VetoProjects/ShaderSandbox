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
    offsetBoxY = new QDoubleSpinBox();
    offsetBoxZ = new QDoubleSpinBox();
    scalingBoxX = new QDoubleSpinBox();
    scalingBoxY = new QDoubleSpinBox();
    scalingBoxZ = new QDoubleSpinBox();
    rotationBoxX = new QDoubleSpinBox();
    rotationBoxY = new QDoubleSpinBox();
    rotationBoxZ = new QDoubleSpinBox();
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
    objectFile = fileNameBox->text();
    if(objectFile.isEmpty()){
        QMessageBox::warning(this, tr("ShaderSandbox"), "File name cannot be empty.");
        return;
    }

    if(!QFile(objectFile).exists()){
        QMessageBox::warning(this, tr("ShaderSandbox"), "File name must be of a valid file.");
        return;
    }

    objectOffset[0] = offsetBoxX->value();
    objectOffset[1] = offsetBoxY->value();
    objectOffset[2] = offsetBoxZ->value();
    objectScaling[0] = scalingBoxX->value();
    objectScaling[1] = scalingBoxY->value();
    objectScaling[2] = scalingBoxZ->value();
    objectRotation[0] = rotationBoxX->value();
    objectRotation[1] = rotationBoxY->value();
    objectRotation[2] = rotationBoxZ->value();
    qDebug() << objectFile << objectOffset[0] << objectScaling[0] << objectRotation[0];
    close();
}

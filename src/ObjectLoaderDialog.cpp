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

    QVBoxLayout* loader = setupLoader();

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

QVBoxLayout* ObjectLoaderDialog::setupLoader(){
    QHBoxLayout* fileLayout = new QHBoxLayout();

    fileLayout->addWidget(new QLineEdit());
    fileLayout->addWidget(new QPushButton(tr("Select File...")));

    QHBoxLayout* offsetLayout = new QHBoxLayout();
    offsetLayout->addSpacing(5);
    offsetLayout->addWidget(new QLabel("x:"));
    offsetLayout->addWidget(new QDoubleSpinBox());
    offsetLayout->addWidget(new QLabel("y:"));
    offsetLayout->addWidget(new QDoubleSpinBox());
    offsetLayout->addWidget(new QLabel("z:"));
    offsetLayout->addWidget(new QDoubleSpinBox());

    QHBoxLayout* scalingLayout = new QHBoxLayout();
    scalingLayout->addSpacing(5);
    scalingLayout->addWidget(new QLabel("x:"));
    scalingLayout->addWidget(new QDoubleSpinBox());
    scalingLayout->addWidget(new QLabel("y:"));
    scalingLayout->addWidget(new QDoubleSpinBox());
    scalingLayout->addWidget(new QLabel("z:"));
    scalingLayout->addWidget(new QDoubleSpinBox());

    QHBoxLayout* rotationLayout = new QHBoxLayout();
    rotationLayout->addSpacing(5);
    rotationLayout->addWidget(new QLabel("x:"));
    rotationLayout->addWidget(new QDoubleSpinBox());
    rotationLayout->addWidget(new QLabel("y:"));
    rotationLayout->addWidget(new QDoubleSpinBox());
    rotationLayout->addWidget(new QLabel("z:"));
    rotationLayout->addWidget(new QDoubleSpinBox());

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

void ObjectLoaderDialog::load(){}

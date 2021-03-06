#include "SettingsTab.hpp"

/**
 * @brief SettingsTab::SettingsTab
 *
 * The constructor of the SettingsTab class.
 * Translates the QSettings into a list of strings and integers
 * we can make use of.
 */
SettingsTab::SettingsTab(QHash<QString, QVariant> *Settings, QWidget* parent) : QWidget(parent){
    settings = Settings;
}

/**
 * @brief LayoutTab::~LayoutTab
 *
 * Destructor of the LayoutTab class.
 * Deletes the GUI elements.
 */
LayoutTab::~LayoutTab(){
    delete design;
    delete highlighting;
    delete language;
}

/**
 * @brief LayoutTab::LayoutTab
 *
 * Constructor of the LayoutTab class.
 * Calls the addLayout member function.
 */
LayoutTab::LayoutTab(QHash<QString, QVariant> *Settings, QWidget* parent) : SettingsTab(Settings, parent){
    addLayout();
}

/**
 * @brief LayoutTab::addLayout
 *
 * Creates the layout tab UI and makes it interactive.
 */
void LayoutTab::addLayout() noexcept{
    design = new QGroupBox(tr("Design"));

    designBoxLabel = new QLabel(tr("Design:"));
    designBox = new QComboBox;
    auto designConfig = settings->value("Design").toString();
    for(auto style : QStyleFactory::keys()){
        designBox->addItem(style);
        if(designConfig == style)
            designBox->setCurrentIndex(designBox->count() - 1);
    }

    connect(designBox, &QComboBox::currentTextChanged, this, &LayoutTab::designSettings);

    verticalDesign = new QHBoxLayout;
    verticalDesign->addWidget(designBoxLabel);
    verticalDesign->addWidget(designBox);

    horizontalDesign = new QVBoxLayout;
    horizontalDesign->addLayout(verticalDesign);
    design->setLayout(horizontalDesign);

    highlighting = new QGroupBox(tr("Highlighting"));

    hlBoxLabel = new QLabel(tr("Highlighting:"));
    hlBox = new QComboBox;
    hlBox->addItem(tr("GLSL"));
    hlBox->addItem(tr("Variable Names"));    
    hlBox->addItem(tr("None"));

    auto hlConfig = settings->value("Highlighting").toInt();
    if(hlConfig >= 0 && hlConfig <= 2)
        hlBox->setCurrentIndex(hlConfig);

    // old style connect because of overloaded function
    connect(hlBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(hlSettings(int)));

    verticalHl = new QHBoxLayout;
    verticalHl->addWidget(hlBoxLabel);
    verticalHl->addWidget(hlBox);

    horizontalHl = new QVBoxLayout;
    horizontalHl->addLayout(verticalHl);
    highlighting->setLayout(horizontalHl);

    language = new QGroupBox(tr("Language"));

    languageBoxLabel = new QLabel(tr("Language:"));
    languageBox = new QComboBox;
    languageBox->addItem(tr("English"));
    languageBox->addItem(tr("German"));

    auto languageConfig = settings->value("Language").toInt();
    if(languageConfig >= 0 || languageConfig <= 1)
        languageBox->setCurrentIndex(languageConfig);

    // old style connect because of overloaded function
    connect(languageBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(languageSettings(int)));

    verticalLanguage = new QHBoxLayout;
    verticalLanguage->addWidget(languageBoxLabel);
    verticalLanguage->addWidget(languageBox);

    horizontalLanguage = new QVBoxLayout;
    horizontalLanguage->addLayout(verticalLanguage);
    language->setLayout(horizontalLanguage);

    main = new QVBoxLayout;
    main->addWidget(design);
    main->addWidget(highlighting);
    main->addWidget(language);
    main->addStretch(1);
    setLayout(main);
}

/**
 * @brief LayoutTab::designSettings
 * @param index
 *
 * SLOT that reacts to the currentIndexChanged SIGNAL
 * of the Design drop down list. Changes it according to the users input.
 */
void LayoutTab::designSettings(QString text) noexcept{
    settings->insert("Design", text);
    Q_EMIT contentChanged();
}


/**
 * @brief LayoutTab::hlSettings
 * @param index
 *
 * SLOT that reacts to the currentIndexChanged SIGNAL
 * of the Highlighting drop down list. Changes it according to the users input.
 */
void LayoutTab::hlSettings(int index) noexcept{
    settings->insert("Highlighting", index);
    Q_EMIT contentChanged();
}


/**
 * @brief LayoutTab::languageSettings
 * @param index
 *
 * SLOT that reacts to the currentIndexChanged SIGNAL
 * of the Language drop down list. Changes it according to the users input.
 */
void LayoutTab::languageSettings(int index) noexcept{
    settings->insert("Language", index);
    Q_EMIT contentChanged();
}

/**
 * @brief BehaviourTab::BehaviourTab
 *
 * Constructor of the BehaviourTab class.
 * Calls addLayout().
 */
BehaviourTab::BehaviourTab(QHash<QString, QVariant> *Settings, QWidget* parent) : SettingsTab(Settings, parent){
    addLayout();
}

/**
 * @brief BehaviourTab::~BehaviourTab
 *
 * Destructor of the BehaviourTab class.
 * Deletes the GUI elements.
 */
BehaviourTab::~BehaviourTab(){
    delete startup;
}

/**
 * @brief BehaviourTab::addLayout
 *
 * Creates the layout tab UI and makes it interactive.
 */
void BehaviourTab::addLayout() noexcept{
    startup = new QGroupBox(tr("Startup Behaviour"));
    openCheck = new QCheckBox(tr("Open Last Files On Startup"));
    sizeCheck = new QCheckBox(tr("Remember Size Of Application"));

    openCheck->setChecked(settings->value("OpenFiles").toBool());
    sizeCheck->setChecked(settings->value("RememberSize").toBool());


    startupLayout = new QVBoxLayout;
    startupLayout->addWidget(openCheck);
    startupLayout->addWidget(sizeCheck);
    startupLayout->addSpacing(10);
    startup->setLayout(startupLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(startup);
    mainLayout->addSpacing(12);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

/**
 * @brief BehaviourTab::openSlot
 * @param toggled
 *
 * SLOT that reacts to the toggled() SIGNAL of
 * openCheck. Writes change to Hashlist and Q_EMITs
 * a contentChanged signal.
 */
void BehaviourTab::openSlot(bool toggled) noexcept{
    settings->insert("OpenFiles", toggled);
    Q_EMIT contentChanged();
}

/**
 * @brief BehaviourTab::sizeSlot
 * @param toggled
 *
 * SLOT that reacts to the toggled() SIGNAL of
 * sizeCheck. Writes change to Hashlist and Q_EMITs
 * a contentChanged signal.
 */
void BehaviourTab::sizeSlot(bool toggled) noexcept{
    settings->insert("RememberSize", toggled);
    Q_EMIT contentChanged();
}

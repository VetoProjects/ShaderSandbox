#include "EditorWindow.hpp"

// no constexpr, because QString is not a literal type
static const QString vertexTemplate = ":/rc/template.vert";
static const QString fragmentTemplate = ":/rc/template.frag";

/**
 * @brief EditorWindow::EditorWindow
 *
 * The constructor of the EditorWindow class.
 * Sets up the whole UI around the Editor,
 * including slots and signals.
 * Also loads the settings and loads the file
 * that was last displayed when closing the editor.
 * Also, it deals with platform-specific displaying quirks.
 */
EditorWindow::EditorWindow(const QHash<QString, QVariant> &settings, QWidget *parent) : QMainWindow(parent){
    vertexCodeEditor = new CodeEditor(this);
    fragmentCodeEditor = new CodeEditor(this);

    codeEditors = new QSplitter(Qt::Horizontal, this);
    codeEditors->addWidget(vertexCodeEditor);
    codeEditors->addWidget(fragmentCodeEditor);

    setCentralWidget(codeEditors);

    objectLoaderDialog = new ObjectLoaderDialog();
    connect(objectLoaderDialog, &ObjectLoaderDialog::objectInfo,
        [this](const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation) {
            this->modelFile = file;
            this->modelOffset = offset;
            this->modelScaling = scaling;
            this->modelRotation = rotation;
            this->saveSettings();
            Q_EMIT loadModel(file, offset, scaling, rotation);
        }
    );

    addActions();
    addMenus();
    addToolBars();
    addStatusBar();

    connect(vertexCodeEditor->document(), SIGNAL(contentsChanged()), this, SLOT(docModified()));
    connect(fragmentCodeEditor->document(), SIGNAL(contentsChanged()), this, SLOT(docModified()));

    applySettings(settings);

    // Mac quirks
    setUnifiedTitleAndToolBarOnMac(true);

#if defined(Q_OS_MAC)
    setWindowIcon(QIcon("Icon.icns"));
#elif defined(Q_OS_WIN)
    setWindowIcon(QIcon("Icon.ico"));
#endif
}

/**
 * @brief EditorWindow::~EditorWindow
 *
 * Destructor of the EditorWindow class.
 * Deletes all the GUI elements.
 */
EditorWindow::~EditorWindow(){
    delete vertexCodeEditor;
    delete fragmentCodeEditor;
    delete newAction;
    delete openAction;
    delete saveAction;
    delete exitAction;
    delete runAction;
    delete settingsAction;
    delete helpAction;
    delete loadObjectAction;
    delete fMenu;
    delete eMenu;
    delete hMenu;
    delete fileBar;
    delete editBar;
    delete objectLoaderDialog;
}

/**
 * @brief EditorWindow::closeEvent
 * @param event
 *
 * reacts to the close signal; saves the current preferences
 * if wanted and exits(SLOT).
 */
void EditorWindow::closeEvent(QCloseEvent *event){
    if(saveDialog()){
        saveSettings();
        Q_EMIT closing(this);
        event->accept();
    }else
        event->ignore();
}

void EditorWindow::gotOpenHelp()
{
    Q_EMIT openHelp(this);
}

void EditorWindow::gotCloseAll()
{
    Q_EMIT closeAll(this);
}

/**
 * @brief EditorWindow::newFile
 *
 * opens a new, empty file in the editor(SLOT).
 */
void EditorWindow::newFile(){
    QStringList editor;
    bool ok;
    editor << "Both" << "VertexShader" << "FragmentShader";
    QString choice = QInputDialog::getItem(this, "Which file do you want to save", "Files: ", editor, 0, false, &ok);

    if(!ok) return;

    if(choice == "VertexShader" || choice == "Both"){
        if(vertexCodeEditor->document()->isModified()) {
            QMessageBox::StandardButton question = QMessageBox::warning(this,
                tr("ShaderSandbox"),
                tr("The vertex shader have been modified but are unsaved.\n"
                   "Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

            if(question == QMessageBox::Cancel) return;
            if(question == QMessageBox::Save && !saveFile("VertexShader")) return;
        }

        vertexCodeEditor->clear();
        setAsCurrentFile("", currentFragmentFile);
        loadFile(vertexTemplate);
    }

    if(choice == "FragmentShader" || choice == "Both"){
        if(fragmentCodeEditor->document()->isModified()){
            QMessageBox::StandardButton question = QMessageBox::warning(this,
                tr("ShaderSandbox"),
                tr("The fragment shader have been modified but are unsaved.\n"
                   "Do you want to save your changes?"),
                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

            if(question == QMessageBox::Cancel) return;
            if(question == QMessageBox::Save && !saveFile("FragmentShader")) return;
        }

        fragmentCodeEditor->clear();
        setAsCurrentFile(currentVertexFile, "");
        loadFile(fragmentTemplate);
    }
}

/**
 * @brief EditorWindow::openFile
 *
 * Opens a file choosing dialog and opens
 * the requested file(SLOT).
 */
void EditorWindow::openFile(){
    if(saveDialog()){
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

/**
 * @brief EditorWindow::saveSettings
 *
 * saves own settings.
 */
void EditorWindow::saveSettings(){
    QHash<QString, QVariant> settings;
    settings.insert("pos", this->pos());
    settings.insert("size", this->size());

    settings.insert("vertexFile", currentVertexFile);
    settings.insert("fragmentFile", currentFragmentFile);

    settings.insert("modelFile", modelFile);
    settings.insert("modelOffsetX", modelOffset.x());
    settings.insert("modelOffsetY", modelOffset.y());
    settings.insert("modelOffsetZ", modelOffset.z());
    settings.insert("modelScalingX", modelScaling.x());
    settings.insert("modelScalingY", modelScaling.y());
    settings.insert("modelScalingZ", modelScaling.z());
    settings.insert("modelRotationX", modelRotation.x());
    settings.insert("modelRotationY", modelRotation.y());
    settings.insert("modelRotationZ", modelRotation.z());

    Q_EMIT changedSettings(this, settings);
}

/**
 * @brief EditorWindow::openSettings
 *
 * creates an instance of SettingsWindow and executes it.
 */
void EditorWindow::gotOpenSettings(){
    Q_EMIT openSettings(this);
}

/**
 * @brief EditorWindow::docModified
 *
 * reacts to the contentsChanged() SIGNAL by setting the
 * window properties to modified/not modified( as shown by
 * the little * at the top of the window beside the name).
 */
void EditorWindow::docModified(){
    setWindowModified(vertexCodeEditor->document()->isModified() || fragmentCodeEditor->document()->isModified());
}

/**
 * @brief EditorWindow::warningDisplay
 * @param message
 *
 * Displays a warning box containing message.
 */
void EditorWindow::warningDisplay(const QString &message){
    QMessageBox::warning(this, tr("VeToLC"), message);
}

/**
 * @brief EditorWindow::codeStopped
 *
 * Sets the icon back to normal.
 */
void EditorWindow::codeStopped()
{
    runAction->setIcon(QIcon(":/images/run.png"));
}

/**
 * @brief EditorWindow::highlightErroredLine
 * @param lineno
 *
 * Highlights a given line in red. Signifies an error in
 * that line.
 */
void EditorWindow::highlightErroredLine(int lineno){
    vertexCodeEditor->highlightErroredLine(lineno);
    // TODO: filter fragment-shader-errors
}

/**
 * @brief EditorWindow::apply‚Settings
 *
 * Reads the "persistent platform-independent
 * application settings" and gets the settings.
 */
void EditorWindow::applySettings(const QHash<QString, QVariant> &settings){
    if(settings.value("RememberSize").toBool()){
        const QPoint  pos  = settings.value("pos", QPoint(0, 0)).toPoint();
        const QSize   size = settings.value("size", QSize(800, 600)).toSize();
        move(pos);
        resize(size);
    }
    modelFile = settings.value("modelFile", "").toString();
    modelOffset.setX(settings.value("modelOffsetX", 0).toFloat());
    modelOffset.setY(settings.value("modelOffsetY", 0).toFloat());
    modelOffset.setZ(settings.value("modelOffsetZ", 0).toFloat());
    modelScaling.setX(settings.value("modelScalingX", 1).toFloat());
    modelScaling.setY(settings.value("modelScalingY", 1).toFloat());
    modelScaling.setZ(settings.value("modelScalingZ", 1).toFloat());
    modelRotation.setX(settings.value("modelRotationX", 0).toFloat());
    modelRotation.setY(settings.value("modelRotationY", 0).toFloat());
    modelRotation.setZ(settings.value("modelRotationZ", 0).toFloat());
    objectLoaderDialog->setData(modelFile, modelOffset, modelScaling, modelRotation);

    currentVertexFile = settings.value("vertexFile", vertexTemplate).toString();
    currentFragmentFile = settings.value("fragmentFile", fragmentTemplate).toString();

    loadFile(currentVertexFile, true, false);
    loadFile(currentFragmentFile, false, true);
}

/**
 * @brief EditorWindow:: runFile
 *
 * lets the backend run the file.
 */
void EditorWindow::runFile(){
    runAction->setIcon(QIcon(":/images/refresh.png"));
    Q_EMIT runCode(this);
    Q_EMIT loadModel(modelFile, modelOffset, modelScaling, modelRotation);
}

/**
 * @brief EditorWindow::showResults
 * @param returnedValue
 *
 * Is called after the execution of the interpreter thread has finished.
 * Shows its return code or an exception traceback.
 */
void EditorWindow::showResults(const QString &returnedValue){
    statusBar()->showMessage(returnedValue, 5000);
    runAction->setIcon(QIcon(":/images/run.png"));
}

/**
 * @brief EditorWindow::getSourceCode
 * @return plain text in editor
 *
 * returns code in editor.
 */
QString EditorWindow::getVertexSourceCode() const{
    return vertexCodeEditor->toPlainText();
}

QString EditorWindow::getFragmentSourceCode() const
{
    return fragmentCodeEditor->toPlainText();
}

/**
 * @brief EditorWindow::getTitle
 * @return current file name as string.
 *
 * returns current file name.
 */
QString EditorWindow::getTitle() const{
    return currentVertexFile + " | " + currentFragmentFile;
}

/**
 * @brief EditorWindow::addActions
 *
 * adds signals(buttons, shortcuts) to the window.
 */
void EditorWindow::addActions(){
    newAction = new QAction(QIcon(":/images/new.png"), tr("&New File"), this);
    newAction->setShortcuts(QKeySequence::New);
    newAction->setStatusTip(tr("Create a new file"));
    connect(newAction, SIGNAL(triggered()), this, SLOT(newFile()));

    openAction = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

    saveAction = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save the document to disk"));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(gotCloseAll()));

    runAction = new QAction(QIcon(":/images/run.png"), tr("&Run"), this);
    runAction->setShortcuts(QKeySequence::Refresh);
    runAction->setStatusTip(tr("Runs the code in the editor"));
    connect(runAction, SIGNAL(triggered()), this, SLOT(runFile()));

    settingsAction = new QAction(QIcon(":/images/settings.png"), tr("Settings"), this);
    settingsAction->setShortcuts(QKeySequence::Preferences);
    settingsAction->setStatusTip(tr("Opens A Settings Window"));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(gotOpenSettings()));

    helpAction = new QAction(tr("About"), this);
    helpAction->setShortcuts(QKeySequence::HelpContents);
    helpAction->setStatusTip(tr("Opens the Help"));
    connect(helpAction, SIGNAL(triggered()), this, SLOT(gotOpenHelp()));

    loadObjectAction = new QAction(QIcon(":/images/object.png"), tr("Load Object"), this);
    loadObjectAction->setStatusTip(tr("Loads an object into your 3D-World"));
    connect(loadObjectAction, SIGNAL(triggered()), objectLoaderDialog, SLOT(show()));
}

/**
 * @brief EditorWindow::addMenus
 *
 * adds the menus to the window(duh!) and fills them
 * with the corresponding actions.
 */
void EditorWindow::addMenus(){
    fMenu = menuBar()->addMenu(tr("&File"));
    fMenu->addAction(newAction);
    fMenu->addAction(openAction);
    fMenu->addAction(saveAction);
    fMenu->addSeparator();
    fMenu->addAction(exitAction);

    eMenu = menuBar()->addMenu(tr("&Edit"));
    eMenu->addAction(settingsAction);
    eMenu->addAction(runAction);
    menuBar()->addSeparator();

    hMenu = menuBar()->addMenu(tr("&Help"));
    hMenu->addAction(helpAction);
}

/**
 * @brief EditorWindow::addToolBars
 *
 * adds the toolbars to the window(duh!) and fills them
 * with the corresponding actions.
 */
void EditorWindow::addToolBars(){
    fileBar = addToolBar(tr("File"));
    fileBar->addAction(newAction);
    fileBar->addAction(openAction);
    fileBar->addAction(saveAction);

    editBar = addToolBar(tr("Edit"));
    fileBar->addAction(settingsAction);
    fileBar->addAction(runAction);
    fileBar->addAction(loadObjectAction);
}

/**
 * @brief EditorWindow::addStatusBar
 *
 * Adds a status bar to the window(duh!) and shows a
 * message in it displaying it is ready to use.
 */
void EditorWindow::addStatusBar(){
    statusBar()->showMessage(tr("Ready"));
}

/**
 * @brief EditorWindow::saveDialog
 * @return bool - true if the user does not want to save or the
 *         document is saved in its current state/false if he
 *         cancels/return code of the saveFile() function
 *
 * Opens a dialog asking the user if he wants to save the changes.
 * Invoked when he exits the application/the current file unchanged.
 */
bool EditorWindow::saveDialog(){
    if(vertexCodeEditor->document()->isModified() || fragmentCodeEditor->document()->isModified()){
        QMessageBox::StandardButton question;
        question = QMessageBox::warning(this, tr("ShaderSandbox"),
                                tr("The documents have been modified"
                                   " but are unsaved.\n"
                                "Do you want to save your changes?"),
                                QMessageBox::Save | QMessageBox::Discard
                                | QMessageBox::Cancel);
        if(question == QMessageBox::Save)
            return saveFile();
        else if(question == QMessageBox::Cancel)
            return false;
    }
    return true;
}

/**
 * @brief EditorWindow::loadFile
 * @param path
 *
 * loads a file of a given name
 */
void EditorWindow::loadFile(const QString &path, bool v, bool f){
    QFileInfo fileInfo(path);
    QString suffix = fileInfo.suffix().toLower();
    bool vertexFile = !f && (v || suffix == "vs" || suffix == "vert" || suffix == "vertex" || suffix == "vertexshader");
    bool fragmentFile = !v && (f || suffix == "fs" || suffix == "frag" || suffix == "fragment" || suffix == "fragmentshader");

    if(!vertexFile && !fragmentFile){
        QStringList editor;
        bool ok;
        editor << "VertexShader" << "FragmentShader";
        QString choice = QInputDialog::getItem(this, "Which file do you want to load", "Files: ", editor, 0, false, &ok);

        if(!ok) return;
        if(choice == "VertexShader")
            vertexFile = true;
        else
            fragmentFile = true;
    }

    QFile file(path);
    //display an error message if the file cannot be opened and why
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        QString msg = tr("Cannot read file %1:\n%2.").arg(path).arg(file.errorString());
        QMessageBox::warning(this, tr("ShaderSandbox"), msg);
        return;
    }

    /*
     * while the application is busy loading,
     * set a waiting cursor. Real fancy, huh?
     */
    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    if(vertexFile){
        vertexCodeEditor->setPlainText(in.readAll());
        setAsCurrentFile(path, currentFragmentFile);
    }
    if(fragmentFile){
        fragmentCodeEditor->setPlainText(in.readAll());
        setAsCurrentFile(currentVertexFile, path);
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    //set current file and display a message in the status bar
    statusBar()->showMessage(tr("File was loaded"), 2000);
}

/**
 * @brief EditorWindow::saveFile
 * @param name
 * @return bool - false if the file cannot be saved/
 *         true if everything went fine
 *
 * saves the current file under a provided name
 */
bool EditorWindow::saveFile(){
    QStringList editor;
    bool ok;
    editor << "Both" << "VertexShader" << "FragmentShader";
    QString choice = QInputDialog::getItem(this, "Which file do you want to save", "Files: ", editor, 0, false, &ok);

    if(!ok) return false;

    // if saving the vertex shader was not successful, save fragment shader anyway (& not &&)
    if(choice == "Both")
        return saveFile("VertexShader") & saveFile("FragmentShader");
    else
        return saveFile(choice);
}

bool EditorWindow::saveFile(QString shaderType){
    QFile file;
    if(shaderType == "VertexShader" && (currentVertexFile.isEmpty() || currentVertexFile == vertexTemplate))
        file.setFileName(QFileDialog::getSaveFileName(this));
    else if(shaderType == "FragmentShader" && (currentFragmentFile.isEmpty() || currentFragmentFile == fragmentTemplate))
        file.setFileName(QFileDialog::getSaveFileName(this));
    else if(shaderType == "VertexShader")
        file.setFileName(currentVertexFile);
    else
        file.setFileName(currentFragmentFile);


    if(file.fileName().isEmpty()) return false;


    //display an error message if the file cannot be saved and why
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        warningDisplay(tr("Cannot write file %1:\n%2.").arg(file.fileName()).arg(file.errorString()));
        return false;
    }

    /*
     * while the application is busy loading,
     * set a waiting cursor. Real fancy, huh?
     */
    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif

    if(shaderType == "VertexShader"){
        out << vertexCodeEditor->toPlainText();
        currentVertexFile = file.fileName();
    } else {
        out << fragmentCodeEditor->toPlainText();
        currentFragmentFile = file.fileName();
    }

#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setAsCurrentFile(currentVertexFile, currentFragmentFile);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}


/**
 * @brief EditorWindow::setAsCurrentFile
 * @param name
 *
 * sets the file name and displays it at the top of the window
 */
void EditorWindow::setAsCurrentFile(const QString &vertexFile, const QString &fragmentFile){
    currentVertexFile = vertexFile;
    currentFragmentFile = fragmentFile;

    Q_EMIT changedSetting(this, "vertexFile", vertexFile);
    Q_EMIT changedSetting(this, "fragmentFile", fragmentFile);

    vertexCodeEditor->document()->setModified(false);
    fragmentCodeEditor->document()->setModified(false);

    setWindowModified(false);

    QString title = "ShaderSandbox";

    if(vertexFile != "" && !vertexFile.contains("template."))
        title.append(" | ").append(stripName(vertexFile));
    if(fragmentFile != "" && !fragmentFile.contains("template."))
        title.append(" | ").append(stripName(fragmentFile));
    setWindowFilePath(title);
    setWindowTitle(title.append(" [*]"));
}

/**
 * @brief EditorWindow::stripName
 * @param fullName
 * @return return code of QFileInfo
 *
 * strips the fully qualified name of a file to the unqualified one
 */
QString EditorWindow::stripName(const QString &fullName){
    return QFileInfo(fullName).fileName();
}

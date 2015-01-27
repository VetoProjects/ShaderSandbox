#include "Renderer.hpp"


static GLfloat vertices[] = {
    1, 1,0,  1,-1,0, -1,1,0,
    1,-1,0, -1,-1,0, -1,1,0
};
static GLfloat uvs[] = {
    1,0, 1,1, 0,0,
    1,1, 0,1, 0,0
};

const QString Renderer::defaultVertexShader = QString(QFile(":/rc/template.vert").readAll());
const QString Renderer::defaultFragmentShader = QString(QFile(":/rc/template.frag").readAll());

/**
 * @brief Renderer::Renderer
 * @param parent Parent object of the render window
 *
 * Create a new Renderer with default shader
 */
Renderer::Renderer(QWindow *parent) : Renderer::Renderer(defaultVertexShader, defaultFragmentShader, parent){ }

/**
 * @brief Renderer::Renderer
 * @param filename Name that should shown in the title
 * @param instructions Shader code for execution
 * @param parent Parent object of the render window
 *
 * Create a new Renderer with given code and set filename as title
 */
Renderer::Renderer(const QString &vertexShader, const QString &fragmentShader, QWindow *parent) :
    QWindow(parent),
    clearColor(Qt::black),
    context(0), device(0),
    time(0),
    pendingUpdate(false),
    vao(0), uvBuffer(0), audioLeftTexture(0), audioRightTexture(0),
    vertexAttr(0), uvAttr(0), timeUniform(0),
    mID(0), vID(0), pID(0),
    shaderProgram(0),
    vertexSource(vertexShader), fragmentSource(fragmentShader),
    textureRegEx("(^|\n|\r)\\s*#texture\\s+([A-Za-z_][A-Za-z0-9_]*)\\s+([^\n\r]+)")
{
    setTitle("ShaderSandbox Renderer");

    m_logger = new QOpenGLDebugLogger( this );

    connect(m_logger, &QOpenGLDebugLogger::messageLogged,
            this, &Renderer::onMessageLogged, Qt::DirectConnection);

    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setVersion(3, 3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setDepthBufferSize(24);
    setFormat(format);

    cameraPosition.setY(-1);
    cameraPosition.setZ(-2);
    cameraRotation = 0;
    cameraPitch = 30;
    V.rotate(cameraPitch, 1, 0, 0);
    V.translate(cameraPosition);
    P.perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    time = new QTime();
    time->start();

    audio = new AudioInputProcessor(this);
    connect(audio, &AudioInputProcessor::processData,
            this,  &Renderer::updateAudioData);
    audio->start();
}

/**
 * @brief Renderer::~Renderer
 *
 * Free resources
 */
Renderer::~Renderer(){
    if(context)
        context->makeCurrent(this);
    if(shaderProgram){
        shaderProgram->bind();
        for(auto *texture : textures){
            texture->destroy();
            delete texture;
        }
        delete shaderProgram;
    }
    glDeleteBuffers(1, &uvBuffer);
    glDeleteTextures(1, &audioLeftTexture);
    glDeleteTextures(1, &audioRightTexture);
    delete time;
    delete vao;
    delete device;
    delete m_logger;
}

/**
 * @brief Renderer::init
 * @return True on success, otherwise false
 *
 * Allocates grafic memory and initialize the shader program
 */
bool Renderer::init(){
    model.init();
    if(modelFile != "")
        model.loadModel(modelFile.toStdString());

    delete vao;
    vao = new QOpenGLVertexArrayObject(this);
    vao->create();
    vao->bind();

    glDeleteBuffers(1, &vertexBuffer);
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glDeleteBuffers(1, &uvBuffer);
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glDeleteTextures(1, &audioLeftTexture);
    glGenTextures(1, &audioLeftTexture);
    glBindTexture(GL_TEXTURE_1D, audioLeftTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glDeleteTextures(1, &audioRightTexture);
    glGenTextures(1, &audioRightTexture);
    glBindTexture(GL_TEXTURE_1D, audioRightTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    bool result = initShaders(vertexSource, fragmentSource);

    vao->release();

    return result;
}


/**
 * @brief Renderer::initShaders
 * @param fragmentShader Code to compile as shader
 * @return True on success, otherwise false
 *
 * Initialze and compile the shader program
 */
bool Renderer::initShaders(QString vertexShader, QString fragmentShader){
    QList<QPair<QString, QString>> images;
    QDir modelDir = QFileInfo(modelFile).dir();

    int pos = 0;
    while((pos = textureRegEx.indexIn(fragmentShader, pos)) != -1){
        QString imageName = textureRegEx.cap(2).trimmed();
        QString imagePath = textureRegEx.cap(3).trimmed();
        QFileInfo textureImage;
		
        if(modelDir.exists())
            textureImage = QFileInfo(modelDir, imagePath);
        else
            textureImage = QFileInfo(imagePath);

        if(!textureImage.isFile()){
            qDebug() << "Texture image does not exsit: " << imagePath;
            if(shaderProgram == 0){
                if(vertexShader == defaultVertexShader && fragmentShader == defaultFragmentShader)
                    qWarning() << tr("Failed to compile default shader.");
                else
                    initShaders(defaultVertexShader, defaultFragmentShader);
            }
            Q_EMIT fragmentError("Image file does not exist: " + imagePath, fragmentShader.mid(0, pos).count('\n'));
            return false;
        }

        images.append(QPair<QString, QString>(imageName, textureImage.absoluteFilePath()));

        QString textureDefinition(textureRegEx.cap(1) + "uniform sampler2D " + imageName + ";");
        fragmentShader.remove(pos, textureRegEx.matchedLength());
        fragmentShader.insert(pos, textureDefinition);
        pos += textureDefinition.length();
    }
	
    QOpenGLShaderProgram *newShaderProgram = new QOpenGLShaderProgram(this);
    bool vertexOk = newShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader);
    bool fragmentOk = vertexOk && newShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,fragmentShader);
    bool linkOk = fragmentOk && newShaderProgram->link();

    if(!linkOk){
        QString error = newShaderProgram->log();
        delete newShaderProgram;

        if(vertexOk && fragmentOk)
            Q_EMIT errored(error);
        else{
            //mac  :<line>:
            //mesa :<line>(<errorcode>):
            QRegExp errorline(":([0-9]+)(\\([0-9]+\\))?:");
            if(errorline.indexIn(error) > -1){
                QString text = errorline.cap(1);
                int line = text.toInt();
                if (line >= 3) line -= 3;
                if(!vertexOk)
                    Q_EMIT vertexError(error, line);
                else
                    Q_EMIT fragmentError(error, line);
            }
        }

        if(shaderProgram == 0 && (vertexShader != defaultFragmentShader || fragmentShader != defaultFragmentShader)){
            initShaders(defaultVertexShader, defaultFragmentShader);
        } else {
            qWarning() << tr("Failed to compile default shader.");
        }

        return false;
    }

    QList<QOpenGLTexture*> newTextures;
    for(auto image: images){

        QOpenGLTexture* texture = new QOpenGLTexture(QImage(image.second));

        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);

        //qDebug() << imageName << " from " << imagePath << ": " << texture->textureId() << " (" << texture << ")";

        newTextures.append(texture);
    }

    shaderProgramMutex.lock();

        if(shaderProgram){
            shaderProgram->bind();
            for(auto *texture : textures){
                texture->destroy();
                delete texture;
            }
            delete shaderProgram;
        }
        textures = newTextures;
        shaderProgram = newShaderProgram;
        shaderProgram->bind();

        vertexAttr = shaderProgram->attributeLocation("position");
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        shaderProgram->setAttributeBuffer(vertexAttr, GL_FLOAT, 0, 3);
        shaderProgram->enableAttributeArray(vertexAttr);

        uvAttr = shaderProgram->attributeLocation("texCoord");
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        shaderProgram->setAttributeBuffer("texCoord", GL_FLOAT, 0, 2);
        shaderProgram->enableAttributeArray(uvAttr);

        vID = shaderProgram->uniformLocation("V");
        pID = shaderProgram->uniformLocation("P");
        mID = shaderProgram->uniformLocation("M");

        timeUniform = shaderProgram->uniformLocation("time");
        mouseUniform = shaderProgram->uniformLocation("mouse");
        rationUniform = shaderProgram->uniformLocation("ration");

        shaderProgram->setUniformValue("audioLeft", GLint(0));
        shaderProgram->setUniformValue("audioRight", GLint(1));
        const int end = images.length();
        for(int i = 0; i < end; ++i)
            shaderProgram->setUniformValue(images[i].first.toLocal8Bit().data(), GLint(i + 2));

        vertexSource = vertexShader;
        fragmentSource = fragmentShader;
    shaderProgramMutex.unlock();

//    qDebug() << "vertexAttr" << vertexAttr;
//    qDebug() << "uvAttr" << uvAttr;
//    qDebug() << "timeUniform" << timeUniform;
//    qDebug() << "audioUniform" << audioUniform;

//    uploadMVP();

    return true;
}



/**
 * @brief Renderer::render
 *
 * Initialize output device, execute the shader and display the result
 */
void Renderer::render(){
    if(!device)
        device = new QOpenGLPaintDevice();

    device->setSize(size());

//    qDebug() << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_VERSION))) << " " << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    QPoint mouse = this->mapFromGlobal(QCursor::pos());
    QVector2D mousePosition((float)mouse.x() / (float)this->width(),
                            (float)mouse.y() / (float)this->height());
    float ration = ((this->height() == 0) ? 1 : (float)this->width() / (float)this->height());

    handleInput();

    shaderProgramMutex.lock();
        shaderProgram->bind();
        glClearColor(0, 0, 0.3, 1);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDisable(GL_CULL_FACE);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
//        glDisable(GL_ALPHA_TEST);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClearDepth(1);
        glDepthFunc(GL_LESS);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vao->bind();

        glUniformMatrix4fv(pID, 1, GL_FALSE, P.data());
        glUniformMatrix4fv(vID, 1, GL_FALSE, V.data());
        glUniformMatrix4fv(mID, 1, GL_FALSE, M.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, audioLeftTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, audioRightTexture);

        for(int i = 0; i < textures.length(); ++i){
            glActiveTexture(GL_TEXTURE2 + i);
            textures[i]->bind();
        }

        shaderProgram->setUniformValue(mouseUniform, mousePosition);
        shaderProgram->setUniformValue(rationUniform, ration);
        shaderProgram->setUniformValue(timeUniform, GLfloat(time->elapsed()));

//        glDrawArrays(GL_TRIANGLES, 0, 6);
//        MV  = V * M,
//        MVP = P * MV;

//        glUniformMatrix4fv(pID,   1, GL_FALSE, P.data());
//        glUniformMatrix4fv(vID,   1, GL_FALSE, V.data());
//        glUniformMatrix4fv(mID,   1, GL_FALSE, M.data());
//        glUniformMatrix4fv(mvID,  1, GL_FALSE, MV.data());
//        glUniformMatrix4fv(mvpID, 1, GL_FALSE, MVP.data());


        model.draw();

        vao->release();
        shaderProgram->release();
    shaderProgramMutex.unlock();
}

void Renderer::handleInput(){
    float timeDelta = time->elapsed() - lastTime;
    lastTime = time->elapsed();

    bool changed = false;
    bool alt = pressedKeys.contains(Qt::Key_Alt);
    bool ctrl = pressedKeys.contains(Qt::Key_Control);
    bool shift = pressedKeys.contains(Qt::Key_Shift);
    if(!mouseDragLeft.isNull()){
        if(!alt && !ctrl && !shift){
            changed = true;
            cameraRotation += mouseDragLeft.x() * mouseRotationSpeed;
            cameraPitch += mouseDragLeft.y() * mouseRotationSpeed;
        }
        mouseDragLeft = QPoint();
    }

    if(pressedKeys.contains(Qt::Key_Up)){
        changed = true;
        cameraPitch -= timeDelta * keyRotationSpeed;
    }
    if(pressedKeys.contains(Qt::Key_Down)){
        changed = true;
        cameraPitch += timeDelta * keyRotationSpeed;
    }
    if(pressedKeys.contains(Qt::Key_Right)){
        changed = true;
        cameraRotation += timeDelta * keyRotationSpeed;
    }
    if(pressedKeys.contains(Qt::Key_Left)){
        changed = true;
        cameraRotation -= timeDelta * keyRotationSpeed;
    }

    QVector3D offset;

    if(!mouseDragRight.isNull()){
        changed = true;
        offset.setX(+ mouseDragRight.x() * mouseMovementSpeed);
        offset.setY(- mouseDragRight.y() * mouseMovementSpeed);
        mouseDragRight = QPoint();
    }

    if(pressedKeys.contains(Qt::Key_W)){
        changed = true;
        offset.setZ(offset.z() + timeDelta * keyMovementSpeed);
    }
    if(pressedKeys.contains(Qt::Key_S)){
        changed = true;
        offset.setZ(offset.z() - timeDelta * keyMovementSpeed);
    }
    if(pressedKeys.contains(Qt::Key_D)){
        changed = true;
        offset.setX(offset.x() - timeDelta * keyMovementSpeed);
    }
    if(pressedKeys.contains(Qt::Key_A)){
        changed = true;
        offset.setX(offset.x() + timeDelta * keyMovementSpeed);
    }

    if(changed){
        V = QMatrix4x4();
        V.rotate(-cameraRotation, 0, 1, 0);
        V.rotate(-cameraPitch, 1, 0, 0);
        cameraPosition += V * offset;

        V = QMatrix4x4();
        V.rotate(cameraPitch, 1, 0, 0);
        V.rotate(cameraRotation, 0, 1, 0);
        V.translate(cameraPosition);
    }
}

/**
 * @brief Renderer::renderLater
 *
 * Enqueue an update event to event queue
 */
void Renderer::renderLater(){
    if(!pendingUpdate){
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        pendingUpdate = true;
    }
}

/**
 * @brief Renderer::renderNow
 *
 * Use the compiled shader to render on the widget
 */
void Renderer::renderNow(){
    pendingUpdate = false;

    if(!isExposed())
        return;

    if(context)
        context->makeCurrent(this);
    else {
        context = new QOpenGLContext(this);
        context->setFormat(requestedFormat());
        context->create();
        context->makeCurrent(this);

        if (m_logger->initialize()){
            m_logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            m_logger->enableMessages();
        }
        initializeOpenGLFunctions();
        init();
    }


    if(!shaderProgram)
        initShaders(vertexSource, fragmentSource);

    if(shaderProgram)
        render();

    context->swapBuffers(this);

    renderLater();
}

/**
 * @brief Renderer::event
 * @param event The event that should be proccessed
 * @return True if the event was successful proccessed, otherwise false
 *
 * Called if a new event is poped from the event-queue to render on update event
 * and Q_EMIT doneSignal on close event.
 */
bool Renderer::event(QEvent *event){
    QMouseEvent *mouse;

    switch(event->type()){
    case QEvent::UpdateRequest:
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        renderNow();
        return true;
    case QEvent::Close:
        Q_EMIT doneSignal(tr("User closed renderer"));
        return true;
    case QEvent::KeyPress:
        pressedKeys.insert(((QKeyEvent*)event)->key());
        return QWindow::event(event);
    case QEvent::KeyRelease:
        pressedKeys.remove((((QKeyEvent*)event)->key()));
        return QWindow::event(event);
    case QEvent::MouseMove:
        mouse = (QMouseEvent*)event;
        if(mouse->buttons() & Qt::LeftButton)  mouseDragLeft  += mouse->pos() - lastMousePosition;
        if(mouse->buttons() & Qt::RightButton) mouseDragRight += mouse->pos() - lastMousePosition;
        lastMousePosition = mouse->pos();
        return QWindow::event(event);
    default:
        return QWindow::event(event);
    }
}

/**
 * @brief Renderer::exposeEvent
 *
 * Called if the window is ready to start rendering
 */
void Renderer::exposeEvent(QExposeEvent *){
    if(isExposed())
        renderNow();
}

/**
 * @brief Renderer::updateCode
 * @param filename Text for the title
 * @param code New shader program code
 * @return True on success, otherwise false.
 *
 * Set new title and compile new code for the shader program
 */
bool Renderer::updateCode(const QString &vertCode, const QString &fragCode){
    if(!initShaders(vertCode, fragCode))
        return false;
    show();
    return true;
}

/**
 * @brief Renderer::updateAudioData
 * @param data New audio data
 *
 * Copy the new sound-data to the graphics memory for visualisation
 */
void Renderer::updateAudioData(QByteArray data){
    if(!shaderProgram)
        return;
    GLenum type, internalType;
    Q_UNUSED(internalType);
    char typeSize;
    switch(audio->format().sampleType() + audio->format().sampleSize()){
        case  8: case 10:
            type = GL_UNSIGNED_BYTE;
            internalType = GL_R8UI;
            typeSize = 1;
            break;
        case  9:
            type = GL_BYTE;
            internalType = GL_R8I;
            typeSize = 1;
            break;
        case 16: case 18:
            type = GL_UNSIGNED_SHORT;
            internalType = GL_R16UI;
            typeSize = 2;
            break;
        case 17:
            type = GL_SHORT;
            internalType = GL_R16I;
            typeSize = 2;
            break;
        case 32: case 34:
            type = GL_UNSIGNED_INT;
            internalType = GL_R32UI;
            typeSize = 4;
            break;
        case 33:
            type = GL_INT;
            internalType = GL_R32I;
            typeSize = 4;
            break;
        case 35:
            type = GL_FLOAT;
            internalType = GL_R32F;
            typeSize = 4;
            break;
        default: return;
    }
    char *left, *right;
    int count = data.size();
    if(audio->format().channelCount() == 2){
        count /= 2;
        left  = new char[count];
        right = new char[count];
        for(int i = 0; i < count; i += typeSize){
            for(int j = 0; j < typeSize; ++j){
                left [i+j] = data[i*2+j];
                right[i+j] = data[i*2+j+typeSize];
            }
        }
    } else {
        left = right = data.data();
    }


    shaderProgramMutex.lock();
        shaderProgram->bind();

        glBindTexture(GL_TEXTURE_1D, audioLeftTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, count / typeSize, 0, GL_RED, type, left);

        glBindTexture(GL_TEXTURE_1D, audioRightTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, count / typeSize, 0, GL_RED, type, right);

        shaderProgram->release();
    shaderProgramMutex.unlock();
    if(left != data.data())
        delete[] left;
    if(right != data.data())
        delete[] right;
}

/**
 * @brief Renderer::onMessageLogged
 * @param message Message text
 *
 * Write the message text in the debug output
 */
void Renderer::onMessageLogged(QOpenGLDebugMessage message){
    qDebug() << message;
}

/**
 * @brief Renderer::loadModel
 * @param file Path to file, which contains the model that should be loaded
 * @param offset Model position in 3D-Space
 * @param scaling Model scaling in 3D-Space
 * @param rotation Model rotation in 3D-Space
 * @return True on success, otherwise false.
 */
bool Renderer::loadModel(const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation){
//    if(file == modelFile)
//        return true;

    if(file != modelFile &&  context && context->isValid()){
        shaderProgramMutex.lock();
            shaderProgram->bind();
            bool ok = model.loadModel(file.toStdString(), false);
            shaderProgram->release();
        shaderProgramMutex.unlock();

        if(!ok)
            return false;
    }

    setTitle("ShaderSandbox | " + file);

    modelFile = file;
    modelOffset = offset;
    modelScaling = scaling;
    modelRotation = rotation;

    M = QMatrix4x4();
    M.rotate(rotation.x(), QVector3D(1, 0, 0));
    M.rotate(rotation.y(), QVector3D(0, 1, 0));
    M.rotate(rotation.z(), QVector3D(0, 0, 1));
    M.scale(scaling);
    M.translate(offset);
//    uploadMVP();

    return true;
}

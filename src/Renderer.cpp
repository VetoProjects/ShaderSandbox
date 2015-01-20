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
Renderer::Renderer(QWindow *parent) : Renderer::Renderer("new", defaultVertexShader, defaultFragmentShader, parent){ }

/**
 * @brief Renderer::Renderer
 * @param filename Name that should shown in the title
 * @param instructions Shader code for execution
 * @param parent Parent object of the render window
 *
 * Create a new Renderer with given code and set filename as title
 */
Renderer::Renderer(const QString &path, const QString &vertexShader, const QString &fragmentShader, QWindow *parent) :
    QWindow(parent),
    currentPath(path),
    clearColor(Qt::black),
    context(0), device(0),
    time(0),
    pendingUpdate(false),
    vao(0), uvBuffer(0), audioLeftTexture(0), audioRightTexture(0),
    vertexAttr(0), uvAttr(0), timeUniform(0),
    mID(0), mvID(0), mvpID(0),
    shaderProgram(0),
    vertexSource(vertexShader), fragmentSource(fragmentShader),
    textureRegEx("(^|\n|\r)\\s*#texture\\s+([A-Za-z_][A-Za-z0-9_]*)\\s+([^\n\r]+)")
{
    setTitle(path);

    m_logger = new QOpenGLDebugLogger( this );

    connect(m_logger, SIGNAL(messageLogged(QOpenGLDebugMessage)),
             this, SLOT(onMessageLogged(QOpenGLDebugMessage)),
             Qt::DirectConnection );

    setSurfaceType(QWindow::OpenGLSurface);

    V.lookAt(QVector3D(0, 1, 2), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    P.perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    time = new QTime();
    time->start();

    audio = new AudioInputProcessor(this);
    connect(audio, SIGNAL(processData(QByteArray)), this, SLOT(updateAudioData(QByteArray)));
    audio->start();

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
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
        for(QOpenGLTexture *texture : textures){
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
    glClearColor(0, 0, 0.3, 1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CW);
    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

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
            Q_EMIT errored("Image file does not exist: " + imagePath, fragmentShader.mid(0, pos).count('\n'));
            return false;
        }

        images.append(QPair<QString, QString>(imageName, textureImage.absoluteFilePath()));

        QString textureDefinition(textureRegEx.cap(1) + "uniform sampler2D " + imageName + ";");
        fragmentShader.remove(pos, textureRegEx.matchedLength());
        fragmentShader.insert(pos, textureDefinition);
        pos += textureDefinition.length();
    }
	
    QOpenGLShaderProgram *newShaderProgram = new QOpenGLShaderProgram(this);
    bool hasError = false;
	
    if(!hasError && !newShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader)){
        hasError = true;
    }

    if(!hasError && !newShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,fragmentShader)){
        hasError = true;
    }

    if(!hasError && !newShaderProgram->link()){
        hasError = true;
    }

    if(hasError){
        QString error = newShaderProgram->log();
        delete newShaderProgram;

        //mac  :<line>:
        //mesa :<line>(<errorcode>):
        QRegExp errorline(":([0-9]+)(\\([0-9]+\\))?:");
        if(errorline.indexIn(error) > -1){
            QString text = errorline.cap(1);
            if(text.toInt()-3 > 0)  // because: "#define lowp", "#define mediump" and "#define highp"
                Q_EMIT errored(error, text.toInt()-3);
            else
                Q_EMIT errored(error, text.toInt());
        }

        if(shaderProgram == 0 && (vertexShader != defaultFragmentShader || fragmentShader != defaultFragmentShader)){
            initShaders(defaultVertexShader, defaultFragmentShader);
        } else {
            qWarning() << tr("Failed to compile default shader.");
        }

        return false;
    }

    QList<QOpenGLTexture*> newTextures;
    for(const QPair<QString, QString> image: images){

        QOpenGLTexture* texture = new QOpenGLTexture(QImage(image.second));

        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);

        //qDebug() << imageName << " from " << imagePath << ": " << texture->textureId() << " (" << texture << ")";

        newTextures.append(texture);
    }

    shaderProgramMutex.lock();

        if(shaderProgram){
            shaderProgram->bind();
            for(QOpenGLTexture *texture : textures){
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
        mvID = shaderProgram->uniformLocation("MV");
        mvpID = shaderProgram->uniformLocation("MVP");

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    device->setSize(size());

//    qDebug() << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_VERSION))) << " " << QLatin1String(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    QPoint mouse = this->mapFromGlobal(QCursor::pos());
    QVector2D mousePosition((float)mouse.x() / (float)this->width(),
                            (float)mouse.y() / (float)this->height());
    float ration = ((this->height() == 0) ? 1 : (float)this->width() / (float)this->height());


    shaderProgramMutex.lock();
        shaderProgram->bind();
        vao->bind();
        uploadMVP(); // attention: uncomment mutex => deadlock

// TODO: Find good start M, V and P (M from dialog, V and P by mouse / keyboard control?)
// TODO: Light source


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
    switch(event->type()){
    case QEvent::UpdateRequest:
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
        renderNow();
        return true;
    case QEvent::Close:
        Q_EMIT doneSignal(tr("User closed renderer"));
        return true;
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
bool Renderer::updateCode(const QString &path, const QString &vertCode, const QString &fragCode){
    if(!initShaders(vertCode, fragCode))
        return false;
    currentPath = path;
    setTitle(path);
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

void Renderer::uploadMVP(){
//    if(shaderProgram){
        MV = V * M;
        MVP = P * MV;
//        shaderProgramMutex.lock();
//            shaderProgram->bind();
//            vao->bind();
            glUniformMatrix4fv(pID, 1, GL_FALSE, P.data());
            glUniformMatrix4fv(vID, 1, GL_FALSE, V.data());
            glUniformMatrix4fv(mID, 1, GL_FALSE, M.data());
            glUniformMatrix4fv(mvID, 1, GL_FALSE, MV.data());
            glUniformMatrix4fv(mvpID, 1, GL_FALSE, MVP.data());
//            vao->release();
//            shaderProgram->release();
//        shaderProgramMutex.unlock();
//    }
}

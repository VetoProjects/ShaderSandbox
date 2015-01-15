#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QWindow>
#include <QOpenGLPaintDevice>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLDebugLogger>
#include <QOpenGLTexture>
#include <QTime>
#include <QOpenGLShader>
#include <QCoreApplication>
#include <QDir>
#include <QMutex>

#include "AudioInputProcessor.hpp"
#include "Model3D.hpp"

/**
 * @brief The Renderer class
 *
 * A subclass of QWindow and QOPenGLFunctions that implements
 * a GLSL fragment shader renderer.
 */
class Renderer : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit Renderer(QWindow *parent = 0);
    explicit Renderer(const QString &, const QString &vertexShader, const QString &fragmentShader, QWindow *parent = 0);
    ~Renderer();

Q_SIGNALS:
    void doneSignal(QString);
    void errored(QString, int);

public Q_SLOTS:
    void renderNow();
    void renderLater();
    bool updateCode(const QString &, const QString &, const QString &);
    void updateAudioData(QByteArray);
    void onMessageLogged(QOpenGLDebugMessage message);
    bool loadModel(const QString &, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation);
    void uploadMVP();

protected:
    virtual bool event(QEvent *);
    virtual void exposeEvent(QExposeEvent *);

private:
    bool init();
    void render();
    bool initShaders(QString, QString);
    QString currentPath;
    QColor clearColor;
    QOpenGLContext *context;
    QOpenGLPaintDevice *device;
    QTime *time;
    bool pendingUpdate;

    QOpenGLVertexArrayObject *vao;
    GLuint vertexBuffer, uvBuffer, audioLeftTexture, audioRightTexture;
    GLint vertexAttr, uvAttr,
        timeUniform, mouseUniform, rationUniform, samplerLeft, samplerRight,
        pID, vID, mID, mvID, mvpID;
    QOpenGLShaderProgram *shaderProgram;
    QMutex shaderProgramMutex;
    QString vertexSource, fragmentSource;
    QList<QOpenGLTexture*> textures;
    QString modelFile;
    QVector3D modelOffset, modelScaling, modelRotation;
    Model3D model;
    QMatrix4x4 P, V, M, MV, MVP;

    AudioInputProcessor *audio;

    QOpenGLDebugLogger* m_logger;

    QRegExp textureRegEx;

//    static bool mapFormat(float *target, char *source, int count, const QAudioFormat &format);
//    template <typename T>
//    static void convertArray(float *target, const T *source, int count, qreal div, qreal sub = 0);
    static const QString defaultVertexShader, defaultFragmentShader;
};

#endif // RENDERER_HPP

#ifndef LIVETHREAD
#define LIVETHREAD

#include <QThread>

#include "Renderer.hpp"

/**
 * @brief The LiveThread class
 *
 * A subclass of QThread that is optimized for
 * running the interpreters we need.
 */
class LiveThread : public QThread{
    Q_OBJECT
public:
    LiveThread(const long identity, QObject* parent = 0): QThread(parent), ID(identity){}
    virtual void run() = 0;
    virtual void initialize(const QString &title, const QString &vertexShader, const QString &fragmentShader) = 0;
    virtual bool updateCode(const QString &title, const QString &vertexShader, const QString &fragmentShader) = 0;
    virtual bool loadModel(const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation) = 0;
    const long ID;
private:
    LiveThread& operator=(const LiveThread& rhs);
    LiveThread& operator=(LiveThread&& rhs);
};

class GlLiveThread: public LiveThread{
    Q_OBJECT
public:
    GlLiveThread(const long identity, QObject* parent = 0) : LiveThread(identity, parent){
        runObj = 0;
    }
    ~GlLiveThread(){
        if(runObj)
            delete runObj;
    }
    void run() Q_DECL_OVERRIDE{
        if(runObj)
            runObj->show();
    }
    // No parent object =(
    void initialize(const QString &title, const QString &vertexShader, const QString &fragmentShader){
        runObj = new Renderer(title, vertexShader, fragmentShader);
        connect(runObj, SIGNAL(doneSignal(QString)), this, SLOT(doneSignalReceived(QString)));
        connect(runObj, SIGNAL(errored(QString,int)), this, SLOT(erroredReceived(QString, int)));

        runObj->resize(800, 600);
        runObj->show();

    }
    bool updateCode(const QString &filename, const QString &vertexShader, const QString &fragmentShader){
        if(runObj)
            return runObj->updateCode(filename, vertexShader, fragmentShader);
        return false;
    }
    bool loadModel(const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation){
        qDebug() << "Thread";
        return runObj->loadModel(file, offset, scaling, rotation);
    }

public Q_SLOTS:
    void doneSignalReceived(QString exception){
        Q_EMIT doneSignal(this, exception);
    }
    void erroredReceived(QString error, int lineno){
        Q_EMIT errorSignal(this, error, lineno);
    }
Q_SIGNALS:
    void doneSignal(GlLiveThread*, QString);
    void errorSignal(GlLiveThread*, QString, int);
private:
    Renderer* runObj;
};

#endif // LIVETHREAD

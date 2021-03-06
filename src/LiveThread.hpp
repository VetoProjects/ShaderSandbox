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
    virtual void initialize(Renderer* renderer) = 0;
    virtual bool updateCode(const QString &vertexShader, const QString &fragmentShader) = 0;
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

    void run()  noexcept Q_DECL_OVERRIDE{
        /*if(runObj)
                runObj->show();*/
        exec();
    }
    // No parent object =(
    void initialize(Renderer* renderer) noexcept Q_DECL_OVERRIDE{
        runObj = renderer;

        connect(runObj, &Renderer::doneSignal, [=](QString msg){
            Q_EMIT doneSignal(this, msg);
        });
        connect(runObj, &Renderer::errored, [=](QString msg){
            Q_EMIT errorSignal(this, msg);
        });
        connect(runObj, &Renderer::vertexError, [=](QString msg, int line){
            Q_EMIT vertexError(this, msg, line);
        });
        connect(runObj, &Renderer::fragmentError, [=](QString msg, int line){
            Q_EMIT fragmentError(this, msg, line);
        });
    }
    bool updateCode(const QString &vertexShader, const QString &fragmentShader) noexcept{
        return runObj && runObj->updateCode(vertexShader, fragmentShader);
    }
    bool loadModel(const QString &file, const QVector3D &offset, const QVector3D &scaling, const QVector3D &rotation) noexcept{
        return runObj && runObj->loadModel(file, offset, scaling, rotation);
    }
Q_SIGNALS:
    void doneSignal(GlLiveThread*, QString);
    void errorSignal(GlLiveThread*, QString);
    void vertexError(GlLiveThread*, QString, int);
    void fragmentError(GlLiveThread*, QString, int);
private:
    Renderer* runObj;
};

#endif // LIVETHREAD

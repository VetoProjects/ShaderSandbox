#ifndef LIVETHREAD
#define LIVETHREAD

#ifdef WITH_PYTHON
#include "PyLiveInterpreter.hpp"
#include "PySoundGenerator.hpp"
#endif

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
    virtual void initialize(const QString &title, const QString &instructions) = 0;
    virtual bool updateCode(const QString &title, const QString &instructions) = 0;
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
    void initialize(const QString &title, const QString &instructions){
        runObj = new Renderer(title, instructions);
        connect(runObj, SIGNAL(doneSignal(QString)), this, SLOT(doneSignalReceived(QString)));
        connect(runObj, SIGNAL(errored(QString,int)), this, SLOT(erroredReceived(QString, int)));

        runObj->resize(800, 600);
        runObj->show();

    }
    bool updateCode(const QString &filename, const QString &code){
        if(runObj)
            return runObj->updateCode(filename, code);
        return false;
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

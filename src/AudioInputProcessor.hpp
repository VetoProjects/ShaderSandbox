#ifndef AUDIOINPUTPROCESSOR_HPP
#define AUDIOINPUTPROCESSOR_HPP

#include <memory>

#include <QIODevice>
#include <QDebug>
#include <QAudioInput>

class AudioInputProcessor : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioInputProcessor(QObject *parent = 0);
    void start() noexcept;
    const QAudioFormat format() const noexcept;

Q_SIGNALS:
    void processData(QByteArray);


private:
    std::unique_ptr<QAudioInput> input;
//    QMutex dataAccess;
//    char *data;


    // QIODevice interface
protected:
    virtual qint64 readData(char *data, qint64 maxlen) noexcept;
    virtual qint64 writeData(const char *data, qint64 len) noexcept;
};

#endif // AUDIOINPUTPROCESSOR_HPP

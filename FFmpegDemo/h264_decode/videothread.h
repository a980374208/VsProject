#ifndef AUDIOTHREAD_H
#define AUDIOTHREAD_H


#include <QThread>

class VideoThread : public QThread {
    Q_OBJECT
private:
    void run();

public:
    explicit VideoThread(QObject *parent = nullptr);
    ~VideoThread();
signals:
    void decoder_finished();
};


#endif // AUDIOTHREAD_H

#include "audiothread.h"
#include "ffmpegs.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}


AudioThread::AudioThread(QObject *parent) : QThread(parent) {
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "release AudioThread";
}

void AudioThread::run() {
    VideoDecodeSpec out;
    out.filename = "E:/decode.yuv";

    FFmpegs::h264Decode("E:/nv12.264", out);

    qDebug() << "width:"<< out.width << "height:" << out.height
             << "fps:" << out.fps << "fmt:"<< av_get_pix_fmt_name(out.pixFmt);
}

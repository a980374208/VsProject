#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

// 输入缓冲区的大小
#define IN_DATA_SIZE 4096

FFmpegs::FFmpegs() {

}

static int frameIdx = 0;
static int decode(AVCodecContext *ctx,
                  AVPacket *pkt,
                  AVFrame *frame,
                  QFile &outFile) {
    // 发送压缩数据到解码器
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    while (true) {
        // 获取解码后的数据
        ret = avcodec_receive_frame(ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            ERROR_BUF(ret);
            qDebug() << "avcodec_receive_frame error" << errbuf;
            return ret;
        }
        qDebug() << "parser index:" << ++frameIdx << "frame";

       //// 将解码后的数据写入文件
       //// 代码假设图像数据是连续的，并且没有考虑可能存在的行对齐（即 linesize 可能大于 width）。在实际的 YUV420p 数据中，每行的 U 和 V 分量的实际字节数可能小于 width / 2，因为通常会存在对齐。
       //// // 写入Y平面
       ////outFile.write((char *) frame->data[0] ,
       ////                   frame->width * frame->height);
       //// // 写入U平面
       ////outFile.write((char *) frame->data[1] ,
       ////                   frame->width * frame->height / 4);
       //// // 写入V平面
       ////outFile.write((char *) frame->data[2],
       ////                   frame->width * frame->height / 4);
       // 
       //// 写入Y平面
       //for (int y = 0; y < frame->height; y++) {
       //    outFile.write((char*)frame->data[0] + y * frame->linesize[0], frame->width);
       //}
       //// 写入U平面
       //for (int y = 0; y < frame->height / 2; y++) {
       //    outFile.write((char*)frame->data[1] + y * frame->linesize[1], frame->width / 2);
       //}
       //// 写入V平面
       //for (int y = 0; y < frame->height / 2; y++) {
       //    outFile.write((char*)frame->data[2] + y * frame->linesize[2], frame->width / 2);
       //}
       // qDebug() << "pix_fmt:" << ctx->pix_fmt;

         //将源图片格式转为nv12格式
        static bool first = true;
        static struct SwsContext* sws_ctx;
        static AVFrame* frame_nv12 = av_frame_alloc();
        if (first) {
            first = false;
            sws_ctx = sws_getContext(
                frame->width, frame->height, ctx->pix_fmt,
                frame->width, frame->height, AV_PIX_FMT_NV12,
                SWS_BILINEAR, NULL, NULL, NULL
            );
            frame_nv12->format = AV_PIX_FMT_NV12;
            frame_nv12->width = frame->width;
            frame_nv12->height = frame->height;
            av_frame_get_buffer(frame_nv12, 0);
        }
        sws_scale(sws_ctx,
            (const uint8_t* const*)frame->data, frame->linesize,
            0, frame->height,
            frame_nv12->data, frame_nv12->linesize
        );
     // 写入Y平面
     for (int y = 0; y < frame_nv12->height; y++) {
         outFile.write((char*)frame_nv12->data[0] + y * frame_nv12->linesize[0], frame_nv12->width);
     }
     // 写入UV平面
     for (int y = 0; y < frame_nv12->height/2; y++) {
         outFile.write((char*)frame_nv12->data[1] + y * frame_nv12->linesize[1], frame_nv12->width);
     }
    }
}

void FFmpegs::h264Decode(const char *inFilename,
                         VideoDecodeSpec &out) {
    // 返回结果
    int ret = 0;

    // 用来存放读取的输入文件数据（h264）
    // 加上AV_INPUT_BUFFER_PADDING_SIZE是为了防止某些优化过的reader一次性读取过多导致越界
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = inDataArray;

    // 每次从输入文件中读取的长度（h264）
    // 输入缓冲区中，剩下的等待进行解码的有效数据长度
    int inLen;
    // 是否已经读取到了输入文件的尾部
    int inEnd = 0;

    // 文件
    QFile inFile(inFilename);
    QFile outFile(out.filename);

    // 解码器
    const AVCodec *codec = nullptr;
    // 上下文
    AVCodecContext *ctx = nullptr;
    // 解析器上下文
    AVCodecParserContext *parserCtx = nullptr;

    // 存放解码前的数据(h264)
    AVPacket *pkt = nullptr;
    // 存放解码后的数据(yuv)
    AVFrame *frame = nullptr;

    // 获取解码器
    //codec = avcodec_find_encoder_by_name("h264_qsv");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qDebug() << "decoder not found";
        return;
    }

    // 初始化解析器上下文
    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        qDebug() << "av_parser_init error";
        return;
    }

    // 创建上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // 打开解码器
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    // 读取文件数据
    do {
        inLen = inFile.read(inDataArray, IN_DATA_SIZE);
        // 设置是否到了文件尾部
        inEnd = !inLen;

        // 让inData指向数组的首元素
        inData = inDataArray;

        // 只要输入缓冲区中还有等待进行解码的数据
        while (inLen > 0 || inEnd) {
            // 到了文件尾部（虽然没有读取任何数据，但也要调用av_parser_parse2，修复bug）

            // 经过解析器解析
            ret = av_parser_parse2(parserCtx, ctx,
                                   &pkt->data, &pkt->size,
                                   (uint8_t *) inData, inLen,
                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            
            
            if (ret < 0) {
                ERROR_BUF(ret);
                qDebug() << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            // 跳过已经解析过的数据
            inData += ret;
            // 减去已经解析过的数据大小
            inLen -= ret;

            qDebug() << inEnd << pkt->size << ret;

            // 解码
            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
                goto end;
            }

            // 如果到了文件尾部
            if (inEnd) break;
        }
    } while (!inEnd);

    // 刷新缓冲区
    decode(ctx, nullptr, frame, outFile);

    // 赋值输出参数
    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    // 用framerate.num获取帧率，并不是time_base.den
    out.fps = ctx->framerate.num;

end:
    inFile.close();
    outFile.close();
    av_packet_free(&pkt);
    av_frame_free(&frame);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);
}



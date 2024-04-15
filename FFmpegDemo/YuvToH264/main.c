#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>

int64_t get_time() {
    return av_gettime_relative() / 1000;  // ����ɺ���
}

static int encode(AVCodecContext* enc_ctx, AVFrame* frame, AVPacket* pkt,
    FILE* outfile) {
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);
    /* ͨ�����Ĵ��룬ʹ��x264���б���ʱ�����建��֡����x264Դ����У�
     * ��������avframe��Ӧbuffer��reference*/
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return -1;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            return -1;
        }

        if (pkt->flags & AV_PKT_FLAG_KEY)
            printf("Write packet flags:%d pts:%3"PRId64" dts:%3"PRId64" (size:%5d)\n",
                pkt->flags, pkt->pts, pkt->dts, pkt->size);
        if (!pkt->flags)
            printf("Write packet flags:%d pts:%3"PRId64" dts:%3"PRId64" (size:%5d)\n",
                pkt->flags, pkt->pts, pkt->dts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
    }
    return 0;
}


int main(int argc, char** argv) {
    char* in_yuv_file = NULL;
    char* out_h264_file = NULL;
    FILE* infile = NULL;
    FILE* outfile = NULL;

    const char* codec_name = NULL;
    const AVCodec* codec = NULL;
    AVCodecContext* codec_ctx = NULL;
    AVFrame* frame = NULL;
    AVPacket* pkt = NULL;
    int ret = 0;

    in_yuv_file = "E:/yuv420p_1920x1080.yuv";      // ����YUV�ļ�
    //out_h264_file = "E:/x264.h264";
    //codec_name = "libx264";
    out_h264_file = "E:/x264.h264";
    codec_name = "h264_nvenc";

    /* ����ָ���ı����� */
    codec = avcodec_find_encoder_by_name(codec_name);
    if (!codec) {
        fprintf(stderr, "Codec '%s' not found\n", codec_name);
        exit(1);
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }


    /* ���÷ֱ���*/
    codec_ctx->width = 1920;
    codec_ctx->height = 1080;
    /* ����time base */
    codec_ctx->time_base = (AVRational){ 1, 25 };
    codec_ctx->framerate = (AVRational){ 25, 1 };
    /* ����I֡���
     * ���frame->pict_type����ΪAV_PICTURE_TYPE_I, �����gop_size�����ã�һֱ����I֡���б���
     */
    codec_ctx->gop_size = 25;   // I֡���
    codec_ctx->max_b_frames = 2; // ����������B֡������Ϊ0
    codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    //
    if (codec->id == AV_CODEC_ID_H264) {
        // ��صĲ������Բο�libx264.c�� AVOption options
        ret = av_opt_set(codec_ctx->priv_data, "preset", "medium", 0);
        if (ret != 0) {
            printf("av_opt_set preset failed\n");
        }
        ret = av_opt_set(codec_ctx->priv_data, "profile", "main", 0); // Ĭ����high
        if (ret != 0) {
            printf("av_opt_set profile failed\n");
        }
    }

    /*
     * ���ñ���������
    */
    /* ����bitrate */
    codec_ctx->bit_rate = 3000000;

    /* ��codec_ctx��codec���а� */
    ret = avcodec_open2(codec_ctx, codec, NULL);
    if (ret < 0) {
        fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
        exit(1);
    }
    printf("thread_count: %d, thread_type:%d\n", codec_ctx->thread_count, codec_ctx->thread_type);
    // �����������ļ�
    infile = fopen(in_yuv_file, "rb");
    if (!infile) {
        fprintf(stderr, "Could not open %s\n", in_yuv_file);
        exit(1);
    }
    outfile = fopen(out_h264_file, "wb");
    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", out_h264_file);
        exit(1);
    }

    // ����pkt��frame
    pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    // Ϊframe����buffer
    frame->format = codec_ctx->pix_fmt;
    frame->width = codec_ctx->width;
    frame->height = codec_ctx->height;
    //ret = avformat_write_header(codec_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "avformat_write_header write error\n");
    }
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }
    // �����ÿһ֡������ ���ظ�ʽ * �� * ��
    // 1382400
    int frame_bytes = av_image_get_buffer_size(frame->format, frame->width,
        frame->height, 1);
    printf("frame_bytes %d\n", frame_bytes);
    uint8_t* yuv_buf = (uint8_t*)malloc(frame_bytes);
    if (!yuv_buf) {
        printf("yuv_buf malloc failed\n");
        return 1;
    }
    int64_t begin_time = get_time();
    int64_t end_time = begin_time;
    int64_t all_begin_time = get_time();
    int64_t all_end_time = all_begin_time;
    int64_t pts = 0;
    printf("start enode\n");
    for (;;) {
        memset(yuv_buf, 0, frame_bytes);
        size_t read_bytes = fread(yuv_buf, 1, frame_bytes, infile);
        if (read_bytes <= 0) {
            printf("read file finish\n");
            break;
        }
        /* ȷ����frame��д, ����������ڲ��������ڴ�ο�����������Ҫ���¿���һ������
            Ŀ������д������ݺͱ�������������ݲ��ܲ�����ͻ
        */
        int frame_is_writable = 1;
        if (av_frame_is_writable(frame) == 0) { // ����ֻ����������
            printf("the frame can't write, buf:%p\n", frame->buf[0]);
            if (frame->buf && frame->buf[0])        // ��ӡreferenc-counted�����뱣֤���������Чָ��
                printf("ref_count1(frame) = %d\n", av_buffer_get_ref_count(frame->buf[0]));
            frame_is_writable = 0;
        }
        ret = av_frame_make_writable(frame);
        if (frame_is_writable == 0) {  // ����ֻ����������
            printf("av_frame_make_writable, buf:%p\n", frame->buf[0]);
            if (frame->buf && frame->buf[0])        // ��ӡreferenc-counted�����뱣֤���������Чָ��
                printf("ref_count2(frame) = %d\n", av_buffer_get_ref_count(frame->buf[0]));
        }
        if (ret != 0) {
            printf("av_frame_make_writable failed, ret = %d\n", ret);
            break;
        }
        int need_size = av_image_fill_arrays(frame->data, frame->linesize, yuv_buf,
            frame->format,
            frame->width, frame->height, 1);
        if (need_size != frame_bytes) {
            printf("av_image_fill_arrays failed, need_size:%d, frame_bytes:%d\n",
                need_size, frame_bytes);
            break;
        }
        pts += 40;
        // ����pts
        frame->pts = pts;       // ʹ�ò�������Ϊpts�ĵ�λ�����廻����� pts*1/������
        begin_time = get_time();
        ret = encode(codec_ctx, frame, pkt, outfile);
        end_time = get_time();
        printf("encode time:%lldms\n", end_time - begin_time);
        if (ret < 0) {
            printf("encode failed\n");
            break;
        }
    }
    encode(codec_ctx, NULL, pkt, outfile);
    all_end_time = get_time();
    printf("all encode time:%lldms\n", all_end_time - all_begin_time);
    // �ر��ļ�
    fclose(infile);
    fclose(outfile);

    // �ͷ��ڴ�
    if (yuv_buf) {
        free(yuv_buf);
    }

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);

    printf("main finish, please enter Enter and exit\n");
    getchar();
    return 0;
}

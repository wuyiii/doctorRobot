#ifndef DECODER_H
#define DECODER_H

#include <QThread>
#include <QImage>
#include <QFile>

extern "C"
{
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

class VPU;

class DecodeThread : public QThread
{
    Q_OBJECT

public:
    void decoder_close();
    int  decoder_setup();
    int  decoder_process();
    int  decoder_filldata(unsigned long len);

protected:
    void run();

signals:
    void get_frame(char *dat, int w, int h);

public slots:
    void end();
    void set_url(const char *s);
    void set_outfile(char *s);

private:
    AVFormatContext *fmtctx = NULL;
    enum AVPixelFormat srcfmt = AV_PIX_FMT_NONE;
    enum AVPixelFormat dstfmt = AV_PIX_FMT_RGB24;
    AVPacket *pkt = NULL;
    AVFrame *frm = NULL;
    AVCodecID decde_id = AV_CODEC_ID_NONE;

    int dst_buff_size;
    uint8_t *dst_data[4], *src_data[4];
    int32_t src_linesize[4], dst_linesize[4], video_id=0;
    uint32_t src_h, src_w, dst_h, dst_w;
    uint8_t url[100];

#define NET_DATA_POOL 200000
    char *npool = NULL;
    uint32_t nremain = 0;
    uint32_t noffset = 0;

    QImage img;
    QFile outfile;
    VPU *vpu;

    bool isend = false;
};

#endif // DECODER_H

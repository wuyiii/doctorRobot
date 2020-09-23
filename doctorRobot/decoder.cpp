#include <QFile>
#include <vpu.h>
#include <stdio.h>
#include <decoder.h>

#define STREAM_READ_SIZE 100000

int DecodeThread::decoder_setup()
{
    int ret;
    uint32_t i;
    AVCodecParameters *codecpar = NULL;
    AVDictionary* options = NULL;

    av_dict_set(&options, "buffer_size", "1024000", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    //打开 rtsp://...
    ret = avformat_open_input(&fmtctx, (const char *)url, NULL, &options);
    if(ret < 0){
        qDebug("can't open input stream errno: %d %s", ret, strerror(errno));
        return -1;
    }

    qDebug("%s url %s",qPrintable(this->objectName()), fmtctx->url);

    //分析媒体流的信息并绑定到上下文
    ret = avformat_find_stream_info(fmtctx, NULL);
    if(ret < 0){
        qDebug("can't find steam info");
        return -1;
    }

    //媒体流中可能包含视频、音频、字母等，这里只使用视频
    for(i=0; i<fmtctx->nb_streams; i++)
        if(fmtctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_id = i;
            break;
        }

    //记录视频的宽高
    codecpar = fmtctx->streams[video_id]->codecpar;
    src_h = codecpar->height;
    src_w = codecpar->width;
    srcfmt = (enum AVPixelFormat)codecpar->format;

    //只是分配了结构体大小，并没有分配数据的空间
    pkt = av_packet_alloc();
    if(pkt == NULL){
        qDebug("can't alloc AVPacket");
        return -1;
    }

    //只是分配了结构体大小，并没有分配数据的空间
    frm = av_frame_alloc();
    if(frm == NULL){
        qDebug("can't alloc AVFrame1");
        return -1;
    }

    dst_w = src_w;
    dst_h = src_h;

    vpu = new VPU;
    vpu->open();

    npool = (char *)malloc(NET_DATA_POOL);
    if(!npool){
        qDebug("malloc for net pool error");
        return -1;
    }

    return 0;
}

int DecodeThread::decoder_process()
{
    bool flag;
    int ret;
    void *imgaddr = NULL;
    DecOutputInfo info;

    decoder_filldata(STREAM_READ_SIZE);
    flag = vpu->parse();
    if(!flag){
        qDebug("parse bitstream error");
        return -1;
    }

    flag = vpu->alloc_framebuffer();
    if(!flag){
        qDebug("alloc frame buffer error");
        return -1;
    }

    qDebug("vpu would start decode");

    while(1){
        flag = vpu->start_decode_oneframe();
        if(!flag){
            qDebug("start one frame error");
            return -1;
        }

        while(vpu->is_busy()){
            ret = decoder_filldata(STREAM_READ_SIZE);
            if(ret < 0)
                return -1;
        }

        flag = vpu->get_output_info(&info);
        if(!flag){
            qDebug("get output info error");
            return false;
        }

//        qDebug("decidx %d, disidx %d, decodongSuccess 0x%x",
//               info.indexFrameDecoded, info.indexFrameDisplay, info.decodingSuccess);

        if(info.decodingSuccess == 0){

            continue;
        }

        if(info.decodingSuccess & 0x10){
            ret = decoder_filldata(STREAM_READ_SIZE);
            if(ret < 0)
                return 0;

            continue;
        }

        if(info.decodingSuccess & 0x100000)
            qDebug("sequence parameter has been changed");

        if(info.notSufficientPsBuffer){
            qDebug("PS buffer overflow");
            return -1;
        }

        if(info.notSufficientSliceBuffer){
            qDebug("Slice buffer overflow");
            return -1;
        }

        if(info.indexFrameDisplay == -1)
            isend = true;

        if(info.indexFrameDisplay >= 0){
            imgaddr = vpu->convert_pixel(info.indexFrameDisplay);
            if(imgaddr == NULL){
                qDebug("convert pixel error");
                return -1;
            }

//            emit get_frame((char *)imgaddr, dst_w, dst_h);
            vpu->clear_disp_flag(info.indexFrameDisplay);
        }

        if(isend){
            isend = false;
            break;
        }
    }

    //正常情况不执行到这里
    return 0;
}

int DecodeThread::decoder_filldata(unsigned long len)
{
    bool flag;
    int ret;
    qint32 wsize;
    unsigned long read_pos, write_pos, space;

    flag = vpu->get_stream_buff_info(&read_pos, &write_pos, &space);
    if(!flag){
        qDebug("can't get stream buff info");
        return -1;
    }

    if(space < len)
        return 0;

    if(nremain > 0){
        if(nremain > len){
            flag = vpu->fillbuff((unsigned long)(npool+noffset), len);
            if(!flag){
                qDebug("fill buffer error");
                return -1;
            }
            nremain -= len;
            noffset += len;
            return 0;
        }else {
            flag = vpu->fillbuff((unsigned long)(npool+noffset), nremain);
            if(!flag){
                qDebug("fill buffer error");
                return -1;
            }
            len -= nremain;
            nremain = 0;
            noffset = 0;
        }
    }

    while((ret = av_read_frame(fmtctx, pkt)) == 0)
        if(pkt->stream_index == video_id)
            break;

    if((unsigned)pkt->size > len){
        wsize = len;
        if((pkt->size - len) > NET_DATA_POOL){
            av_packet_unref(pkt);
            return 0;
        }
        nremain = pkt->size - len;
        memcpy((void *)npool, (void *)(pkt->data + len), nremain);
    }else
        wsize = pkt->size;

    flag = vpu->fillbuff((unsigned long)pkt->data, wsize);
    if(!flag){
        qDebug("fill buffer error");
        return -1;
    }

    av_packet_unref(pkt);

    return 0;
}

void DecodeThread::set_url(const char *s)
{
    strcpy((char *)url, s);
}

void DecodeThread::set_outfile(char *s)
{
    outfile.setFileName(s);
}

void DecodeThread::run()
{
    int ret;

    ret = decoder_setup();
    if(ret < 0){
        qDebug("decoder setup meet a trouble");
        return;
    }

    ret = decoder_process();
    if(ret < 0){
        qDebug("decoder process meet a trouble");
        return;
    }else
        qDebug("%s process complete",qPrintable(this->objectName()));

    decoder_close();
}

void DecodeThread::end()
{
    isend = true;
}

void DecodeThread::decoder_close()
{
    vpu->close();
    av_frame_unref(frm);
    avformat_close_input(&fmtctx);
}

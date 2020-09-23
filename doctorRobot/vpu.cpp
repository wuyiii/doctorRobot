#include <vpu.h>

#define STREAM_BUF_SIZE 0x200000
#define PS_SAVE_SIZE    0x080000

void VPU::vpu_init()
{
    RetCode ret;

    ret = vpu_Init(NULL);
    if(ret != RETCODE_SUCCESS){
        qDebug("init vpu error");
        return;
    }
}

void VPU::vpu_uninit()
{
    vpu_UnInit();
}

bool VPU::open()
{
    int err = -1;
    RetCode ret;
    vpu_versioninfo v;

    ret = vpu_GetVersionInfo(&v);
    if(ret == RETCODE_FAILURE){
        qDebug("can't get vpu version [ret:%d]", ret);
        return false;
    }

    qDebug("VPU version %d.%d.%d_r%d", v.fw_major, v.fw_minor, v.fw_release, v.fw_code);

    vpu_mem = (vpu_mem_desc *)malloc(sizeof(vpu_mem_desc));
    if(!vpu_mem){
        qDebug("can't malloc vpu_mem");
        return false;
    }

    ps_mem = (vpu_mem_desc *)malloc(sizeof(vpu_mem_desc));
    if(!ps_mem){
        qDebug("can't malloc ps_mem");
        return false;
    }

    slice_mem = (vpu_mem_desc *)malloc(sizeof(vpu_mem_desc));
    if(!slice_mem){
        qDebug("can't malloc slice mem");
        return false;
    }

    bzero(vpu_mem, sizeof(vpu_mem_desc));
    bzero(ps_mem, sizeof(vpu_mem_desc));
    bzero(slice_mem, sizeof(vpu_mem_desc));

    vpu_mem->size = STREAM_BUF_SIZE;
    ps_mem->size = PS_SAVE_SIZE;

    err = IOGetPhyMem(vpu_mem);
    if(err == -1)
        return false;

    err = IOGetVirtMem(vpu_mem);
    if(err == -1)
        return false;

    err = IOGetPhyMem(ps_mem);
    if(err == -1)
        return false;

    err = IOGetVirtMem(ps_mem);
    if(err == -1)
        return false;

    open_param = (DecOpenParam *)malloc(sizeof(DecOpenParam));
    if(!open_param){
        qDebug("can't malloc open_param");
        return false;
    }

    bzero(open_param, sizeof(DecOpenParam));

    open_param->bitstreamBuffer = vpu_mem->phy_addr;
    open_param->bitstreamBufferSize = STREAM_BUF_SIZE;
    open_param->pBitStream = (Uint8 *)vpu_mem->virt_uaddr;
    open_param->psSaveBuffer = ps_mem->phy_addr;
    open_param->psSaveBufferSize = PS_SAVE_SIZE;

    open_param->bitstreamFormat = STD_AVC;
    open_param->reorderEnable = 1;
    open_param->chromaInterleave = 1;
    open_param->bitstreamMode = 1;
    open_param->jpgLineBufferMode = 0;

    ret = vpu_DecOpen(&dec, open_param);
    if(ret != RETCODE_SUCCESS){
        qDebug("can't open decoder [ret:%d]", ret);
        return false;
    }

    ipufd = ipu_open();
    if(ipufd < 0){
        qDebug("open ipudev error");
        return false;
    }

    return true;
}

void VPU::close()
{
    ipu_free_task(task);
    ipu_close(ipufd);
    ipu_free_fb(ipufd, inbuf, fbcount);
    ipu_free_fb(ipufd, outbuf, 1);

    vpu_DecClose(dec);

    if(bufinfo)
        free(bufinfo);
    if(fb)
        free(fb);
    if(slice_mem)
        free(slice_mem);
    if(initinfo)
        free(initinfo);
    if(open_param)
        free(open_param);
    if(ps_mem)
        free(ps_mem);
    if(vpu_mem)
        free(vpu_mem);
}

bool VPU::fillbuff(quint32 addr, quint32 len)
{
    RetCode ret;
    unsigned long read_pos,write_pos, avil_size;
    quint32 target_addr, room, left;

    ret = vpu_DecGetBitstreamBuffer(dec, &read_pos,&write_pos, &avil_size);
    if(ret != RETCODE_SUCCESS)
        return false;

    if(avil_size <= 0)
        return false;

    if(len > avil_size)
        return true;

    target_addr = vpu_mem->virt_uaddr + (write_pos - vpu_mem->phy_addr);

    if( (target_addr + len) > (vpu_mem->virt_uaddr + STREAM_BUF_SIZE)){
        room = vpu_mem->virt_uaddr + STREAM_BUF_SIZE - target_addr;
        left = len - room;
        memcpy((void *)target_addr, (void *)addr, room);
        memcpy((void *)vpu_mem->virt_uaddr, (void *)(addr + room), left);
    }else
        memcpy((void *)target_addr, (void *)addr, len);

    ret = vpu_DecUpdateBitstreamBuffer(dec, len);
    if(ret != RETCODE_SUCCESS)
        return false;

    return true;
}

bool VPU::parse()
{
    RetCode ret;
    int err, align, extended_fbcount;

    initinfo = (DecInitialInfo *)malloc(sizeof(DecInitialInfo));
    if(!initinfo){
        qDebug("malloc initinfo error");
        return false;
    }
    bzero(initinfo, sizeof(DecInitialInfo));

    vpu_DecSetEscSeqInit(dec, 1);
    ret = vpu_DecGetInitialInfo(dec, initinfo);
    vpu_DecSetEscSeqInit(dec, 0);
    if(ret != RETCODE_SUCCESS){
        qDebug("vpu_DecGetInitialInfo error [ret:%d]", ret);
        return false;
    }

    if(initinfo->streamInfoObtained){
        qDebug("H.264 Profile: %d Level: %d Interlace: %d",
               initinfo->profile, initinfo->level, initinfo->interlace);

        if(initinfo->aspectRateInfo){
            int aspect_ratio_idc;

            if((initinfo->aspectRateInfo >> 16) == 0){
                aspect_ratio_idc = (initinfo->aspectRateInfo & 0xFF);
                qDebug("aspect_ratio_idc: %d", aspect_ratio_idc);
            }
        }
    }

    qDebug("Decoder: width: %d, height: %d, frameRateRes = %lu, frameRateDiv = %lu, count = %u, mjpg_srcfmt = %d",
           initinfo->picWidth, initinfo->picHeight, initinfo->frameRateRes, initinfo->frameRateDiv, initinfo->minFrameBufferCount, initinfo->mjpg_sourceFormat);

    if(initinfo->interlace)
        extended_fbcount = 4;
    else
        extended_fbcount = 2;

    fbcount = initinfo->minFrameBufferCount + extended_fbcount;

    width = ((initinfo->picWidth + 15) & ~15);

    align = 16;
    if(initinfo->interlace == 1)
        align = 32;

    height = ((initinfo->picHeight + align - 1) & ~(align - 1));
    stride = width;

    slice_mem->size = initinfo->worstSliceSize * 1024;
    err = IOGetPhyMem(slice_mem);
    if(err == -1)
        return false;

    err = IOGetVirtMem(slice_mem);
    if(err == -1)
        return false;

    task = ipu_alloc_task(width, height, width, height);
    if(!task){
        qDebug("ipu init task error");
        return false;
    }

    return true;
}

bool VPU::get_stream_buff_info(unsigned long *read_ptr, unsigned long *write_ptr, unsigned long *space)
{
    RetCode ret;

    ret = vpu_DecGetBitstreamBuffer(dec, read_ptr, write_ptr, space);
    if(ret != RETCODE_SUCCESS)
        return false;

    return true;
}

bool VPU::alloc_framebuffer()
{
    RetCode ret;
    int divX, divY, err;

    divX = 2;
    divY = 2;

    fb = (FrameBuffer *)calloc(fbcount, sizeof(FrameBuffer));
    if(!fb){
        qDebug("can't malloc framebuffer");
        return false;
    }
    bzero(fb, fbcount * sizeof(FrameBuffer));

    for(int i = 0; i < fbcount; i++){
        inbuf[i].size = stride * height + stride / divX * height / divY * 2;
    }

    err = ipu_alloc_fb(ipufd, inbuf, fbcount);
    if(err < 0){
        qDebug("ipu alloc input framebuffer error");
        return false;
    }

    mvcol_mem = (vpu_mem_desc *)calloc(fbcount, sizeof(vpu_mem_desc));
    if(!mvcol_mem){
        qDebug("can't calloc mvcol_mem");
        return false;
    }
    bzero(mvcol_mem, sizeof(vpu_mem_desc) * fbcount);

    for(int i = 0; i < fbcount; i++){
        fb[i].myIndex = i;
        fb[i].bufY = inbuf[i].ipu_paddr;
        fb[i].bufCb = fb[i].bufY + stride * height;
        fb[i].bufCr = fb[i].bufCb + stride / divX * height / divY;

        mvcol_mem[i].size = stride * height / divX / divY;
        err = IOGetPhyMem(&mvcol_mem[i]);
        if(err == -1)
            return false;

        fb[i].bufMvCol = mvcol_mem[i].phy_addr;
    }

    outbuf[0].size = width * height * 3;
    err = ipu_alloc_fb(ipufd, outbuf, 1);
    if(err < 0){
        qDebug("ipu alloc output framebuffer error");
        return false;
    }

    task->output.paddr = outbuf[0].ipu_paddr;

    stride = ((stride + 15) & ~15);

    bufinfo = (DecBufInfo *)malloc(sizeof(DecBufInfo));
    if(!bufinfo){
        qDebug("can't malloc buffer info");
        return false;
    }
    bzero(bufinfo, sizeof(DecBufInfo));

    bufinfo->avcSliceBufInfo.bufferBase = slice_mem->phy_addr;
    bufinfo->avcSliceBufInfo.bufferSize = slice_mem->size;

    bufinfo->maxDecFrmInfo.maxMbX = stride / 16;
    bufinfo->maxDecFrmInfo.maxMbY = height / 16;
    bufinfo->maxDecFrmInfo.maxMbNum = stride * height / 256;

    ret = vpu_DecRegisterFrameBuffer(dec, fb, fbcount, stride, bufinfo);
    if(ret != RETCODE_SUCCESS){
        qDebug("Register FrameBuffer error [ret:%d]", ret);
        return false;
    }

    return true;
}

bool VPU::start_decode_oneframe()
{
    DecParam param;
    RetCode ret;

    param.dispReorderBuf = 0;
    param.skipframeMode = 0;
    param.skipframeNum = 0;
    param.iframeSearchEnable = 0;

    ret = vpu_DecStartOneFrame(dec, &param);
    if(ret == RETCODE_SUCCESS)
        return true;
    else
        return false;
}

bool VPU::get_output_info(DecOutputInfo *info)
{
    RetCode ret;

    ret = vpu_DecGetOutputInfo(dec, info);
    return ret == RETCODE_SUCCESS ? true : false;
}

bool VPU::update_stream_buff_info(quint32 len)
{
    RetCode ret;

    ret = vpu_DecUpdateBitstreamBuffer(dec, len);
    if(ret != RETCODE_SUCCESS)
        return false;
    return true;
}

bool VPU::is_busy()
{
    int ret = 0;

    ret = vpu_IsBusy();
    return ret ? true : false;
}

bool VPU::wait_interrupt()
{
    int ret;

    ret = vpu_WaitForInt(100);
    return ret == 0 ? true : false;
}

void * VPU::convert_pixel(int bufidx)
{
    int ret;

    task->input.paddr = inbuf[bufidx].ipu_paddr;
    ret = ipu_check_task(ipufd, task);
    if(ret < 0){
        qDebug("ipu check task error");
        return NULL;
    }

    ret = ipu_queue_task(ipufd, task);
    if(ret < 0){
        qDebug("ipu queue task error");
        return NULL;
    }

    return outbuf[0].ipu_vaddr;
}

void VPU::soft_reset()
{
    vpu_SWReset(dec, 0);
}

bool VPU::clear_disp_flag(int index)
{
    RetCode ret;

    ret = vpu_DecClrDispFlag(dec, index);
    return ret == RETCODE_SUCCESS ? true : false;
}

int VPU::get_width()
{
    return width;
}

int VPU::get_height()
{
    return height;
}

int VPU::get_stride()
{
    return stride;
}


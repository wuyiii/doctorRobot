#ifndef VPU_H
#define VPU_H

extern "C"
{
#include <vpu_io.h>
#include <vpu_lib.h>
#include <ipu.h>
}

#include <QObject>

class VPU : public QObject
{
    Q_OBJECT

public:
    static void vpu_init();
    static void vpu_uninit();

    bool open();
    void close();
    bool fillbuff(quint32 addr, quint32 len);
    bool parse();
    bool alloc_framebuffer();
    bool start_decode_oneframe();
    bool get_output_info(DecOutputInfo *info);
    bool update_stream_buff_info(quint32 len);
    bool get_stream_buff_info(unsigned long *read_ptr, unsigned long *write_ptr, unsigned long *space);
    bool is_busy();
    bool wait_interrupt();
    void *convert_pixel(int bufidx);

    void soft_reset();
    bool clear_disp_flag(int index);

    int get_width();
    int get_height();
    int get_stride();

private:
    DecHandle dec;
    vpu_mem_desc *vpu_mem;
    vpu_mem_desc *ps_mem;
    vpu_mem_desc *slice_mem;
    vpu_mem_desc *mvcol_mem;
    DecOpenParam *open_param;
    DecInitialInfo *initinfo;

    FrameBuffer *fb;
    DecBufInfo *bufinfo;

#define MAX_BSBUFF 10
    struct ipu_buf inbuf[MAX_BSBUFF];
    struct ipu_buf outbuf[1];
    struct ipu_task *task = NULL;
    int ipufd = 0;

    int width, height, stride;
    int fbcount;
};

#endif // VPU_H

#include <ipu.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int ipu_open()
{
    int fd;
    char ipu_dev[] = "/dev/mxc_ipu";

    fd = open(ipu_dev, O_RDWR);
    if(fd < 0){
        printf("open ipufile error\n");
        return -1;
    }

    return fd;
}

void ipu_close(int fd)
{
    close(fd);
}

struct ipu_task * ipu_alloc_task(int in_w, int in_h, int out_w, int out_h)
{
    struct ipu_task *task = NULL;

    task = (struct ipu_task *)malloc(sizeof(struct ipu_task));
    if(!task)
        return NULL;

    task->task_id = IPU_TASK_ID_PP;
    task->input.width = in_w;
    task->input.height = in_h;
    task->input.format =  v4l2_fourcc('N', 'V','1', '2');
    task->input.crop.pos.x = 0;
    task->input.crop.pos.y = 0;
    task->input.crop.w = 0;
    task->input.crop.h = 0;
    task->input.deinterlace.enable = 0;
    task->input.deinterlace.motion = 0;

    task->overlay_en = 0;

    task->output.width = out_w;
    task->output.height = out_h;
    task->output.format = v4l2_fourcc('R', 'G','B', '3');
    task->output.rotate = 0;
    task->output.crop.pos.x = 0;
    task->output.crop.pos.y = 0;
    task->output.crop.w = 0;
    task->output.crop.h = 0;

    return task;
}

int ipu_alloc_fb(int fd, ipu_buf *buf, int num)
{
    int i;

//need to solve error if memory not enough
    for(i = 0; i < num; i++){
        buf[i].ipu_paddr = buf[i].size;
        if(ioctl(fd, IPU_ALLOC, &(buf[i].ipu_paddr)) < 0){
            printf("alloc ipu buf error\n");
            return -1;
        }

        buf[i].ipu_vaddr = mmap(NULL, buf[i].size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf[i].ipu_paddr);
        if(buf[i].ipu_vaddr == MAP_FAILED){
            printf("mmap faild\n");
            return -1;
        }
    }

    return 0;
}

int ipu_check_task(int fd, struct ipu_task *t)
{
    int ret;

    ret = ioctl(fd, IPU_CHECK_TASK, t);
    if (ret != IPU_CHECK_OK){
        printf("check error ret %d\n", ret);
        return -1;
    }

    return 0;
}

int ipu_queue_task(int fd, struct ipu_task *t)
{
    int ret;

    ret = ioctl(fd, IPU_QUEUE_TASK, t);
    if (ret < 0){
        printf("ipu_queue_task error ret %d\n", ret);
        return -1;
    }

    return 0;
}

void ipu_free_task(struct ipu_task *t)
{
    free(t);
}

void ipu_free_fb(int fd, ipu_buf *buf, int num)
{
    int i;

    for(i = 0; i < num; i++){
        if(buf[i].ipu_vaddr)
            munmap(buf[i].ipu_vaddr, buf[i].size);
        if(buf[i].ipu_paddr)
            ioctl(fd, IPU_FREE, &(buf[i].ipu_paddr));
    }
}

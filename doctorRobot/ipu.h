#ifndef IPU_H
#define IPU_H

#include <linux-ipu.h>

typedef struct ipu_buf
{
    int ipu_paddr;
    void *ipu_vaddr;
    int size;
}ipu_buf;

int ipu_open();
void ipu_close(int fd);
struct ipu_task * ipu_alloc_task(int in_w, int in_h, int out_w, int out_h);
void ipu_free_task(struct ipu_task *t);
int ipu_alloc_fb(int fd, ipu_buf *buf, int num);
void ipu_free_fb(int fd, ipu_buf *buf, int num);
int ipu_check_task(int fd, struct ipu_task *t);
int ipu_queue_task(int fd, struct ipu_task *t);

#endif // IPU_H

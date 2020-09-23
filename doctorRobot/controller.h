#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "car.h"
#include "arm.h"
#include "server.h"
#include <QObject>
#include <QString>
#include <QThread>
#include <turbojpeg.h>
#include <QMutex>
#include <QWaitCondition>

#include "mynteyed/camera.h"
#include "mynteyed/utils.h"

class MainWindow;

#define IP_ADDRESS      "192.168.1.95"
#define IP_UDP_PORT     3000
#define IP_TCP_PORT     8888

#define ECB_ADDRESS     "192.168.1.30"
#define ECB_UDP_PORT    2000

MYNTEYE_USE_NAMESPACE

#pragma pack(1)

union vframe {

    struct {
        char head;
        char cmd;

        ushort width;
        ushort height;

        int len;
    }d;

    char data[10];
};

#pragma pack()

class Compress : public QThread
{
    Q_OBJECT

public:
    void get_image(char *data);
    void init(int w, int h);
    void deinit();
    void stop();

signals:
    void wait_written();
    void write_cmd(char *data, int len);

protected:
    void run();

private:
    bool end = false;
    unsigned long size;
    char *yuv422;
    char *yuv420, *buf, *jpeg;

    int width, height;
    int head, tail;

    bool get = false;

    tjhandle handle;

    QMutex mutex;
    QWaitCondition notempty;
    QWaitCondition notfull;
};

class Controller : public QThread
{
    Q_OBJECT

friend class MainWindow;

public:
    Controller();
    ~Controller();

    void init();
    void stop();
    void parse_cmd(char *cmd);
    bool setup_arm();
    bool setup_camera();

//    void set_mode(bool mode);

protected:
    void run();

signals:
    void hint(QString info);
    void get_depth(QVector<ushort> depth);
    void write_cmd(char *data, int len);
    void net_stats(bool f);
    void arm_stats(bool f);
    void car_stats(bool f);
    void start_button_status(bool ok);
    void reset_button_status(bool ok);
    void adjust_button_status(bool ok);

public slots:
    void on_finished();
    void on_get_data(char *dat);
    void on_start_clicked(bool checked);
    void on_reset_clicked(bool checked);
    void on_adjust_clicked(bool checked);
    void on_auto_clicked(bool checked);

    void on_left_rotate_clicked(bool checked);
    void on_right_rotate_clicked(bool checked);

    void on_write_cmd(char *data, int len);

#define LEFT_ARM        0x01
#define RIGHT_ARM       0x02
#define CAR             0x03

private:
    Server *server = nullptr;
    Car *car = nullptr;
    ARM *arm = nullptr;

    Camera *cam = nullptr;
    Compress *compress;

    char *buf;
    ushort depx = 0, depy = 0;

    bool stop_flag = false;
    bool get_cont_frame = false;
    bool camera_ok = false;
};

#endif // CONTROLLER_H

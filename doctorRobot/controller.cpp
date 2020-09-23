#include <gpio.h>
#include <ui/infowidget.h>
#include <ui/mainwindow.h>
#include <iostream>
#include <QFile>
#include <QDir>
#include <QTime>
#include <QTimer>
#include <QDebug>
#include <controller.h>
#include <QStringList>
#include <QApplication>
#include <QtSerialPort/QSerialPortInfo>

#include <opencv2/imgproc/imgproc.hpp>

void Controller::parse_cmd(char *cmd)
{
    switch(cmd[0]){
        case LEFT_ARM:
            if (!arm->ok())
                break;

            switch (cmd[1]) {
                case 0x00:
                    if(cmd[2] == 0x00)
                        arm->spraying(false);
                    else if(cmd[2] == 0x01)
                        arm->spraying(true);
                    break;

                case 0x01:
                    arm->move_up(0.5+cmd[2]*0.5);
                    break;

                case 0x02:
                    arm->move_down(0.5+cmd[2]*0.5);
                    break;

                case 0x04:
                    arm->move_left(0.5+cmd[2]*0.5);
                    break;

                case 0x08:
                    arm->move_right(0.5+cmd[2]*0.5);
                    break;

                default:
                    break;
            }
            break;

        case RIGHT_ARM:
            switch (cmd[1]) {
                case 0x00:
                    if(cmd[2] == 0x00)
                        qDebug() << "right arm stop spraying";
                    else if(cmd[2] == 0x01)
                        qDebug() << "right arm start spraying";
                    break;

                case 0x01:
                    qDebug() << "right arm move up";
                    break;

                case 0x02:
                    qDebug() << "right arm move down";
                    break;

                case 0x04:
                    qDebug() << "right arm move left";
                    break;

                case 0x08:
                    qDebug() << "right arm move right";
                    break;

                default:
                    break;
            }
            break;

        case CAR:
            switch (cmd[1]) {
                case 0x00:
                    if(cmd[2] == 0x00)
                        qDebug() << "car stop";
                    else if(cmd[2] == 0x01)
                        qDebug() << "car start";
                    break;

                case 0x01:
                    car->move_ahead(cmd[2], cmd[3]);
                    break;

                case 0x02:
                    car->move_left_ahead(cmd[2], cmd[3]);
                    break;

                case 0x04:
                    car->move_right_ahead(cmd[2], cmd[3]);
                    break;

                case 0x08:
                    car->move_back(cmd[2], cmd[3]);
                    break;
                case 0x011:
                    car->move_left_back(cmd[2], cmd[3]);
                    break;

                case 0x012:
                    car->move_right_back(cmd[2], cmd[3]);
                    break;

                case 0x014:
                    car->move_left(cmd[2], cmd[3]);
                    break;

                case 0x018:
                    car->move_right(cmd[2], cmd[3]);
                    break;

                default:
                    break;
            }
            break;
        //mynt camera
        case 0x04:
            switch (cmd[1]) {
                //image
                case 0x01:
                    if(get_cont_frame){
                        get_cont_frame = false;
                    }
                    else{
                        get_cont_frame = true;
                    }
                    break;
                //depth
                case 0x02:
                    ushort *p = (ushort *)(cmd+2);
                    depx = *p;
                    p++;
                    depy = *p;
                    break;
            }
    }
}

static bool parse(const char *s, quint8 dat[][2])
{
    char buf[50];
    quint8 i;
    qint64 ret;

    QString path = QApplication::applicationDirPath().append("/robot.conf");
    QFile conf(path);

    if(!conf.open(QIODevice::ReadOnly)){
        qDebug() << "file robot.conf not exist";
        return false;
    }

    while(1){
        ret = conf.readLine(buf, 50);
        if(ret <= 0){
            conf.close();
            return false;
        }

        QString line = QString(buf);
        if(!line.contains(QString(s)))
            continue;

        for(i = 0; i < 3; i++){
            ret = conf.readLine(buf, 50);
            if(ret <= 0){
                conf.close();
                qDebug() << "robot.conf not correct";
                return false;
            }

            line = QString(buf);
            QStringList list = line.split(' ');
            dat[i][0] = (quint8)list.at(1).toInt();
            dat[i][1] = (quint8)list.at(2).toInt();
        }

        conf.close();
        return true;
    }
}

bool Controller::setup_arm()
{
    quint8 device[3][2];

    if(!parse("ARML", device))
        return false;

    arm->init(device[0][0], device[0][1],
              device[1][0], device[1][1],
              device[2][0], device[2][1],
              gpio_info[0][0], gpio_info[0][1]);

    return true;
}

MYNTEYE_USE_NAMESPACE

bool Controller::setup_camera()
{
    DeviceInfo dev_info;

    if (!util::select(*cam, &dev_info))
        return false;

//    util::print_stream_infos(*cam, dev_info.index);

    OpenParams params(dev_info.index);
    {
        // 最好不要超过10fps
        // Framerate: 10(default), [0,60], [0,30](STREAM_2560x720)
        params.framerate = 10;

        // Color mode: raw(default), rectified
        // params.color_mode = ColorMode::COLOR_RECTIFIED;

        // Depth mode: colorful(default), gray, raw
        // Note: must set DEPTH_RAW to get raw depth values
        params.depth_mode = DepthMode::DEPTH_RAW;

        // Stream mode: left color only
        // params.stream_mode = StreamMode::STREAM_640x480;  // vga
        params.stream_mode = StreamMode::STREAM_1280x720;  // hd
        // Stream mode: left+right color
        // params.stream_mode = StreamMode::STREAM_1280x480;  // vga
        // params.stream_mode = StreamMode::STREAM_2560x720;  // hd

        // Auto-exposure: true(default), false
        // params.state_ae = false;

        // Auto-white balance: true(default), false
        // params.state_awb = false;

        // Infrared intensity: 0(default), [0,10]
        params.ir_intensity = 0;
    }

    if (cam->Open(params) != ErrorCode::SUCCESS) {
        qDebug() << "Error: Open camera failed";
        return false;
    }

    qDebug() << "--------------------------------------------------------------------------------\n";
    qDebug() << "Open camera device successed";

//    width = 1280;
//    height = 720;

    return true;
}

//void Controller::set_mode(bool mode)
//{
//    if(mode)
//        this->start();
//    else
//        this->quit();
//}

void Controller::on_write_cmd(char *data, int len)
{
    server->send(data, len);
}

Controller::Controller()
{
    car = new Car;
    arm = new ARM;
    cam = new Camera;
    server = new Server;
    compress = new Compress;

    connect(server, &Server::get_data, this, &Controller::on_get_data);
    connect(compress, &Compress::write_cmd, this, &Controller::on_write_cmd);
    connect(this, &Controller::finished, this, &Controller::on_finished);
    connect(this, &Controller::write_cmd, this, &Controller::on_write_cmd);

    qRegisterMetaType<QVector<ushort>>("QVector<ushort>");
}

Controller::~Controller()
{
    delete arm;
    delete cam;
    delete compress;
    delete server;
    delete car;
}

void Controller::init()
{
    server->start();
    car->start();
    setup_arm();
}

void Controller::stop()
{
    car->stop();
    server->stop();
    arm->pre_stop();
}

#define DEPTHNUM 10

void Controller::run()
{
    int i, len;
    bool ret;
    vframe frame;
    ushort dep_val;
//    static quint32 frame_cnt = 0, depth_cnt = 0;

    buf = (char *)malloc(10+DEPTHNUM*sizeof(ushort));

    if (car->ok())
        emit car_stats(true);
    else
        emit car_stats(false);

    if (server->ok())
        emit net_stats(true);
    else
        emit net_stats(false);

    arm->pre_start();
    if(arm->ok()){
        emit arm_stats(true);
        emit reset_button_status(true);
        emit adjust_button_status(true);
    }
    else
        emit arm_stats(false);

//    if(arm->ok()) {
//        emit arm_stats(true);
//        emit hint("调整机械臂到工作位置。");
//        ret = arm->init_posture(170.0, 0.0);
//        if(ret) {
//            qDebug("init position ok");
//            emit arm_stats(true);
//            emit hint("已调整到工作位置。");
//        }else
//            qDebug("init position timeout");
//    }else {
//        emit arm_stats(false);
//        emit hint("机械臂连接故障");
//    }

    emit start_button_status(true);

    if (setup_camera()){
        camera_ok = true;
        compress->init(1280, 720);
        compress->start();
    }else {
        qDebug() << "init camera failed";
        emit hint("打开深度相机失败");
    }

    while(1){

        if(stop_flag)
            break;

        if(camera_ok && get_cont_frame){

            cam->WaitForStream();

            auto image_color = cam->GetStreamData(ImageType::IMAGE_LEFT_COLOR);
            if (image_color.img) {
                cv::Mat color = image_color.img->To(ImageFormat::COLOR_YUYV)->ToMat();
                compress->get_image((char *)color.ptr(0));
//                qDebug() << "captureI" << frame_cnt++ << QTime::currentTime();
            }

            auto image_depth = cam->GetStreamData(ImageType::IMAGE_DEPTH);
            if (image_depth.img) {
                cv::Mat depth_matrix = image_depth.img->To(ImageFormat::DEPTH_RAW)->ToMat();

                frame.d.head = 0x04;
                frame.d.cmd = 0x02;
                frame.d.width = (ushort)depth_matrix.cols;
                frame.d.height = (ushort)depth_matrix.rows;
                len = DEPTHNUM * sizeof(ushort);
                frame.d.len = len;
                memcpy(buf, frame.data, 10);

                ushort *depth = (ushort *)(buf+10);
                for(i = 0; i < DEPTHNUM && (depy+4*i) < depth_matrix.cols; i++){
                    dep_val = depth_matrix.at<ushort>(depth_matrix.rows-1-depx, depy+4*i);
                    depth[i] = dep_val;
                }

                emit write_cmd(buf, 10+len);
//                qDebug() << "captureD" << depth_cnt++ << QTime::currentTime();
            }
        }
        else{
            QThread::msleep(1000);
        }
    }

    if (arm->ok()){
        emit hint("调整机械臂到初始位置。");
        ret = arm->reset_posture(170.0, 0.0);
        if(ret){
            qDebug("reset position ok");
            emit hint("已调整到初始位置。");
        }
        else{
            qDebug("reset position timeout");
            emit hint("调整到初始位置超时。");
        }
    }

//  bug 
//    cam->Close();

    emit net_stats(false);
    emit arm_stats(false);
    emit car_stats(false);

    if (camera_ok){
        compress->stop();
        compress->deinit();
    }

    free(buf);
    camera_ok = false;
    emit start_button_status(true);
}

void Controller::on_finished()
{
    arm->stop();
}

void Controller::on_get_data(char *dat)
{
    parse_cmd(dat);
}

void Controller::on_start_clicked(bool checked)
{
    if(checked) {
        stop_flag = false;
        emit hint("正在启动......");
        emit start_button_status(false);
        init();
        start();
    }else {
        stop_flag = true;
        emit hint("正在停止......");
        emit start_button_status(false);
        stop();
    }
}

void Controller::on_reset_clicked(bool checked)
{
    (void)checked;
    arm->init_posture(170.0, 0.0);
}

void Controller::on_adjust_clicked(bool checked)
{
    if(checked)
        arm->adjust_start();
    else
        arm->adjust_stop();
}

void Controller::on_left_rotate_clicked(bool checked)
{
    (void)checked;
    arm->adjust_angle(true);
}

void Controller::on_right_rotate_clicked(bool checked)
{
    (void)checked;
    arm->adjust_angle(false);
}

void Controller::on_auto_clicked(bool checked)
{
    (void)checked;
    arm->auto_move();
}

#define IMAGE_NUM 5

void Compress::get_image(char *data)
{    
    if ((head + 1)%IMAGE_NUM == tail)
        notfull.wait(&mutex);

    if (end)
        return;

    memcpy(yuv422+head*width*height*2, data, width*height*2);

    head++;
    head %= IMAGE_NUM;

    notempty.wakeAll();
}

void Compress::init(int w, int h)
{
    head = tail = 0;

    width = w;
    height = h;

    yuv422 = (char *)malloc(width*height*2*IMAGE_NUM);
    yuv420 = (char *)malloc(width*height*3/2);

    handle = tjInitCompress();

    size = tjBufSize(width, height, TJSAMP_420);
    buf = (char *)malloc(10+size);
    jpeg = buf+10;

    end = false;
}

void Compress::deinit()
{
    tjDestroy(handle);
    free(yuv422);
    free(yuv420);
    free(buf);
}

void Compress::stop()
{
    end = true;

    notfull.wakeAll();
    notempty.wakeAll();
}

void Compress::run()
{
    int i, j, k;
    char *p;
    vframe frame;
    unsigned long asize;

    while(1){
        //  yuv422 to yuv420
        if (tail == head)
            notempty.wait(&mutex);

        if (end)
            break;

        p = yuv422 + tail*width*height*2;

        k = 0;
        for (i = 0; i < width*height*2; i+=2)
            yuv420[k++] = p[i];

        for (i = 0; i < height; i+=2)
            for (j = 1; j < width*2; j+=4)
                yuv420[k++] = p[i*width*2+j];

        for (i = 0; i < height; i+=2)
            for (j = 3; j < width*2; j+=4)
                yuv420[k++] = p[i*width*2+j];

        asize = size;
        if (tjCompressFromYUV(handle, (const unsigned char *)yuv420, width, 1, height,
                              TJSAMP_420, (unsigned char **)&jpeg, (unsigned long *)&asize, 80, 0) < 0){
            qDebug() << "compress yuv to jpeg error";
            return;
        }

        /*                       定义格式
         *  字节   01   02   [03 04] [05 06] [07 08 09 10] ...
         *       帧头  类型    列数     行数      总长度
         *        04   01(图像数据)
         *             02(深度数据)
         */

        frame.d.head = 0x04;
        frame.d.cmd = 0x01;
        frame.d.width = (ushort)width;
        frame.d.height = (ushort)height;
        frame.d.len = (int)asize;
        memcpy(buf, frame.data, 10);

        emit write_cmd(buf, 10+asize);

        tail++;
        tail %= IMAGE_NUM;
        notfull.wakeAll();
    }

    mutex.unlock();
}

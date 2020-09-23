#ifndef CANBUS_H
#define CANBUS_H

#include <QCanBusDeviceInfo>
#include <QCanBusFrame>
#include <QCanBusDevice>
#include <QCanBus>
#include <QQueue>
#include <QSet>

class CanDevice;
class QTimer;

class CanBus : public QObject
{
    Q_OBJECT

public:
    static CanBus * get_instance();
    void write_frame(quint8 id, quint8 *data, quint8 len);
    void add_device(quint8 id, CanDevice *device);

public slots:
    void on_errorOccurred(QCanBusDevice::CanBusError error);
    void on_framesReceived();
    void on_framesWritten(qint64 framesCount);

private:
    CanBus(){}
    ~CanBus();

    static CanBus * canbus;
    QCanBusDevice *device = nullptr;

    QSet<quint8> devices_id;
    CanDevice * devices[10];
};

class CanDevice : public QObject
{
    Q_OBJECT

public:
    CanDevice();
    ~CanDevice();

    void start();
    void stop();
    void hello();
    void set_id(quint8 id);
    void set_radio(quint8 radio);
    void set_normal(bool f);
    void set_angle(float angle);
    void set_max_speed(quint16 s);
    void set_position(float p);
    void clear_error();  

    quint8 get_id(){return id;}
    quint8 get_radio(){return radio;}
    float get_angle(){return angle;}
    float get_device_temp(){return device_temp;}
    float get_iverter_temp(){return iverter_temp;}
    void get_frame(QCanBusFrame frame);

signals:
    void connected();
    void disconnected();

public slots:
    void query();
    void init();

private:
    void get_position();
    void get_temperature();
    void get_mode();

private:
    quint8 id;                      // can id
    quint8 radio;                   // 减速比
    float angle;                    // 当前角度
    float pos;                      // 当前位置 单位(r)
    float device_temp;              // 执行器温度
    float iverter_temp;             // 逆变器温度
    quint16 alarm;                  // 报警值

    bool normal = true;             // 设置安装面，正安装/反安装，决定正转反转
    bool status = false;            // 是否在线
    bool started = false;
    QTimer *timer;                  // 心跳计数器
    QTimer *timer_query;            // 查询参数计时器
};

#endif // CANBUS_H

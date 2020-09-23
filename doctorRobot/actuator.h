#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QTimer>
#include <QObject>
#include <QMutex>
#include <QUdpSocket>

class Actuator;

class ECB : public QUdpSocket
{
    Q_OBJECT

public:
    static ECB *instance();
    ~ECB();

    void add_device(const quint8 id, Actuator *device);
    void send_frame(quint8 id, char cmd, char *data, quint8 len);

    bool ok(){return stats;}
    void init();
    void stop();
    void hello();
    void query();
    void start();
    void devices();

public slots:
    void on_readyRead();

private:
    explicit ECB(QObject *parent = nullptr);

    QMutex mutex;

    bool stats = false;
    bool inited = false;

    static ECB *ecb;
    QSet<quint8> devset;
    Actuator *dev[10] = {nullptr};
};

enum State {
  None = 0,     //空状态
  Opened,       //已打开(2A命令)
  SetMode,      //已设置模式(01 07命令)
  Closed,       //已关闭(2A命令)
  vMAX,
};

class Actuator : public QObject
{
    Q_OBJECT

public:
    void hello();
    void set_id(quint8 id);
    void set_radio(quint8 radio);
    void set_normal(bool f);

    void set_angle(qreal a);
    void set_offset(qreal o);
    void set_max_speed(quint16 s);
    void set_position(qreal p);
    void clear_error();

    quint8 get_id(){return id;}
    quint8 get_radio(){return radio;}
    void get_state();

    void get_position();
    void get_temperature();
    void get_mode();

    qreal get_offset(){return offset;}
    qreal get_angle(){return angle;}
    qreal get_device_temp(){return device_temp;}
    qreal get_iverter_temp(){return iverter_temp;}

    void get_frame(QByteArray &frame);

    State state = None;

signals:
    void connected();
    void disconnected();

public slots:
    void open();
    void set_position_mode();
    void set_current_mode();
    void close();

private:
    quint8 id;                        // device id
    quint8 radio;                   // 减速比
    qreal offset = 0.0;
    qreal angle;                    // 当前角度
    qreal pos;                      // 当前位置 单位(r)
    qreal device_temp;              // 执行器温度
    qreal iverter_temp;             // 逆变器温度
    quint16 alarm;                  // 报警值

    bool normal = true;             // 设置安装面，正安装/反安装，决定正转反转
    bool status = false;            // 是否在线
    bool started = false;

//    QTimer *timer;                  // 心跳计数器
};

#endif // ACTUATOR_H

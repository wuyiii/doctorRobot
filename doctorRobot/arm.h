#ifndef ARM_H
#define ARM_H

#include <actuator.h>
#include <QObject>
#include <QPointF>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/*- 数字2: 手关节电机
 *- 数字1: 肘关节电机
 *- 数字0: 臂关节电机
 */

#define ANGLE 60.0
#define ARM_LEN2 0.0
#define ARM_LEN1 80.0
#define ARM_LEN0 100.0

class GPIO;

class ARM : public QThread
{
    Q_OBJECT

public:
    explicit ARM(QObject *parent = nullptr);
    ~ARM();

    void init(quint8 id2, quint8 radio2,
             quint8 id1, quint8 radio1,
             quint8 id0, quint8 radio0,
             qint8 chip, qint8 cnt);
    bool ok(){return  status;}
    void pre_start();
    void pre_stop();
    void stop();

    void adjust_start();
    void adjust_stop();
    void adjust_angle(bool dir);

    void spraying(bool dir);

    bool check_p3_position(QPointF p);
    bool check_p2_position(QPointF p);
    void set_p3_position(QPointF p);
    void set_p2_position(QPointF p);

    // 仅用于初始化位置
    void set_posture(qreal a1, qreal a0);
    bool init_posture(qreal a1, qreal a0);
    bool reset_posture(qreal a1, qreal a0);

    void auto_move();
    void move_left(qreal d);
    void move_right(qreal d);
    void move_up(qreal d);
    void move_down(qreal d);

    void move_s2_left(qreal a);
    void move_s2_right(qreal a);

    void move_s1_left(qreal a);
    void move_s1_right(qreal a);

    void move_s0_left(qreal a);
    void move_s0_right(qreal a);

signals:
    void ecb_init();
    void ecb_stop();
    void get_angle(qreal a2, qreal a1, qreal a0);
    void get_angle_can(qreal a2, qreal a1, qreal a0);
    void get_tempt(qreal t2_d, qreal t2_i, qreal t1_d, qreal t1_i, qreal t0_d, qreal t0_i);
    void get_position(QPointF p3, QPointF p2, QPointF p1, QPointF p0);

protected:
    void timerEvent(QTimerEvent *event);
    void run();

private:
    // 当前位置
    QPointF _pos3;
    QPointF _pos2;
    QPointF _pos1;
    QPointF _pos0;

    // 当前角度
    qreal _angle2;
    qreal _angle1;
    qreal _angle0;

    // p2-p3段要保持的角度
    qreal _angle23 = 60.0;

    Actuator *can_dev2;
    Actuator *can_dev1;
    Actuator *can_dev0;

    GPIO *nozzle;

    QMutex mutex;
    QWaitCondition wait;

    int query_timer = -1;
    bool status = false;
    bool end = false;
};

#endif // ARM_H

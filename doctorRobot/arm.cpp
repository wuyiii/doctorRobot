#include "arm.h"
#include <gpio.h>
#include <QTimerEvent>
#include <QMatrix>
#include <QtMath>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>

ARM::ARM(QObject *parent)
    : QThread(parent)
{
    can_dev2 = new Actuator;
    can_dev1 = new Actuator;
    can_dev0 = new Actuator;
    nozzle = new GPIO;
}

ARM::~ARM()
{
    delete can_dev2;
    delete can_dev1;
    delete can_dev0;
    delete nozzle;
}

void ARM::init(quint8 id2, quint8 radio2,
              quint8 id1, quint8 radio1,
              quint8 id0, quint8 radio0,
              qint8 chip, qint8 cnt)
{
    can_dev2->set_id(id2);
    can_dev1->set_id(id1);
    can_dev0->set_id(id0);

    can_dev2->set_radio(radio2);
    can_dev1->set_radio(radio1);
    can_dev0->set_radio(radio0);

    can_dev2->set_normal(true);
    can_dev1->set_normal(true);
    can_dev0->set_normal(true);

    nozzle->setid(chip, cnt);

    ECB::instance()->init();
    query_timer = this->startTimer(50);

    qDebug() << "can2 id " << can_dev2->get_id() << " radio " << can_dev2->get_radio();
    qDebug() << "can1 id " << can_dev1->get_id() << " radio " << can_dev1->get_radio();
    qDebug() << "can0 id " << can_dev0->get_id() << " radio " << can_dev0->get_radio();

//    emit get_angle(_angle2, _angle1, _angle0);
//    emit get_position(_pos3, _pos2, _pos1, _pos0);
}

void ARM::pre_start()
{
    nozzle->open();
    nozzle->setup();
    nozzle->output(false);

    ECB::instance()->add_device(can_dev2->get_id(), can_dev2);
    ECB::instance()->add_device(can_dev1->get_id(), can_dev1);
    ECB::instance()->add_device(can_dev0->get_id(), can_dev0);

    if (!ECB::instance()->ok())
        return;

    ECB::instance()->hello();
    ECB::instance()->query();
    QThread::msleep(1000);

    ECB::instance()->devices();
    ECB::instance()->start();
    ECB::instance()->hello();
    QThread::msleep(50);

    can_dev2->get_state();
    can_dev1->get_state();
    can_dev0->get_state();

    QThread::msleep(100);

    if(can_dev2->state != State::Opened)
        can_dev2->open();

    if(can_dev1->state != State::Opened)
        can_dev1->open();

    if(can_dev0->state != State::Opened)
        can_dev0->open();

    QThread::msleep(1000);

    if(can_dev0->state != Opened && can_dev1->state != Opened){
//            || can_dev2->state != Opened){
        qDebug() << "open device error, device not connected";
        return;
    }

    can_dev2->set_position_mode();
    can_dev1->set_position_mode();
    can_dev0->set_position_mode();
    QThread::msleep(100);

    if(can_dev0->state != SetMode && can_dev1->state != SetMode){
//            || can_dev2->state != SetMode){
        qDebug() << "setmode device error, device not connected";
        return;
    }

    status = true;

    can_dev2->set_offset(0.0);
    can_dev1->set_offset(0.0);
    can_dev0->set_offset(0.0);

    // 等待各电机获取初始位置
    QThread::sleep(2);
    can_dev2->set_offset(can_dev2->get_angle()+180.0);
    can_dev1->set_offset(can_dev1->get_angle()-180.0);
    can_dev0->set_offset(can_dev0->get_angle());

    init_posture(180.0, 0.0);

    end = false;
    start();
    return;
}

void ARM::pre_stop()
{
    end = true;
    wait.wakeAll();
}

void ARM::timerEvent(QTimerEvent *event)
{    
    if(status && (event->timerId() == query_timer)){
        emit get_angle_can(can_dev2->get_angle(),
                           can_dev1->get_angle(),
                           can_dev0->get_angle());

        emit get_tempt(can_dev2->get_device_temp(), can_dev2->get_iverter_temp(),
                       can_dev1->get_device_temp(), can_dev1->get_iverter_temp(),
                       can_dev0->get_device_temp(), can_dev0->get_iverter_temp());

        can_dev2->hello();
        can_dev2->get_position();
        can_dev2->get_temperature();

        can_dev1->hello();
        can_dev1->get_position();
        can_dev1->get_temperature();

        can_dev0->hello();
        can_dev0->get_position();
        can_dev0->get_temperature();
    }
}

void ARM::run()
{
    int i;

    while(1){

        wait.wait(&mutex);

        if (end)
            break;

        set_p2_position(QPointF(50, 150));
        sleep(4);

        for (i = 0; i < 200; i++){
            move_right(0.2);
            move_down(0.05);
            msleep(50);
        }

        for (i = 0; i < 200; i++){
            move_left(0.2);
            move_down(0.05);
            msleep(50);
        }

        for (i = 0; i < 200; i++){
            move_right(0.2);
            move_down(0.05);
            msleep(50);
        }

        for (i = 0; i < 200; i++){
            move_left(0.2);
            move_down(0.05);
            msleep(50);
        }
    }
}

// 发送 0x2A 命令除能电机
void ARM::stop()
{
    if (query_timer > 0)
        killTimer(query_timer);
    query_timer = 0;

    nozzle->close();
    can_dev2->close();
    can_dev1->close();
    can_dev0->close();

    QThread::msleep(50);
    ECB::instance()->stop();
    status = false;
}

void ARM::adjust_start()
{
    can_dev2->set_current_mode();
    can_dev1->set_current_mode();
    can_dev0->set_current_mode();

    can_dev2->set_offset(0.0);
    can_dev1->set_offset(0.0);
    can_dev0->set_offset(0.0);
}

void ARM::adjust_stop()
{
    can_dev2->set_offset(can_dev2->get_angle()+180.0);
    can_dev1->set_offset(can_dev1->get_angle()-180.0);
    can_dev0->set_offset(can_dev0->get_angle());

    can_dev2->set_position_mode();
    can_dev1->set_position_mode();
    can_dev0->set_position_mode();
}

// true: left; false: right
void ARM::adjust_angle(bool dir)
{
    if (dir){
        move_s2_left(5.0);
        _angle23 += 5.0;
    }else {
        move_s2_right(5.0);
        _angle23 -= 5.0;
    }

    emit get_angle(_angle2, _angle1, _angle0);
    emit get_position(_pos3, _pos2, _pos1, _pos0);
}

void ARM::spraying(bool dir)
{
    nozzle->output(dir);
}

bool ARM::check_p3_position(QPointF p)
{
    qreal l1;
    qreal l2;
    QPointF p0 = p - _pos0;

    if(p0.x() < -0.05 || p0.y() < -0.05)
        return false;

    l1 = qPow(p0.x(), 2) + qPow(p0.y(), 2);
    l2 = qPow(ARM_LEN0 + ARM_LEN1 + ARM_LEN2, 2);
    if(l1 > l2)
        return false;

    return true;
}

bool ARM::check_p2_position(QPointF p)
{
    qreal l1;
    QPointF p0 = p - _pos0;

    if(p0.x() < -0.05 || p0.y() < -0.05 || p0 == _pos0)
        return false;

    l1 = qPow(p0.x(), 2) + qPow(p0.y(), 2);
    l1 = qSqrt(l1);

    if(l1 > ARM_LEN0 + ARM_LEN1 || l1 < qAbs(ARM_LEN0 - ARM_LEN1))
        return false;

    return true;
}

void ARM::set_p2_position(QPointF p)
{
    qreal t0, a0, a1;
    qreal d, d2, x0, x1, x2, y0, y1, y2, l0, l1;
    QPointF p2;

    p2 = p;
    if(!check_p2_position(p2))
        return;

    //计算两圆心之间的距离
    d2 = qPow((p2.x() - _pos0.x()), 2) + qPow((p2.y() - _pos0.y()), 2);
    d = qSqrt(d2);

    l0 = qPow(ARM_LEN0, 2) - qPow(ARM_LEN1, 2) + d2;
    l0 = l0 / ( 2 * d);

    //得到筝形的对角线的交点位置
    x2 = _pos0.x() + l0 * (p2.x() - _pos0.x()) / d;
    y2 = _pos0.y() + l0 * (p2.y() - _pos0.y()) / d;

    // 筝形对角线的其中一条长度的一半
    l1 = qPow(ARM_LEN0, 2) - qPow(l0, 2);
    l1 = qSqrt(l1);

    //两圆交点 a 的位置 （位于交点的左方）
    x0 = x2 - l1 * (p2.y() - _pos0.y()) / d;
    y0 = y2 + l1 * (p2.x() - _pos0.x()) / d;

    //两圆交点 b 的位置 （位于交点的右方）
    x1 = x2 + l1 * (p2.y() - _pos0.y()) / d;
    y1 = y2 - l1 * (p2.x() - _pos0.x()) / d;

    // 求第一个交点(x0, y0)产生的直线p2-p1与直线p1-p0的角度
    QPointF pp1(x0 - _pos0.x(), y0 - _pos0.y());
    QPointF pp2(p2.x() - x0, p2.y() - y0);

    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
    l0 = qSqrt(l0);
    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
    l1 = qSqrt(l1);

    // 根据向量点积求夹角
    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
    t0 = t0 / (l0 * l1);
    t0 = qAcos(t0);
    t0 = qRadiansToDegrees(t0);

    // 根据向量积求正负
    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
    if(d > 0)
        t0 = -t0;

    // 求第二个交点(x1, y1)产生的直线p2-p1与直线p1-p0的角度
//    t1 = -t0;

    //选择合适的_angle0
    a0 = qAtan2(y0 - _pos0.y(), x0 - _pos0.x());
    a0 = qRadiansToDegrees(a0);
    a1 = qAtan2(y1 - _pos0.y(), x1 - _pos0.x());
    a1 = qRadiansToDegrees(a1);

//    if(qAbs(a0 -  90.0 + _angle0) < qAbs(a1 - 90.0 + _angle0)){
//        _angle0 = a0;
//        _pos1 = QPointF(x0, y0);
//        _angle1 = t0;
//    }else{
//        _angle0 = a1;
//        _pos1 = QPointF(x1, y1);
//        _angle1 = t1;
//    }

    if (p2.x() < x0 && p2.y() < y0)
        return;

    _angle0 = 90.0 - a0;
    _angle1 = t0;
    _angle2 = qRadiansToDegrees(qAtan2(p2.y() - y0, p2.x() - x0)) - _angle23;

    _pos1 = QPointF(x0, y0);
    _pos2 = p2;

    x0 = p2.x() + ARM_LEN2 * qCos(qDegreesToRadians(_angle23));
    y0 = p2.y() + ARM_LEN2 * qSin(qDegreesToRadians(_angle23));
    _pos3 = QPointF(x0, y0);

    emit get_angle(_angle2, _angle1, _angle0);
    emit get_position(_pos3, _pos2, _pos1, _pos0);

    can_dev2->set_angle(_angle2);
    can_dev1->set_angle(_angle1);
    can_dev0->set_angle(_angle0);
}

void ARM::set_p3_position(QPointF p)
{
    qreal t0, a0, a1;
    qreal d, d2, x0, x1, x2, y0, y1, y2, l0, l1;
    QPointF p2, p3;

    p3 = p;
    if(!check_p3_position(p3))
        return;

    // p2-p3段保持固定 30°，可得p2位置
    p2.rx() = p3.x() - ARM_LEN2 * qCos(qDegreesToRadians(_angle23));
    p2.ry() = p3.y() - ARM_LEN2 * qSin(qDegreesToRadians(_angle23));

    if(!check_p2_position(p2))
        return;

    //计算两圆心之间的距离
    d2 = qPow((p2.x() - _pos0.x()), 2) + qPow((p2.y() - _pos0.y()), 2);
    d = qSqrt(d2);

    l0 = qPow(ARM_LEN0, 2) - qPow(ARM_LEN1, 2) + d2;
    l0 = l0 / ( 2 * d);

    //得到筝形的对角线的交点位置
    x2 = _pos0.x() + l0 * (p2.x() - _pos0.x()) / d;
    y2 = _pos0.y() + l0 * (p2.y() - _pos0.y()) / d;

    // 筝形对角线的其中一条长度的一半
    l1 = qPow(ARM_LEN0, 2) - qPow(l0, 2);
    l1 = qSqrt(l1);

    //两圆交点 a 的位置 （位于交点的左方）
    x0 = x2 - l1 * (p2.y() - _pos0.y()) / d;
    y0 = y2 + l1 * (p2.x() - _pos0.x()) / d;

    //两圆交点 b 的位置 （位于交点的右方）
    x1 = x2 + l1 * (p2.y() - _pos0.y()) / d;
    y1 = y2 - l1 * (p2.x() - _pos0.x()) / d;

    // 求第一个交点(x0, y0)产生的直线p2-p1与直线p1-p0的角度
    QPointF pp1(x0 - _pos0.x(), y0 - _pos0.y());
    QPointF pp2(p2.x() - x0, p2.y() - y0);

    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
    l0 = qSqrt(l0);
    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
    l1 = qSqrt(l1);

    // 根据向量点积求夹角
    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
    t0 = t0 / (l0 * l1);
    t0 = qAcos(t0);
    t0 = qRadiansToDegrees(t0);

    // 根据向量积求正负
    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
    if(d > 0)
        t0 = -t0;

    // 求第二个交点(x1, y1)产生的直线p2-p1与直线p1-p0的角度
//    t1 = -t0;

    //选择合适的_angle0
    a0 = qAtan2(y0 - _pos0.y(), x0 - _pos0.x());
    a0 = qRadiansToDegrees(a0);
    a1 = qAtan2(y1 - _pos0.y(), x1 - _pos0.x());
    a1 = qRadiansToDegrees(a1);

//    if(qAbs(a0 -  90.0 + _angle0) < qAbs(a1 - 90.0 + _angle0)){
//        _angle0 = a0;
//        _pos1 = QPointF(x0, y0);
//        _angle1 = t0;
//    }else{
//        _angle0 = a1;
//        _pos1 = QPointF(x1, y1);
//        _angle1 = t1;
//    }

    _angle0 = 90.0 - a0;
    _angle1 = t0;

    _pos1 = QPointF(x0, y0);
    _pos2 = p2;
    _pos3 = p3;

    // 调整 p3-p2 段角度
    pp1 = QPointF(_pos2.x() - _pos1.x(), _pos2.y() - _pos1.y());
    pp2 = QPointF(_pos3.x() - _pos2.x(), _pos3.y() - _pos2.y());

    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
    l0 = qSqrt(l0);
    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
    l1 = qSqrt(l1);

    // 根据向量点积求夹角
    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
    t0 = t0 / (l0 * l1);
    t0 = qAcos(t0);
    t0 = qRadiansToDegrees(t0);

    // 根据向量积求正负
    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
    if(d > 0)
        t0 = -t0;

    _angle2 = t0;

    emit get_angle(_angle2, _angle1, _angle0);
    emit get_position(_pos3, _pos2, _pos1, _pos0);

    can_dev2->set_angle(_angle2);
    can_dev1->set_angle(_angle1);
    can_dev0->set_angle(_angle0);
}

//void ARM::set_position(QPointF p)
//{
//    qreal t0, t1, t2, t3, a0, a1, a2;
//    qreal d, d2, x0, x1, x2, y0, y1, y2, l0, l1;
//    QPointF p1, p2, p3;
//    QMatrix m;
//    bool ok = true;
//    int i = 0;

//    if(!check_p3_position(p))
//        return;

//    // 第一步使得 pos0 pos1 pos3 共线
//    x0 = p.x() - _pos0.x();
//    y0 = p.y() - _pos0.y();

//    t0 = qAtan2(x0, y0);
//    t0 = qRadiansToDegrees(t0);
//    a0 = t0;

//    m.reset();
//    m.rotate(-(a0 - _angle0));
//    p1 = _pos1 * m;

//    //计算两圆心之间的距离
//    d2 = qPow((p.x() - p1.x()), 2) + qPow((p.y() - p1.y()), 2);
//    d = qSqrt(d2);
//    if(d < ARM_LEN1 - ARM_LEN2 || d > ARM_LEN1 + ARM_LEN2)
//        ok = false;

//    l0 = qPow(ARM_LEN1, 2) - qPow(ARM_LEN2, 2) + d2;
//    l0 = l0 / ( 2 * d);

//    //得到筝形的对角线的交点位置
//    x2 = p1.x() + l0 * (p.x() - p1.x()) / d;
//    y2 = p1.y() + l0 * (p.y() - p1.y()) / d;

//    // 筝形对角线的其中一条长度的一半
//    l1 = qPow(ARM_LEN1, 2) - qPow(l0, 2);
//    l1 = qSqrt(l1);

//    //两圆交点 a 的位置 （位于交点的左方）
//    x0 = x2 - l1 * (p.y() - p1.y()) / d;
//    y0 = y2 + l1 * (p.x() - p1.x()) / d;

//    //两圆交点 b 的位置 （位于交点的右方）
//    x1 = x2 + l1 * (p.y() - p1.y()) / d;
//    y1 = y2 - l1 * (p.x() - p1.x()) / d;

//    // 求第一个交点(x0, y0)相对p1-p0直线的角度
//    QPointF pp1(p1.x() - _pos0.x(), p1.y() - _pos0.y());
//    QPointF pp2(x0 - p1.x(), y0 - p1.y());

//    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
//    l0 = qSqrt(l0);
//    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
//    l1 = qSqrt(l1);

//    // 根据向量点积求夹角
//    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
//    t0 = t0 / (l0 * l1);
//    t0 = qAcos(t0);
//    t0 = qRadiansToDegrees(t0);

//    QPointF pp3(p.x() - x0, p.y() - y0);
//    t2 = qAtan2(pp3.y(), pp3.x());
//    t2 = qRadiansToDegrees(t2);

//    QPointF pp4(p.x() - x1, p.y() - y1);
//    t3 = qAtan2(pp4.y(), pp4.x());
//    t3 = qRadiansToDegrees(t3);

//    // 根据向量积求正负
//    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
//    if(d > 0)
//        t0 = -t0;

//    // 求第二个交点(x1, y1)相对p1-p0直线的角度
//    pp1 = QPointF(p1.x() - _pos0.x(), p1.y() - _pos0.y());
//    pp2 = QPointF(x1 - p1.x(), y1 - p1.y());

//    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
//    l0 = qSqrt(l0);
//    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
//    l1 = qSqrt(l1);

//    // 根据向量点积求夹角
//    t1 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
//    t1 = t1 / (l0 * l1);
//    t1 = qAcos(t1);
//    t1 = qRadiansToDegrees(t1);

//    // 根据向量积求正负
//    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
//    if(d > 0)
//        t1 = -t1;

//    l0 = qAtan2(x0 - p1.x(), y0 - p1.y());
//    l0 = qRadiansToDegrees(l0);

//    l1 = qAtan2(x1 - p1.x(), y1 - p1.y());
//    l1 = qRadiansToDegrees(l1);

//    //选择最优角度
//    if(y0 < 0){
//        if(y1 < 0)
//            return;
//        else {
//            if(t3 < -ANGLE || t3 > )
//                ok = false;
//            else {
//                p2 = QPointF(x1, y1);
//                a1 = t1;
//            }
//        }
//    }else if(y1 < 0){
//        if(y0 < 0)
//            return;
//        else {
//            if(t2 < -ANGLE || t2 > ANGLE)
//                ok = false;
//            else{
//                p2 = QPointF(x0, y0);
//                a1 = t0;
//            }
//        }
//    }else{
//        if(t2 < -ANGLE || t2 > ANGLE){
//            if(t3 < -ANGLE || t3 > ANGLE)
//                ok = false;
//            else {
//                p2 = QPointF(x1, y1);
//                a1 = t1;
//            }
//        }else if(t3 < -ANGLE || t3 > ANGLE){
//            if(t2 < -ANGLE || t2 > ANGLE)
//                ok = false;
//            else {
//                p2 = QPointF(x0, y0);
//                a1 = t0;
//            }
//        }else if(qAbs(t0 - _angle1) < qAbs(t1 - _angle1)){
//            p2 = QPointF(x0, y0);
//            a1 = t0;
//        }
//        else{
//            p2 = QPointF(x1, y1);
//            a1 = t1;
//        }
//    }

//    // 第二阶段，调整pos1位置
//    if(!ok){
//        for (i = 0; i < 10; i++) {

//            // 求直线p2-p3接近ANGLE时的位置
//            m.reset();
//            m.translate(p.x()-_pos3.x(), p.y()-_pos3.y());
//            p2 = _pos2 * m;

//            m.reset();
//            m.translate(-p.x(), -p.y());
//            p2 = p2 * m;

//            t0 = qAtan2(-p2.y(), -p2.x());
//            t0 = qRadiansToDegrees(t0);

//            if(t0 > ANGLE || t0 < -ANGLE){
//                if(t2 > 0)
//                    d = -(10.0 - i);
//                else
//                    d = 10.0 - i;

//                m.reset();
//                m.rotate(d);
//                p2 = p2 * m;
//            }

//            // 得到pos2新位置
//            m.reset();
//            m.translate(p.x(), p.y());
//            p2 = p2 * m;

//            if(!check_p2_position(p2))
//                continue;
//            else
//                break;
//        }

//        if(i == 10)
//            return;

//        // 求pos1的最佳位置
//        d2 = qPow((p2.x() - _pos0.x()), 2) + qPow((p2.y() - _pos0.y()), 2);
//        d = qSqrt(d2);

//        l0 = qPow(ARM_LEN0, 2) - qPow(ARM_LEN1, 2) + d2;
//        l0 = l0 / ( 2 * d);

//        //得到筝形的对角线的交点位置
//        x2 = _pos0.x() + l0 * (p2.x() - _pos0.x()) / d;
//        y2 = _pos0.y() + l0 * (p2.y() - _pos0.y()) / d;

//        l1 = qPow(ARM_LEN0, 2) - qPow(l0, 2);
//        l1 = qSqrt(l1);

//        //两圆交点a的位置
//        x0 = x2 - l1 * (p2.y() - _pos0.y()) / d;
//        y0 = y2 + l1 * (p2.x() - _pos0.x()) / d;

//        //两圆交点b的位置
//        x1 = x2 + l1 * (p2.y() - _pos0.y()) / d;
//        y1 = y2 - l1 * (p2.x() - _pos0.x()) / d;

//        t0 = qAtan2(y0, x0);
//        t0 = 90 - qRadiansToDegrees(t0);

//        t1 = qAtan2(y1, x1);
//        t1 = 90 - qRadiansToDegrees(t1);

//        if(qAbs(t0) < qAbs(t1)){
//            p1 = QPointF(x0, y0);
//            a0 = t0;
//        }
//        else{
//            p1 = QPointF(x1, y1);
//            a0 = t1;
//        }

//        // 求_angle1
//        pp1 = QPointF(p1.x() - _pos0.x(), p1.y() - _pos0.y());
//        pp2 = QPointF(p2.x() - p1.x(), p2.y() - p1.y());

//        l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
//        l0 = qSqrt(l0);
//        l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
//        l1 = qSqrt(l1);

//        // 根据向量点积求夹角
//        t1 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
//        t1 = t1 / (l0 * l1);
//        t1 = qAcos(t1);
//        t1 = qRadiansToDegrees(t1);

//        d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
//        if(d > 0)
//            t1 = -t1;

//        a1 = t1;
//    }

//    p3 = p;

//    // 得到 pos2 的坐标
//    pp1 = QPointF(p2.x() - p1.x(), p2.y() - p1.y());
//    pp2 = QPointF(p3.x() - p2.x(), p3.y() - p2.y());

//    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
//    l0 = qSqrt(l0);
//    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
//    l1 = qSqrt(l1);

//    // 根据向量点积求夹角
//    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
//    t0 = t0 / (l0 * l1);
//    t0 = qAcos(t0);
//    t0 = qRadiansToDegrees(t0);

//    // 根据向量积求正负
//    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
//    if(d > 0)
//        a2 = -t0;
//    else
//        a2 = t0;

//    _pos1 = p1;
//    _pos2 = p2;
//    _pos3 = p3;

//    _angle0 = a0;
//    _angle1 = a1;
//    _angle2 = a2;

//    emit get_angle(_angle2, _angle1, _angle0);
//    emit get_position(_pos3, _pos2, _pos1, _pos0);

//    can_dev2->set_angle(_angle2);
//    can_dev1->set_angle(_angle1);
//    can_dev0->set_angle(_angle0);
//}


void ARM::set_posture(qreal a1, qreal a0)
{
    qreal t0, l0, l1, d;

    _pos0 = QPointF(0.0, 0.0);
    _pos1 = QPointF(_pos0.x(), _pos0.y()+ARM_LEN0);
    _pos2 = QPointF(_pos1.x(), _pos1.y()+ARM_LEN1);
    _pos3 = QPointF(_pos2.x(), _pos2.y()+ARM_LEN2);

    _angle2 = 0.0;
    _angle1 = 0.0;
    _angle0 = 0.0;

    can_dev1->set_angle(a1);
    can_dev0->set_angle(a0);

    QPointF p3 = _pos3;
    QPointF p2 = _pos2;
    QPointF p1 = _pos1;
    QMatrix m;

    m.translate(-_pos0.x(), -_pos0.y());
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.rotate(-a0);
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.translate(-p1.x(), -p1.y());
    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.rotate(-a1);
    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.translate(p1.x(), p1.y());
    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.translate(_pos0.x(), _pos0.y());
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    // 调整 p3-p2 段角度
    QPointF pp1 = QPointF(p2.x() - p1.x(), p2.y() - p1.y());
    QPointF pp2 = QPointF(1.0, 1.0 * qTan(qDegreesToRadians(_angle23)));

    l0 = qPow(pp1.x(), 2) + qPow(pp1.y(), 2);
    l0 = qSqrt(l0);
    l1 = qPow(pp2.x(), 2) + qPow(pp2.y(), 2);
    l1 = qSqrt(l1);

    // 根据向量点积求夹角
    t0 = pp1.x() * pp2.x() + pp1.y() * pp2.y();
    t0 = t0 / (l0 * l1);
    t0 = qAcos(t0);
    t0 = qRadiansToDegrees(t0);

    // 根据向量积求正负
    d = pp1.x() * pp2.y() - pp1.y() * pp2.x();
    if(d > 0)
        t0 = -t0;

    m.reset();
    m.translate(-p2.x(), -p2.y());
    p3 = p3 * m;

    m.reset();
    m.rotate(-t0);
    p3 = p3 * m;

    m.reset();
    m.translate(p2.x(), p2.y());
    p3 = p3 * m;

    _pos3 = p3;
    _pos2 = p2;
    _pos1 = p1;

    _angle2 = t0;
    _angle1 = a1;
    _angle0 = a0;

    can_dev2->set_angle(t0);
    emit get_angle(_angle2, _angle1, _angle0);
    emit get_position(_pos3, _pos2, _pos1, _pos0);
}

bool ARM::init_posture(qreal a1, qreal a0)
{
//    quint8 cnt = 0;

    _angle23 = 80.0;
    set_posture(a1, a0);

//    while(1){
//        if(qAbs(can_dev1->get_angle() - a1) < 0.5 &&
//           qAbs(can_dev0->get_angle() - a0) < 0.5)
//            break;

//        if(cnt == 10)
//            return false;

//        cnt++;
//        QThread::msleep(400);
//    }

    _angle23 = 60.0;
    return true;
}

bool ARM::reset_posture(qreal a1, qreal a0)
{
    quint8 cnt = 0;

    _angle23 = 80.0;
    set_posture(a1, a0);

    while(1){
        if(qAbs(can_dev1->get_angle() - a1) < 0.5 &&
           qAbs(can_dev0->get_angle() - a0) < 0.5)
            break;

        if(cnt == 10)
            return false;

        cnt++;
        QThread::msleep(400);
    }

    _angle23 = 60.0;
    return true;
}

void ARM::auto_move()
{
    wait.wakeAll();
}

void ARM::move_left(qreal d)
{
    QPointF p(_pos2.x() - d, _pos2.y());
    set_p2_position(p);
}

void ARM::move_right(qreal d)
{
    QPointF p(_pos2.x() + d, _pos2.y());
    set_p2_position(p);
}

void ARM::move_up(qreal d)
{
    QPointF p(_pos2.x(), _pos2.y() + d);
    set_p2_position(p);
}

void ARM::move_down(qreal d)
{
    QPointF p(_pos2.x(), _pos2.y() - d);
    set_p2_position(p);
}

void ARM::move_s2_left(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;

    QMatrix m;
    m.translate(-p2.x(), -p2.y());
    p3 = p3 * m;

    m.reset();
    m.rotate(a);
    p3 = p3 * m;

    m.reset();
    m.translate(p2.x(), p2.y());
    p3 = p3 * m;

    _pos3 = p3;
    _angle2 -= a;
    can_dev2->set_angle(_angle2);
}

void ARM::move_s2_right(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;

    QMatrix m;
    m.translate(-p2.x(), -p2.y());
    p3 = p3 * m;

    m.reset();
    m.rotate(-a);
    p3 = p3 * m;

    m.reset();
    m.translate(p2.x(), p2.y());
    p3 = p3 * m;

    _pos3 = p3;
    _angle2 += a;
    can_dev2->set_angle(_angle2);
}

void ARM::move_s1_left(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;
    QPointF p1 = _pos1;

    QMatrix m;
    m.translate(-p1.x(), -p1.y());

    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.rotate(a);
    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.translate(p1.x(), p1.y());
    p3 = p3 * m;
    p2 = p2 * m;

    _pos3 = p3;
    _pos2 = p2;

    _angle1 -= a;
    can_dev1->set_angle(_angle1);
}

void ARM::move_s1_right(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;
    QPointF p1 = _pos1;

    QMatrix m;
    m.translate(-p1.x(), -p1.y());

    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.rotate(-a);
    p3 = p3 * m;
    p2 = p2 * m;

    m.reset();
    m.translate(p1.x(), p1.y());
    p3 = p3 * m;
    p2 = p2 * m;

    _pos3 = p3;
    _pos2 = p2;

    _angle1 += a;
    can_dev1->set_angle(_angle1);
}

void ARM::move_s0_left(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;
    QPointF p1 = _pos1;
    QPointF p0 = _pos0;

    QMatrix m;
    m.translate(-p0.x(), -p0.y());

    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.rotate(a);
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.translate(p0.x(), p0.y());
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    _pos3 = p3;
    _pos2 = p2;
    _pos1 = p1;

    _angle0 -= a;
    can_dev0->set_angle(_angle0);
}

void ARM::move_s0_right(qreal a)
{
    QPointF p3 = _pos3;
    QPointF p2 = _pos2;
    QPointF p1 = _pos1;
    QPointF p0 = _pos0;

    QMatrix m;
    m.translate(-p0.x(), -p0.y());

    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.rotate(-a);
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    m.reset();
    m.translate(p0.x(), p0.y());
    p3 = p3 * m;
    p2 = p2 * m;
    p1 = p1 * m;

    _pos3 = p3;
    _pos2 = p2;
    _pos1 = p1;

    _angle0 += a;
    can_dev0->set_angle(_angle0);
}

#include <QCanBusFrame>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QQueue>
#include <innfos.h>
#include <QTime>
#include <can.h>
#include <QThread>

CanBus * CanBus::canbus = nullptr;

CanBus * CanBus::get_instance()
{
    if(canbus)
        return canbus;

    bool ok;
    QString errorString;
    QCanBusDevice *device = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),
                                        QStringLiteral("can0"), &errorString);
    if (!device){
        qDebug() << errorString;
        return nullptr;
    }
    else
        ok = device->connectDevice();

    if(!ok){
        qDebug() << "connected failed";
        delete device;
        return nullptr;
    }

    canbus = new CanBus();
    canbus->device = device;

    QObject::connect(device, &QCanBusDevice::errorOccurred, canbus, &CanBus::on_errorOccurred);
    QObject::connect(device, &QCanBusDevice::framesReceived, canbus, &CanBus::on_framesReceived);
    QObject::connect(device, &QCanBusDevice::framesWritten, canbus, &CanBus::on_framesWritten);

    return canbus;
}

void CanBus::write_frame(quint8 id, quint8 *data, quint8 len)
{
    QCanBusFrame frame;
    frame.setFrameId(id);
    frame.setPayload(QByteArray((const char *)data, len));
    frame.setFrameType(QCanBusFrame::DataFrame);

    if(!device->writeFrame(frame))
        qDebug() << QByteArray((const char *)data, len).toHex() << "write error";
}

void CanBus::add_device(quint8 id, CanDevice *device)
{
    if(id > 10)
        return;

    this->devices_id.insert(id);
    this->devices[id] = device;
}

void CanBus::on_errorOccurred(QCanBusDevice::CanBusError error)
{
    qDebug() << error;
}

void CanBus::on_framesReceived()
{
    quint8 id;

    while(device->framesAvailable()){
        QCanBusFrame frame = device->readFrame();

        id = frame.frameId();
        if(this->devices_id.contains(id))
            this->devices[id]->get_frame(frame);
    }
}

void CanBus::on_framesWritten(qint64 framesCount)
{

}

CanBus::~CanBus()
{
    if(device)
        device->disconnectDevice();

    delete device;
    delete canbus;
}

CanDevice::CanDevice()
{
    timer = new QTimer();
    timer->setInterval(800);
    connect(timer, &QTimer::timeout, this, [this]{
        timer->stop();
        emit disconnected();
    });

    timer_query = new QTimer();
    timer_query->setInterval(500);
    connect(timer_query, &QTimer::timeout, this, &CanDevice::query);
}

CanDevice::~CanDevice()
{
    delete timer;
    delete timer_query;
}

void CanDevice::get_position()
{
    quint8 cmd[10] = {GET_CURRT_P};
    CanBus::get_instance()->write_frame(this->id, cmd, 1);
}

void CanDevice::get_temperature()
{
    quint8 cmd[10] = {GET_DEVICE_TEMPR};
    CanBus::get_instance()->write_frame(this->id, cmd, 1);

    cmd[0] = GET_IVERTR_TEMPR;
    CanBus::get_instance()->write_frame(this->id, cmd, 1);
}

void CanDevice::get_mode()
{
    quint8 cmd[10] = {GET_MODE};
    CanBus::get_instance()->write_frame(this->id, cmd, 1);
}

void CanDevice::start()
{
    quint8 cmd[] = {SET_DEVICE_STATE, ENABLE};
    CanBus::get_instance()->write_frame(this->id, cmd, 2);

    timer_query->start();
    timer->start();
}

void CanDevice::init()
{
    quint8 cmd[] = {SET_MODE, POSCURVE_MODE};
    CanBus::get_instance()->write_frame(this->id, cmd, 2);
}

void CanDevice::stop()
{
    quint8 cmd[] = {SET_DEVICE_STATE, DISABLE};
    CanBus::get_instance()->write_frame(this->id, cmd, 2);

    timer_query->stop();
}

void CanDevice::hello()
{
    quint8 cmd[] = {HANDSHAKE};
    CanBus::get_instance()->write_frame(this->id, cmd, 1);
}

void CanDevice::set_id(quint8 id)
{
    this->id = id;
}

void CanDevice::set_radio(quint8 radio)
{
    this->radio = radio;
}

void CanDevice::set_normal(bool f)
{
    normal = f;
}

void CanDevice::set_angle(float angle)
{
    float pos, d;

    if(!normal)
        angle = -angle;

    d = qAbs(angle-this->angle);
    if(0 < d && d <= 10)
        this->set_max_speed(30);
    else if(10 < d && d <= 20)
        this->set_max_speed(40);
    else if(20 < d && d <= 30)
        this->set_max_speed(50);
    else if(30 < d && d <= 40)
        this->set_max_speed(60);
    else if(40 < d && d <= 50)
        this->set_max_speed(70);
    else if(50 < d && d <= 90)
        this->set_max_speed(80);
    else if(90 < d && d <= 180)
        this->set_max_speed(100);
    else
        this->set_max_speed(30);

    pos = angle / 360.0 * this->radio;    
    this->set_position(pos);
    this->angle = angle;
}

void CanDevice::set_max_speed(quint16 s)
{
    int speed = IQ24(s);
    quint8 cmd[10] = {SET_POSCURVE_MAX_V, };

    cmd[1] = (speed>>24) & 0xff;
    cmd[2] = (speed>>16) & 0xff;
    cmd[3] = (speed>>8) & 0xff;
    cmd[4] = (speed>>0) & 0xff;
    CanBus::get_instance()->write_frame(this->id, cmd, 5);
}

void CanDevice::set_position(float p)
{
    int position = IQ24(p);
    quint8 cmd[10] = {SET_CURRT_P, };
    cmd[1] = (position>>24) & 0xff;
    cmd[2] = (position>>16) & 0xff;
    cmd[3] = (position>>8) & 0xff;
    cmd[4] = (position>>0) & 0xff;
    CanBus::get_instance()->write_frame(this->id, cmd, 5);
    this->pos = p;
}

void CanDevice::clear_error()
{
    quint8 cmd[] = {CLEAR_ALARM, };
    CanBus::get_instance()->write_frame(this->id, cmd, 1);
}

void CanDevice::get_frame(QCanBusFrame frame)
{
    quint8 *data;
    float position, tempf;
    float angle;
    int pos, temp, alarm;

    if(!frame.isValid())
        return;

    if(frame.frameType() == QCanBusFrame::DataFrame){

        data = (quint8 *)frame.payload().data();

        switch (data[0]) {
        case HANDSHAKE:
            if(data[1] == 0x01){
                this->status = true;
                emit connected();
                if(this->timer->isActive())
                    this->timer->stop();
                this->timer->start();
            }
            break;
        case GET_LAST_STATE:
//            if(data[1] == 0x00)
//                this->clear_error();
            break;
        case GET_DEVICE_TEMPR:
            temp = 0;
            temp |= (data[1] << 8);
            temp |= (data[2]);

            tempf = _IQ8(temp);
            this->device_temp = tempf;
            break;
        case GET_IVERTR_TEMPR:
            temp = 0;
            temp |= (data[1] << 8);
            temp |= (data[2]);

            tempf = _IQ8(temp);
            this->iverter_temp = tempf;
            break;

        case SET_POSCURVE_MAX_V:
            if(data[1] != 0x01)
                qDebug() << "set max speed error";

            break;
        case GET_ALARM_INFO:
            alarm = 0;
            alarm |= (data[1] << 8);
            alarm |= (data[2]);

            this->alarm = alarm;
            break;
        case GET_CURRT_P:

            pos = 0;
            pos |= (data[1] << 24);
            pos |= (data[2] << 16);
            pos |= (data[3] << 8);
            pos |= (data[4]);

            position = _IQ24(pos);
            this->pos = position;

            angle = position / this->radio * 360.0;
            this->angle = angle;
            break;
        case SET_MODE:
            if(data[1] == 0x01)
                this->get_mode();
            break;
        case GET_MODE:
            if(data[1] == POSCURVE_MODE)
                qDebug() << "device " << this->id << " set mode ok";
            else
                qDebug() << "device " << this->id << " set mode failed";
            break;
        case SET_DEVICE_STATE:
            if(data[1] == 0x01){
                if(started){
                    qDebug() << "stop " << this->id << " ok";
                    started = false;
                }else{
                    started = true;
                    qDebug() << "start " << this->id << " ok";
                    QThread::msleep(500);
                    this->init();
                }
            }else{
                qDebug() << "set device " << this->id << " state failed";
                this->started = false;
            }
            break;
        case CLEAR_ALARM:
            if(data[1] == 0x00)
                qDebug() << "device " << this->id << " clear alarm failed";
            break;

        default:
            break;
        }

    }else if(frame.frameType() == QCanBusFrame::RemoteRequestFrame){

    }else if(frame.frameType() == QCanBusFrame::ErrorFrame){

    }else{

    }
}

void CanDevice::query()
{
    this->hello();
    this->get_position();
    this->get_temperature();
}


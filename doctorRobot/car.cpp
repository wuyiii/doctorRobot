#include "car.h"
#include <QDebug>

Car::Car(QObject *parent)
    : QObject(parent)
{
    serial = new QSerialPort(parent);
    connect(serial, &QSerialPort::readyRead, this, &Car::on_serial_readyRead);
}

Car::~Car()
{
    delete serial;
}

bool Car::setup_uart(QString name)
{
    serial->setPortName(name);

    if(serial->isOpen())
        serial->close();

    serial->setBaudRate(115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);

    if(!serial->open(QIODevice::ReadWrite)){
        qDebug() << "open" << name << "error" << serial->error();
        return false;
    }

    return true;
}

void Car::start()
{
    if (setup_uart("/dev/ttyUSB0")){
        status = true;
        buf = (char *)malloc(BUFNUM);
        pos = 0;
    }else
        status = false;
}

void Car::send(const char *data)
{
    if (!serial->isOpen())
        return;

    mutex.lock();
    serial->write(data);
    mutex.unlock();
    qDebug("CAR [W]: %s", data);
}

void Car::move_ahead(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", 75+25*l, 75+25*r);
    send(dat);
}

void Car::move_left_ahead(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", 75+25*(r-l), 150+25*r);
    send(dat);
}

void Car::move_right_ahead(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", 150+25*r, 75+25*(r-l));
    send(dat);
}

void Car::move_back(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", -(75+25*l), -(75+25*r));
    send(dat);
}

void Car::move_left_back(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", -(75+25*(r-l)), -(150+25*r));
    send(dat);
}

void Car::move_right_back(char l, char r)
{
    char dat[20] = {0};
    sprintf(dat, "!M %d %d\r", -(150+25*r), -(75+25*(r-l)));
    send(dat);
}

void Car::move_left(char l, char r)
{
    char dat[20] = {0};

    sprintf(dat, "!M %d %d\r", l, 75+25*r);
    send(dat);
}

void Car::move_right(char l, char r)
{
    char dat[20] = {0};

    sprintf(dat, "!M %d %d\r", 75+25*l, r);
    send(dat);
}

void Car::stop()
{
    if(status){
        serial->close();
        free(buf);
        pos = 0;
    }

    status = false;
}

void Car::on_serial_readyRead()
{
    int i;
    char *p;
    qint64 ret;

    ret = serial->readLine(buf+pos, 20);
    if(ret < 0){
        qDebug() << "serial read error";
        return;
    }

    p = buf;
    for (i = 0; i < pos+ret; i++){
        if(buf[i] == 0x0D){
            buf[i] = 0;
            qDebug("CAR [R]: %s", p);
            if(i == (pos+ret-1)){
                pos = 0;
                return;
            }else
                p = buf+i+1;
        }
    }

    pos += ret;

    return;
}


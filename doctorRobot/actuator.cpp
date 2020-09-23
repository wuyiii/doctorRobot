#include "innfos_net.h"
#include "actuator.h"
#include "controller.h"
#include <QDebug>
#include <iostream>
#include <QHostInfo>
#include <QThread>

const uint8_t chCRCHTable[] =                                 // CRC 高位字节值表
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

const uint8_t chCRCLTable[] =                                 // CRC 低位字节值表
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40
};

static uint16_t CRC16_1(uint8_t* pchMsg, int16_t wDataLen)
{
    uint8_t chCRCHi = 0xFF;     // 高CRC字节初始化
    uint8_t chCRCLo = 0xFF;     // 低CRC字节初始化
    int16_t wIndex;             // CRC循环中的索引

    while (wDataLen--)
    {
        // 计算CRC
        wIndex = chCRCHi ^ *pchMsg++;
        chCRCHi = chCRCLo ^ chCRCHTable[wIndex];
        chCRCLo = chCRCLTable[wIndex];
    }

    return static_cast<uint16_t>((chCRCHi << 8) | chCRCLo);
}

static void print_array(char mode, quint32 cnt, QByteArray dat)
{
    if(dat.size() == 0)
        return;

    if(dat.at(2) != 0x0A)
        return;

    if(mode == 'r')
        printf("read [%03d]: ", cnt);
    else if(mode == 'w')
        printf("write[%03d]: ", cnt);

    for(int i = 0; i < dat.length(); i++){
        uchar tmp = dat.at(i);
        printf("%02X ", tmp);
    }

    printf("\n");fflush(stdout);
}

ECB *ECB::ecb = nullptr;

ECB *ECB::instance()
{    
    if(ecb)
        return ecb;

    ecb = new ECB;
    return ecb;
}

static int check(quint8 cmd)
{
    int ck = 0;

    switch(cmd){
        case HANDSHAKE:
        case GET_MODE:
        case GET_LAST_STATE:
        case GET_CURRT_STATE:
        case GET_VELCY_STATE:
        case GET_POSITN_STATE:
        case GET_CURRT_BANDWT:
        case GET_VELCY_BANDWT:
        case GET_POSITN_BANDWT:
        case GET_DEVICE_TEMPR:
        case GET_IVERTR_TEMPR:
        case GET_ALARM_INFO:
        case GET_CURRT_C:
        case GET_CURRT_V:
        case GET_CURRT_P:
        case GET_POSITN_UPBAND:
        case GET_POSITN_DNBAND:
            ck = 0;
            break;
        case SET_MODE:
        case SET_CURRT_FILTER:
        case SET_VELCY_FILTER:
        case SET_POSITN_FILTER:
        case SET_DEVICE_STATE:
        case SET_CURRT_C:
        case SET_CURRT_V:
        case SET_CURRT_P:
        case SET_POSCURVE_MAX_V:
        case SET_POSCURVE_ACERT:
        case SET_POSCURVE_DCERT:
        case SET_POSITN_UPBADN:
        case SET_POSITN_DNBADN:
        case SET_DEVICE_POS_UP:
        case SET_DEVICE_POS_DN:
        case SET_HOME:
        case CLEAR_ALARM:
        case CLEAR_HOME:
        case SAVE_TO_EEPROM:
        case ECB_START:
            ck = 1;
            break;
        default:
            ck = 0;
            break;
    }

    return ck;
}

void ECB::send_frame(quint8 id, char cmd, char *data, quint8 len)
{
    static quint32 frame_cnt = 0;
    quint16 checksum;
    QByteArray frame;

    frame += 0xEE;
    frame += id;

    frame += cmd;

    frame += len / 256;
    frame += len % 256;
    frame.append(data, len);

    if(check(cmd)){
        checksum = CRC16_1((uint8_t *)data, len);
        frame += checksum / 256;
        frame += (char)(checksum % 256);
    }

    frame += 0xED;

    mutex.lock();
    writeDatagram(frame.data(), frame.length(), QHostAddress(ECB_ADDRESS), ECB_UDP_PORT);
    mutex.unlock();

//    print_array('w', frame_cnt++, frame);
    if(frame_cnt == 1000)
        frame_cnt = 0;

    return;
}

void ECB::hello()
{
    send_frame(0x00, ECB_HANDSHAKE, nullptr, 0);
}

void ECB::query()
{
    send_frame(0x00, GET_ADDRESS, nullptr, 0);
}

void ECB::start()
{
    char cmd[] = {0x04};
    send_frame(0x00, ECB_START, cmd, 1);
}

void ECB::devices()
{
    qDebug() << "ECB: " << devset;
}


void ECB::add_device(const quint8 id, Actuator *device)
{
    if(id > 10)
        return;

    this->devset.insert(id);
    this->dev[id] = device;
}

void ECB::on_readyRead()
{
    quint8 addr;
    static quint8 hello_cnt = 0;
    static quint32 frame_cnt = 0;
    QByteArray frame;

    while(hasPendingDatagrams())
    {
        frame.resize((int)pendingDatagramSize());
        readDatagram(frame.data(), frame.size());

//        print_array('r', frame_cnt++, frame);
        if(frame_cnt == 1000)
            frame_cnt = 0;

        if(!frame.startsWith(0xEE) || !frame.endsWith(0xED))
            continue;

        addr = frame.at(1);

        if(!inited){
            if(frame.at(2) == ECB_HANDSHAKE){
                if(hello_cnt == 1){
                    hello_cnt = 0;
                    inited = true;

                }else
                    hello_cnt++;
                continue;
            }

            if(frame.at(2) == GET_ADDRESS)
                if(!devset.contains(addr))
                    devset.insert(addr);

        }else {
            if(addr < 10 && addr > 0)
                dev[addr]->get_frame(frame);
        }
    }
}

ECB::ECB(QObject *parent)
    : QUdpSocket(parent)
{
    connect(this, &ECB::readyRead, this, &ECB::on_readyRead);
}

ECB::~ECB()
{
    delete ecb;
}

void ECB::init()
{
    bool ret;

    ret = bind(QHostAddress(IP_ADDRESS), IP_UDP_PORT);
    if(!ret){
        qDebug() << "ECB can't bind to " << IP_ADDRESS << IP_UDP_PORT;
        stats = false;
    }else
        stats = true;
}

void ECB::stop()
{
    this->close();
    this->ecb = nullptr;
}

void Actuator::open()
{
    char cmd[] = {ENABLE};
    ECB::instance()->send_frame(this->id, SET_DEVICE_STATE, cmd, 1);
}

void Actuator::set_position_mode()
{
    char cmd[] = {POSCURVE_MODE};
    ECB::instance()->send_frame(this->id, SET_MODE, cmd, 1);
}

void Actuator::set_current_mode()
{
    char cmd[] = {CURRENT_MODE};
    ECB::instance()->send_frame(this->id, SET_MODE, cmd, 1);
}

void Actuator::close()
{
    char cmd[] = {DISABLE};
    ECB::instance()->send_frame(this->id, SET_DEVICE_STATE, cmd, 1);

    started = false;
    state = Closed;
}

void Actuator::hello()
{
    ECB::instance()->send_frame(this->id, HANDSHAKE, nullptr, 0);
}

void Actuator::get_position()
{
    ECB::instance()->send_frame(this->id, GET_CURRT_P, nullptr, 0);
}

void Actuator::get_temperature()
{
    ECB::instance()->send_frame(this->id, GET_DEVICE_TEMPR, nullptr, 0);
    ECB::instance()->send_frame(this->id, GET_IVERTR_TEMPR, nullptr, 0);
}

void Actuator::get_mode()
{
    ECB::instance()->send_frame(this->id, GET_MODE, nullptr, 0);
}

void Actuator::get_state()
{
    ECB::instance()->send_frame(this->id, GET_DEVICE_STATE, nullptr, 0);
}

void Actuator::set_id(quint8 id)
{
    this->id = id;
}

void Actuator::set_radio(quint8 radio)
{
    this->radio = radio;
}

void Actuator::set_normal(bool f)
{
    normal = f;
}

void Actuator::set_angle(qreal a)
{
    quint8 speed;
    qreal pos, d;

    a += offset;

    if(!normal)
        a = -a;

    d = qAbs(a - this->angle);
    if(0 < d && d <= 10)
        speed = 30;
    else if(10 < d && d <= 20)
        speed = 35;
    else if(20 < d && d <= 30)
        speed = 40;
    else if(30 < d && d <= 40)
        speed = 45;
    else if(40 < d && d <= 50)
        speed = 50;
    else if(50 < d && d <= 90)
        speed = 55;
    else if(90 < d && d <= 180)
        speed = 60;
    else
        speed = 30;

//    this->angle = a;
    pos = a / 360 * this->radio;

    this->set_max_speed(speed);
    QThread::msleep(5);
    this->set_position(pos);

    qDebug() << this->id << pos;
}

void Actuator::set_offset(qreal o)
{
    offset = o;
}

void Actuator::set_max_speed(quint16 s)
{
    int speed = IQ24(s);
    char cmd[10] = {0};

    cmd[0] = (speed>>24) & 0xff;
    cmd[1] = (speed>>16) & 0xff;
    cmd[2] = (speed>>8) & 0xff;
    cmd[3] = (speed>>0) & 0xff;

    ECB::instance()->send_frame(this->id, SET_POSCURVE_MAX_V, cmd, 4);
}

void Actuator::set_position(qreal p)
{
    int position = IQ24(p);
    char cmd[10] = {0};

    cmd[0] = (position>>24) & 0xff;
    cmd[1] = (position>>16) & 0xff;
    cmd[2] = (position>>8) & 0xff;
    cmd[3] = (position>>0) & 0xff;
    ECB::instance()->send_frame(this->id, SET_CURRT_P, cmd, 4);
    this->pos = p;
}

void Actuator::clear_error()
{
    ECB::instance()->send_frame(this->id, CLEAR_ALARM, nullptr, 0);
}

void Actuator::get_frame(QByteArray &frame)
{
    qreal position, tempf;
    int p, temp;

    switch (frame.at(2)) {
        // 读命令
        case HANDSHAKE:
            if(frame.at(5) == 0x01){
                this->status = true;
                emit connected();
//                if(this->timer->isActive())
//                    this->timer->stop();
//                this->timer->start();
            }
            break;
        case GET_LAST_STATE:
            if(frame.at(5) == 0x00)
                this->clear_error();
            break;
        case GET_DEVICE_TEMPR:
            temp = 0;
            temp |= (frame.at(5) << 8);
            temp |= (frame.at(6));

            tempf = IQR8(temp);
            this->device_temp = tempf;
            break;
        case GET_IVERTR_TEMPR:
            temp = 0;
            temp |= (frame.at(5) << 8);
            temp |= (frame.at(6));

            tempf = IQR8(temp);
            this->iverter_temp = tempf;
            break;
        case GET_ALARM_INFO:

            break;
        case GET_MODE:
            if(frame.at(5) == POSCURVE_MODE || frame.at(5) == CURRENT_MODE){
                state = SetMode;
                qDebug() << "device " << this->id << " set mode ok";
            }
            else
                qDebug() << "device " << this->id << " set mode failed";
            break;
        case GET_DEVICE_STATE:
            if(frame.at(5) == 0x01){
                started = true;
                state = Opened;
                qDebug() << "start " << this->id << " ok already";
            }
            break;
        case GET_CURRT_P:

            p = 0;
            p |= (frame.at(5) << 24);
            p |= (frame.at(6) << 16);
            p |= (frame.at(7) << 8);
            p |= (frame.at(8));

            position = IQR24(p);
            this->pos = position;

            this->angle = position / this->radio * 360 - offset;
            break;

        // 写命令
        case SET_MODE:
            if(frame.at(5) == 0x01)
                this->get_mode();
            break;

        case SET_DEVICE_STATE:
            if(frame.at(5) == 0x01){
                if(started){
                    qDebug() << "stop " << this->id << " ok";
                    started = false;
                    state = Closed;
                }else{
                    started = true;
                    state = Opened;
                    qDebug() << "start " << this->id << " ok";
                }
            }else{
                qDebug() << "set device " << this->id << " state failed";
                this->started = false;
            }
            break;
        case CLEAR_ALARM:
            if(frame.at(5) == 0x00)
                qDebug() << "clear device alarm failed";
            break;
        case SET_POSCURVE_MAX_V:
            if(frame.at(5) != 0x01)
                qDebug() << "set max speed error";

            break;

        default:
            break;
    }
}

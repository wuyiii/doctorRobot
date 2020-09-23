#include <pwm.h>
#include <QFile>
#include <QString>

//持有pwm硬件信息 name, chip, index
quint8 pwm_info[][2] = {
    {1, 0}, //pwm0
    {2, 0}, //pwm1
};

// 1.0ms-2.0ms/20ms

PWM::PWM(quint8 chip, quint8 index)
{
    this->chip = chip;
    this->index = index;
}

bool PWM::init()
{
    qint64 num = 0;
    bool ret = false;

    QString export_file_dir = QString("/sys/class/pwm/pwmchip%1/export").arg(chip);
    QFile export_file(export_file_dir);
    ret = export_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", export_file_dir.toLocal8Bit().data());
        return false;
    }

    quint8 cmd = index+0x30;
    num = export_file.write((const char *)&cmd, 1);
    export_file.close();
    if(num < 0){
        qDebug("write export error");
        return false;
    }

    QString pwm_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2").arg(chip).arg(index);
    ret = QFile::exists(pwm_dir);
    if(!ret){
        qDebug("%s doesn't exists", pwm_dir.toLocal8Bit().data());
        return false;
    }

    qDebug("create %s success", pwm_dir.toLocal8Bit().data());
    return true;
}

bool PWM::deinit()
{
    qint64 num = 0;
    bool ret = false;

    QString unexport_file_dir = QString("/sys/class/pwm/pwmchip%1/unexport").arg(chip);
    QFile unexport_file(unexport_file_dir);
    ret = unexport_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", unexport_file_dir.toLocal8Bit().data());
        return false;
    }

    quint8 cmd = index+0x30;
    num = unexport_file.write((const char *)&cmd, 1);
    unexport_file.close();
    if(num < 0){
        qDebug("write unexport error");
        return false;
    }

    QString pwm_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2").arg(chip).arg(index);
    ret = QFile::exists(pwm_dir);
    if(!ret)
        qDebug("%s still exists", pwm_dir.toLocal8Bit().data());
    else
        qDebug("%s deleted", pwm_dir.toLocal8Bit().data());

    return true;
}

bool PWM::enable()
{
    qint64 num = 0;
    bool ret = false;

    QString enable_file_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2/enable").arg(chip).arg(index);
    QFile enable_file(enable_file_dir);
    ret = enable_file.open(QIODevice::ReadWrite);
    if(!ret){
        qDebug("can't open %s", enable_file_dir.toLocal8Bit().data());
        return false;
    }

    num = enable_file.write("1", 1);
    enable_file.close();
    if(num < 0){
        qDebug("can't write 1 to %s", enable_file_dir.toLocal8Bit().data());
        return false;
    }

    return true;
}

bool PWM::disable()
{
    qint64 num = 0;
    bool ret = false;

    QString enable_file_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2/enable").arg(chip).arg(index);
    QFile enable_file(enable_file_dir);
    ret = enable_file.open(QIODevice::ReadWrite);
    if(!ret){
        qDebug("can't open %s", enable_file_dir.toLocal8Bit().data());
        return false;
    }

    num = enable_file.write("0", 1);
    enable_file.close();
    if(num < 0){
        qDebug("can't write 0 to %s", enable_file_dir.toLocal8Bit().data());
        return false;
    }

    return true;
}

bool PWM::set_period(quint32 p)
{
    qint64 num = 0;
    bool ret = false;

    QString period_file_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2/period").arg(chip).arg(index);
    QFile period_file(period_file_dir);
    ret = period_file.open(QIODevice::ReadWrite);
    if(!ret){
        qDebug("can't open %s", period_file_dir.toLocal8Bit().data());
        return false;
    }

    QByteArray value = QString("%1").arg(p).toLocal8Bit();

    num = period_file.write(value);
    period_file.close();
    if(num < 0){
        qDebug("can't write to %s", period_file_dir.toLocal8Bit().data());
        return false;
    }

    this->period = p;

    return true;
}

bool PWM::set_duty(quint32 d)
{
    qint64 num = 0;
    bool ret = false;

    if(d > this->period){
        qDebug("duty is bigger than period");
        return false;
    }

    QString duty_file_dir = QString("/sys/class/pwm/pwmchip%1/pwm%2/duty_cycle").arg(chip).arg(index);
    QFile duty_file(duty_file_dir);
    ret = duty_file.open(QIODevice::ReadWrite);
    if(!ret){
        qDebug("can't open %s", duty_file_dir.toLocal8Bit().data());
        return false;
    }

    QByteArray value = QString("%1").arg(d).toLocal8Bit();

    num = duty_file.write(value);
    duty_file.close();
    if(num < 0){
        qDebug("can't write to %s", duty_file_dir.toLocal8Bit().data());
        return false;
    }

    this->duty = d;

    float aa = 90.0*(d-1000000)/1000000 - 45;
    qDebug("[%d] %d %f", this->chip, d, aa);

    return true;
}

quint32 PWM::get_period()
{
    return period;
}

quint32 PWM::get_duty()
{
    return duty;
}

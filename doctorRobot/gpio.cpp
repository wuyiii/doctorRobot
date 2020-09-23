#include <gpio.h>
#include <QFile>
#include <QString>

/* 持有GPIO硬件信息 chip, index IO
 *   IO从IO0到IO31，index无需再减1。可用的引脚位于J28接口上，可用GPIO需要查看设备树文件 iomux{hoggrp}节点下的配置
 *   MX8MQ_IOMUXC_SPDIF_TX_GPIO5_IO3 0x16
 *   MX8MQ_IOMUXC_SPDIF_RX_GPIO5_IO4 0x16
 *   MX8MQ_IOMUXC_SPDIF_EXT_CLK_GPIO5_IO5 0x16
 *
 * */
qint8 gpio_info[][2] = {
    {5, 3}, //GPIO5_IO3
    {5, 4}, //GPIO5_IO4
};

bool GPIO::open()
{
    qint64 num = 0;
    bool ret = false;

    QFile export_file("/sys/class/gpio/export");
    ret = export_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", export_file.fileName().toLocal8Bit().data());
        return false;
    }

    QString cmd = QString("%1").arg(index);
    num = export_file.write((const char *)cmd.toLocal8Bit().data(), cmd.length());
    export_file.close();
    if(num < 0){
        qDebug("write export error");
        return false;
    }

    QString gpio_dir = QString("/sys/class/gpio/gpio%1").arg(index);
    ret = QFile::exists(gpio_dir);
    if(!ret){
        qDebug("%s doesn't exists", gpio_dir.toLocal8Bit().data());
        return false;
    }

    qDebug("create %s success", gpio_dir.toLocal8Bit().data());
    return true;
}

bool GPIO::setup()
{
    qint64 num = 0;
    bool ret = false;

    QString file_name = QString("/sys/class/gpio/gpio%1/direction").arg(index);
    QFile direction_file(file_name);
    ret = direction_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", file_name.toLocal8Bit().data());
        return false;
    }

    char cmd[] = "out";
    num = direction_file.write((const char *)cmd, 3);
    direction_file.close();
    if(num < 0){
        qDebug("write export error");
        return false;
    }

    return true;
}

void GPIO::setid(int idx)
{
    this->index = idx;
}

void GPIO::setid(int chip, int cnt)
{
    this->chip = chip;
    this->cnt = cnt;
    this->index = (chip - 1) * 32 + cnt;
}

bool GPIO::output(bool high)
{
    qint64 num = 0;
    bool ret = false;

    QString file_name = QString("/sys/class/gpio/gpio%1/value").arg(index);
    QFile value_file(file_name);
    ret = value_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", file_name.toLocal8Bit().data());
        return false;
    }

    char cmd[1];
    if(high)
        cmd[0] = '0';
    else
        cmd[0] = '1';

    num = value_file.write((const char *)cmd, 1);
    value_file.close();
    if(num < 0){
        qDebug("write export error");
        return false;
    }

    return true;
}

bool GPIO::close()
{
    qint64 num = 0;
    bool ret = false;

    QFile unexport_file("/sys/class/gpio/unexport");
    ret = unexport_file.open(QIODevice::WriteOnly);
    if(!ret){
        qDebug("open %s error", unexport_file.fileName().toLocal8Bit().data());
        return false;
    }

    QString cmd = QString("%1").arg(index);
    num = unexport_file.write((const char *)cmd.toLocal8Bit().data(), cmd.length());
    unexport_file.close();
    if(num < 0){
        qDebug("write export error");
        return false;
    }

    QString gpio_dir = QString("/sys/class/gpio/gpio%1").arg(index);
    ret = QFile::exists(gpio_dir);
    if(ret)
        qDebug("%s still exists", gpio_dir.toLocal8Bit().data());
    else
        qDebug("%s deleted", gpio_dir.toLocal8Bit().data());

    return true;
}

#ifndef GPIO_H
#define GPIO_H

#include <QObject>

extern qint8 gpio_info[][2];

class GPIO : public QObject
{
    Q_OBJECT

public:
    bool open();
    bool setup();
    void setid(int idx);
    void setid(int chip, int cnt);
    bool output(bool high = true);
    bool close();

private:
    int chip;
    int cnt;
    int index;  //总索引(0开始)
};

#endif // GPIO_H

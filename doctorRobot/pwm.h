#ifndef PWM_H
#define PWM_H

#include <QObject>

class QFile;

extern quint8 pwm_info[][2];

class PWM : public QObject
{
    Q_OBJECT

public:
    PWM(quint8 chip, quint8 index);

    bool init();
    bool deinit();
    bool enable();
    bool disable();
    bool set_period(quint32 p);
    bool set_duty(quint32 d);

    quint32 get_period();
    quint32 get_duty();

private:
    quint8 chip;
    quint8 index;
    quint32 period;
    quint32 duty;
};

#endif


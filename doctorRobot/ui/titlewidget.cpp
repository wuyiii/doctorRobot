#include "titlewidget.h"
#include <QLabel>
#include <QTime>
#include <QTimerEvent>
#include <QHBoxLayout>

TitleWidget::TitleWidget(QWidget *parent)
    : QFrame(parent)
{
    date = new QLabel(this);
    date->setObjectName("date");

    hint = new QLabel(this);
    hint->setObjectName("hint");

    netstats = new QLabel(this);
    armstats = new QLabel(this);
    carstats = new QLabel(this);

    red.load(":/image/red.png");
    green.load(":/image/green.png");

    netstats->setPixmap(red);
    armstats->setPixmap(red);
    carstats->setPixmap(red);

    layout = new QHBoxLayout();
    layout->setContentsMargins(5,0,5,0);
    layout->addWidget(hint);
    layout->addStretch();

    layout->addWidget(netstats);
    layout->addWidget(armstats);
    layout->addWidget(carstats);

    layout->addWidget(date);

    this->setLayout(layout);

    timer = startTimer(1000);
}

TitleWidget::~TitleWidget()
{
    killTimer(timer);
    delete layout;
}

void TitleWidget::on_hint(QString info)
{
    queue.enqueue(info);
    empty = false;
}

void TitleWidget::on_net_status(bool f)
{
    if (f)
        netstats->setPixmap(green);
    else
        netstats->setPixmap(red);
}

void TitleWidget::on_car_status(bool f)
{
    if (f)
        carstats->setPixmap(green);
    else
        carstats->setPixmap(red);
}

void TitleWidget::on_arm_status(bool f)
{
    if (f)
        armstats->setPixmap(green);
    else
        armstats->setPixmap(red);
}

void TitleWidget::resizeEvent(QResizeEvent *event)
{
    (void)event;
}

void TitleWidget::timerEvent(QTimerEvent *event)
{
    static qint8 cnt = 0;

    if(event->timerId() == timer){
       date->setText(QTime::currentTime().toString("hh:mm:ss"));

       if(queue.empty() && !empty){
           if(cnt == 3){
               hint->clear();
               empty = true;
               cnt = 0;
           }
           cnt++;
       }else if(!queue.empty())
           hint->setText(queue.dequeue());
    }
}

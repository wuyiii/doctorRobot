#include "sidewidget.h"
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include <QLabel>

SideWidget::SideWidget(QWidget *parent)
    : QFrame(parent)
{
    layout = new QVBoxLayout();

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    home = new QPushButton();
    infol = new QPushButton();
    infor = new QPushButton();
    setting = new QPushButton();
    depth = new QPushButton();

    home->setCheckable(true);
    infol->setCheckable(true);
    infor->setCheckable(true);
    setting->setCheckable(true);
    depth->setCheckable(true);

    home->setChecked(true);

    group = new QButtonGroup();
    group->setExclusive(true);
    group->addButton(home, 1);
    group->addButton(infol,2);
    group->addButton(infor,3);
    group->addButton(setting,4);
    group->addButton(depth,5);

    home->setText("主页");
    infol->setText("左臂");
    infor->setText("右臂");
    setting->setText("设置");
    depth->setText("深度");

    layout->addSpacing(4);
    layout->addWidget(home);
    layout->addSpacing(2);
    layout->addWidget(infol);
    layout->addSpacing(2);
    layout->addWidget(infor);
    layout->addSpacing(2);
    layout->addWidget(setting);
    layout->addSpacing(2);
    layout->addWidget(depth);

    this->setLayout(layout);
}

SideWidget::~SideWidget()
{
    delete layout;
}

void SideWidget::resizeEvent(QResizeEvent *event)
{
    home->setFixedSize(event->size().width()-4, 40);
    infol->setFixedSize(event->size().width()-4, 40);
    infor->setFixedSize(event->size().width()-4, 40);
    setting->setFixedSize(event->size().width()-4, 40);
    depth->setFixedSize(event->size().width()-4, 40);
}

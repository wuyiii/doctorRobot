#include "mainwindow.h"
#include <QFile>
#include <QTime>
#include <QPainter>
#include <QPaintEvent>
#include <QToolButton>

#define TITLE_HEIGHT 30
#define SIDE_WIDTH 80

#define SCREEN_HEIGHT   480
#define SCREEN_WIDTH    800

MainWindow::MainWindow(QWidget *parent)
    : QFrame(parent)
{
    sc_width = SCREEN_WIDTH;
    sc_height = SCREEN_HEIGHT;

    l1.setLine(0, TITLE_HEIGHT, sc_width, TITLE_HEIGHT);
    l2.setLine(SIDE_WIDTH, TITLE_HEIGHT, SIDE_WIDTH, sc_height);

    this->setMinimumSize(sc_width, sc_height);
    this->setMaximumSize(sc_width, sc_height);

    QFile style(":/qss/style.qss");
    style.open(QIODevice::ReadOnly);
    this->setStyleSheet(style.readAll().data());
    style.close();

    title = new TitleWidget(this);
    title->setGeometry(0, 0, sc_width, TITLE_HEIGHT);

    sidebar = new SideWidget(this);
    sidebar->setGeometry(0, TITLE_HEIGHT, SIDE_WIDTH, sc_height - TITLE_HEIGHT);

    home = new HomeWidget(this);
    home->setGeometry(SIDE_WIDTH, TITLE_HEIGHT, sc_width - SIDE_WIDTH, sc_height - TITLE_HEIGHT);

    infol = new InfoWidget(this);
    infol->setGeometry(SIDE_WIDTH, TITLE_HEIGHT, sc_width - SIDE_WIDTH, sc_height - TITLE_HEIGHT);
    infol->hide();

    infor = new InfoWidget(this);
    infor->setGeometry(SIDE_WIDTH, TITLE_HEIGHT, sc_width - SIDE_WIDTH, sc_height - TITLE_HEIGHT);
    infor->hide();

    setting = new SettingWidget(this);
    setting->setGeometry(SIDE_WIDTH, TITLE_HEIGHT, sc_width - SIDE_WIDTH, sc_height - TITLE_HEIGHT);
    setting->hide();

    connect(sidebar->group, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &MainWindow::on_buttonClicked);

    controller = new Controller;
    connect(home->start_button, &QToolButton::clicked,  controller,  &Controller::on_start_clicked);
    connect(home->reset_button, &QToolButton::clicked,  controller,  &Controller::on_reset_clicked);
    connect(home->adjust_button, &QToolButton::clicked,  controller,  &Controller::on_adjust_clicked);
    connect(home->auto_button,  &QToolButton::clicked,  controller,  &Controller::on_auto_clicked);

    connect(home->left_rotate_button, &QToolButton::clicked,  controller,  &Controller::on_left_rotate_clicked);
    connect(home->right_rotate_button, &QToolButton::clicked,  controller,  &Controller::on_right_rotate_clicked);

    connect(controller->arm, &ARM::get_angle, this->infol,     &InfoWidget::on_angle_changed);
    connect(controller->arm, &ARM::get_angle_can, this->infol, &InfoWidget::on_angle_can_changed);
    connect(controller->arm, &ARM::get_position, this->infol,  &InfoWidget::on_position_changed);
    connect(controller->arm, &ARM::get_tempt, this->infol,     &InfoWidget::on_tempt_changed);
    connect(controller, &Controller::start_button_status, home, &HomeWidget::set_start_button_status);
    connect(controller, &Controller::reset_button_status, home, &HomeWidget::set_reset_button_status);
    connect(controller, &Controller::adjust_button_status, home, &HomeWidget::set_adjust_button_status);

    connect(controller, &Controller::hint,      title, &TitleWidget::on_hint);
    connect(controller, &Controller::net_stats, title, &TitleWidget::on_net_status);
    connect(controller, &Controller::car_stats, title, &TitleWidget::on_car_status);
    connect(controller, &Controller::arm_stats, title, &TitleWidget::on_arm_status);

//    work = new QThread;
//    work->start();
//    controller->moveToThread(work);
}

MainWindow::~MainWindow()
{
    delete title;
    delete sidebar;
    delete home;
    delete infol;
    delete infor;
    delete setting;
//    delete work;
    delete controller;
}

void MainWindow::on_buttonClicked(int id)
{
    switch(id){
        case 1:
            home->show();
            infol->hide();
            infor->hide();
            setting->hide();
            break;
        case 2:
            home->hide();
            infol->show();
            infor->hide();
            setting->hide();
            break;
        case 3:
            home->hide();
            infol->hide();
            infor->show();
            setting->hide();
            break;
        case 4:
            home->hide();
            infol->hide();
            infor->hide();
            setting->show();
            break;
        case 5:
            home->hide();
            infol->hide();
            infor->hide();
            setting->hide();
            break;
        default:
            break;
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter pint;
    pint.begin(this);
    pint.setPen(qRgb(50,50,50));
    pint.drawLine(l1);
    pint.drawLine(l2);
    pint.end();
}

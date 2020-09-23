#include "infowidget.h"
#include <QDebug>
#include <QVector3D>
#include <QMouseEvent>
#include <qmath.h>
#include <arm.h>

#define WIDGETW 400
#define WIDGETH 400

RenderRegion::RenderRegion(QWidget *parent)
    : QWidget(parent)
{
    this->setMinimumSize(WIDGETW, WIDGETH);
    this->setMaximumSize(WIDGETW, WIDGETH);
}

RenderRegion::~RenderRegion()
{
}

void RenderRegion::set_position(QPointF &p3, QPointF &p2, QPointF &p1, QPointF &p0)
{
    qreal b;

    _p3 = p3;
    _p2 = p2;
    _p1 = p1;
    _p0 = p0;

    b = qTan(qDegreesToRadians(ANGLE));
    b = _p3.y() + b * _p3.x();
    line0.setPoints(QPointF(0, b), _p3);

    b = qTan(qDegreesToRadians(ANGLE));
    b = _p3.y() - b * _p3.x();
    line1.setPoints(QPointF(0, b), _p3);

    update();
}

void RenderRegion::paintEvent(QPaintEvent *event)
{
    (void)event;

    QPainter p;

    p.begin(this);
    p.translate(this->width()/4, this->height()*3/4);
    p.scale(1, -1);
    p.setRenderHint(QPainter::Antialiasing);

    pen1.setColor(Qt::gray);
    pen1.setWidth(1);
    pen1.setStyle(Qt::DashLine);

    p.setPen(pen1);
    p.drawLine(xaxis);
    p.drawLine(yaxis);
    p.drawLine(line0);
    p.drawLine(line1);

    pen1.setWidth(3);
    pen1.setStyle(Qt::SolidLine);

    pen1.setColor(Qt::red);
    p.setPen(pen1);
    p.drawLine(_p0, _p1);

    pen1.setColor(Qt::darkGreen);
    p.setPen(pen1);
    p.drawLine(_p1, _p2);

    pen1.setColor(Qt::blue);
    p.setPen(pen1);
    p.drawLine(_p2, _p3);

    p.end();
}

void RenderRegion::resizeEvent(QResizeEvent *event)
{
    int w,h;

    (void)event;
    w = this->width();
    h = this->height();

    xaxis.setLine(-w/4, 0, w*3/4, 0);
    yaxis.setLine(0, -h/4, 0, h*3/4);
}

InfoRegion::InfoRegion(QWidget *parent)
    : QWidget(parent)
{
    layout = new QGridLayout();
    layout->setAlignment(Qt::AlignCenter | Qt::AlignLeft);

    a2 = new QLabel("0.00");
    a1 = new QLabel("0.00");
    a0 = new QLabel("0.00");

    a5 = new QLabel("(0.00)");
    a4 = new QLabel("(0.00)");
    a3 = new QLabel("(0.00)");

    t5 = new QLabel("0.00");
    t4 = new QLabel("0.00");
    t3 = new QLabel("0.00");

    t2 = new QLabel("0.00");
    t1 = new QLabel("0.00");
    t0 = new QLabel("0.00");

    p7 = new QLabel("0.00");
    p6 = new QLabel("0.00");
    p5 = new QLabel("0.00");
    p4 = new QLabel("0.00");

    p3 = new QLabel("0.00");
    p2 = new QLabel("0.00");
    p1 = new QLabel("0.00");
    p0 = new QLabel("0.00");

    aa2 = new QLabel("电机2角度");
    aa1 = new QLabel("电机1角度");
    aa0 = new QLabel("电机0角度");

    tt2 = new QLabel("电机2温度");
    tt1 = new QLabel("电机1温度");
    tt0 = new QLabel("电机0温度");

    pp3 = new QLabel("喷 嘴位置");
    pp2 = new QLabel("电机2位置");
    pp1 = new QLabel("电机1位置");
    pp0 = new QLabel("电机0位置");

    QFrame *line0 = new QFrame;
    line0->setFrameShape(QFrame::HLine);

    layout->addWidget(aa2, 0, 0, 1, 1);
    layout->addWidget( a2, 0, 1, 1, 1);
    layout->addWidget( a5, 0, 3, 1, 1);
    layout->addWidget(line0, 1, 0, 1, 4);

    QFrame *line1 = new QFrame;
    line1->setFrameShape(QFrame::HLine);

    layout->addWidget(aa1, 2, 0, 1, 1);
    layout->addWidget( a1, 2, 1, 1, 1);
    layout->addWidget( a4, 2, 3, 1, 1);
    layout->addWidget(line1, 3, 0, 1, 4);

    QFrame *line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);

    layout->addWidget(aa0, 4, 0, 1, 1);
    layout->addWidget( a0, 4, 1, 1, 1);
    layout->addWidget( a3, 4, 3, 1, 1);
    layout->addWidget(line2, 5, 0, 1, 4);

    QSpacerItem *spacing0 = new QSpacerItem(5, 20);
    layout->addItem(spacing0, 7, 0, 1, 4);

    QFrame *line4 = new QFrame;
    line4->setFrameShape(QFrame::HLine);

    layout->addWidget(tt2, 8, 0, 1, 1);
    layout->addWidget( t2, 8, 1, 1, 1);
    layout->addWidget( t5, 8, 3, 1, 1);
    layout->addWidget(line4, 9, 0, 1, 4);

    QFrame *line5 = new QFrame;
    line5->setFrameShape(QFrame::HLine);

    layout->addWidget(tt1, 10, 0, 1, 1);
    layout->addWidget( t1, 10, 1, 1, 1);
    layout->addWidget( t4, 10, 3, 1, 1);
    layout->addWidget(line5, 11, 0, 1, 4);

    QFrame *line6 = new QFrame;
    line6->setFrameShape(QFrame::HLine);

    layout->addWidget(tt0, 12, 0, 1, 1);
    layout->addWidget( t0, 12, 1, 1, 1);
    layout->addWidget( t3, 12, 3, 1, 1);
    layout->addWidget(line6, 13, 0, 1, 4);

    QSpacerItem *spacing1 = new QSpacerItem(5, 20);
    layout->addItem(spacing1, 14, 0, 1, 4);

    QFrame *line7 = new QFrame;
    line4->setFrameShape(QFrame::HLine);

    layout->addWidget(pp3, 15, 0, 1, 1);
    layout->addWidget( p3, 15, 1, 1, 1);
    layout->addWidget( p7, 15, 3, 1, 1);
    layout->addWidget(line7, 16, 0, 1, 4);

    QFrame *line8 = new QFrame;
    line4->setFrameShape(QFrame::HLine);

    layout->addWidget(pp2, 17, 0, 1, 1);
    layout->addWidget( p2, 17, 1, 1, 1);
    layout->addWidget( p6, 17, 3, 1, 1);
    layout->addWidget(line8, 18, 0, 1, 4);

    QFrame *line9 = new QFrame;
    line5->setFrameShape(QFrame::HLine);

    layout->addWidget(pp1, 19, 0, 1, 1);
    layout->addWidget( p1, 19, 1, 1, 1);
    layout->addWidget( p5, 19, 3, 1, 1);
    layout->addWidget(line9, 20, 0, 1, 4);

    QFrame *line10 = new QFrame;
    line6->setFrameShape(QFrame::HLine);

    layout->addWidget(pp0, 21, 0, 1, 1);
    layout->addWidget( p0, 21, 1, 1, 1);
    layout->addWidget( p4, 21, 3, 1, 1);
    layout->addWidget(line10, 22, 0, 1, 4);

    this->setLayout(layout);
}

InfoRegion::~InfoRegion()
{
    delete layout;
}

void InfoRegion::set_angle(qreal a2, qreal a1, qreal a0)
{
    this->a2->setText(QString::number(a2, 'f', 2));
    this->a1->setText(QString::number(a1, 'f', 2));
    this->a0->setText(QString::number(a0, 'f', 2));
}

void InfoRegion::set_angle_can(qreal a2, qreal a1, qreal a0)
{
    this->a5->setText(QString("(%1)").arg(QString::number(a2, 'f', 2)));
    this->a4->setText(QString("(%1)").arg(QString::number(a1, 'f', 2)));
    this->a3->setText(QString("(%1)").arg(QString::number(a0, 'f', 2)));
}

void InfoRegion::set_position(QPointF pos3, QPointF pos2, QPointF pos1, QPointF pos0)
{
    this->p3->setText(QString("%1").arg(QString::number(pos3.x(), 'f', 2)));
    this->p7->setText(QString("%1").arg(QString::number(pos3.y(), 'f', 2)));

    this->p2->setText(QString("%1").arg(QString::number(pos2.x(), 'f', 2)));
    this->p6->setText(QString("%1").arg(QString::number(pos2.y(), 'f', 2)));

    this->p1->setText(QString("%1").arg(QString::number(pos1.x(), 'f', 2)));
    this->p5->setText(QString("%1").arg(QString::number(pos1.y(), 'f', 2)));

    this->p0->setText(QString("%1").arg(QString::number(pos0.x(), 'f', 2)));
    this->p4->setText(QString("%1").arg(QString::number(pos0.y(), 'f', 2)));
}

void InfoRegion::set_temperature(qreal t2_d, qreal t2_i, qreal t1_d, qreal t1_i,qreal t0_d, qreal t0_i)
{
    this->t2->setText(QString("%1").arg(QString::number(t2_d, 'f', 2)));
    this->t5->setText(QString("%1").arg(QString::number(t2_i, 'f', 2)));

    this->t1->setText(QString("%1").arg(QString::number(t1_d, 'f', 2)));
    this->t4->setText(QString("%1").arg(QString::number(t1_i, 'f', 2)));

    this->t0->setText(QString("%1").arg(QString::number(t0_d, 'f', 2)));
    this->t3->setText(QString("%1").arg(QString::number(t0_i, 'f', 2)));
}

InfoWidget::InfoWidget(QWidget *parent)
    : QWidget(parent)
{
    region0 = new RenderRegion();
    region1 = new InfoRegion();
    line = new QFrame();
    line->setFrameShape(QFrame::VLine);

    layout = new QHBoxLayout();
    layout->addWidget(region0);
    layout->addSpacing(3);
    layout->addWidget(line);
    layout->addWidget(region1);

    this->setLayout(layout);
}

InfoWidget::~InfoWidget()
{
    delete region0;
    delete region1;
    delete line;
    delete layout;
}

void InfoWidget::on_angle_changed(qreal a2, qreal a1, qreal a0)
{
    region1->set_angle(a2, a1, a0);
}

void InfoWidget::on_angle_can_changed(qreal a2, qreal a1, qreal a0)
{
    region1->set_angle_can(a2, a1, a0);
}

void InfoWidget::on_tempt_changed(qreal t2_d, qreal t2_i, qreal t1_d, qreal t1_i, qreal t0_d, qreal t0_i)
{
    region1->set_temperature(t2_d, t2_i, t1_d, t1_i, t0_d, t0_i);
}

void InfoWidget::on_position_changed(QPointF p3, QPointF p2, QPointF p1, QPointF p0)
{
    region0->set_position(p3, p2, p1, p0);
    region1->set_position(p3, p2, p1, p0);
}

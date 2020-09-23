#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QPen>
#include <QPainter>
#include <QGridLayout>

class MainWindow;

class RenderRegion : public QWidget
{
    Q_OBJECT

public:
    RenderRegion(QWidget *parent = nullptr);
    ~RenderRegion();

public slots:
    void set_position(QPointF &p3, QPointF &p2, QPointF &p1, QPointF &p0);

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QLine xaxis;
    QLine yaxis;

    QLineF line0;
    QLineF line1;

    QPen pen1;
    QPen pen2;

    QPointF _p3;
    QPointF _p2;
    QPointF _p1;
    QPointF _p0;
};

class InfoRegion : public QWidget
{
    Q_OBJECT

public:
    explicit InfoRegion(QWidget *parent = nullptr);
    ~InfoRegion();

    void set_angle(qreal a2, qreal a1, qreal a0);
    void set_angle_can(qreal a2, qreal a1, qreal a0);
    void set_position(QPointF pos3, QPointF pos2, QPointF pos1, QPointF pos0);
    void set_temperature(qreal t2_d, qreal t2_i, qreal t1_d, qreal t1_i,qreal t0_d, qreal t0_i);

private:
    QGridLayout *layout = nullptr;

    // angle
    QLabel *aa2 = nullptr;
    QLabel *aa1 = nullptr;
    QLabel *aa0 = nullptr;

    // device ideal angle
    QLabel *a2 = nullptr;
    QLabel *a1 = nullptr;
    QLabel *a0 = nullptr;

    // device actual angle
    QLabel *a5 = nullptr;
    QLabel *a4 = nullptr;
    QLabel *a3 = nullptr;

    // temperature
    QLabel *tt2 = nullptr;
    QLabel *tt1 = nullptr;
    QLabel *tt0 = nullptr;

    // device temperature
    QLabel *t2 = nullptr;
    QLabel *t1 = nullptr;
    QLabel *t0 = nullptr;

    // iverter temperature
    QLabel *t5 = nullptr;
    QLabel *t4 = nullptr;
    QLabel *t3 = nullptr;

    // position
    QLabel *pp3 = nullptr;
    QLabel *pp2 = nullptr;
    QLabel *pp1 = nullptr;
    QLabel *pp0 = nullptr;

    // device temperature
    QLabel *p3 = nullptr;
    QLabel *p2 = nullptr;
    QLabel *p1 = nullptr;
    QLabel *p0 = nullptr;

    // iverter temperature
    QLabel *p7 = nullptr;
    QLabel *p6 = nullptr;
    QLabel *p5 = nullptr;
    QLabel *p4 = nullptr;
};

class InfoWidget : public QWidget
{
    Q_OBJECT

friend class MainWindow;

public:
    explicit InfoWidget(QWidget *parent = nullptr);
    ~InfoWidget();

signals:

public slots:
    void on_angle_changed(qreal a2, qreal a1, qreal a0);
    void on_angle_can_changed(qreal a2, qreal a1, qreal a0);
    void on_tempt_changed(qreal t2_d, qreal t2_i, qreal t1_d, qreal t1_i,qreal t0_d, qreal t0_i);
    void on_position_changed(QPointF p3, QPointF p2, QPointF p1, QPointF p0);

private:
    QFrame *line = nullptr;
    QHBoxLayout *layout;
    RenderRegion *region0 = nullptr;
    InfoRegion *region1 = nullptr;
};

#endif // INFOWIDGET_H

#ifndef TitleWidget_H
#define TitleWidget_H

#include <QPixmap>
#include <QFrame>
#include <QQueue>

class QHBoxLayout;
class QLabel;

class TitleWidget : public QFrame
{
    Q_OBJECT

public:
    TitleWidget(QWidget *parent = nullptr);
    ~TitleWidget();

public slots:
    void on_hint(QString info);
    void on_net_status(bool f);
    void on_car_status(bool f);
    void on_arm_status(bool f);

protected:
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    int timer;
    bool empty = true;
    QLabel *hint;
    QLabel *date;
    QQueue<QString> queue;
    QHBoxLayout *layout;

    QPixmap red;
    QPixmap green;

    QLabel *netstats;
    QLabel *armstats;
    QLabel *carstats;
};

#endif // WIDGET_H

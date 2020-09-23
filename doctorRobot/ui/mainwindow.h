#ifndef MainWindow_H
#define MainWindow_H

#include "settingwidget.h"
#include "infowidget.h"
#include "titlewidget.h"
#include "sidewidget.h"
#include "homewidget.h"
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QThread>
#include <controller.h>


class MainWindow : public QFrame
{
    Q_OBJECT

friend class Controller;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void on_buttonClicked(int id);

protected:
    void paintEvent(QPaintEvent *event);

private:
    TitleWidget *title;
    SideWidget *sidebar;
    HomeWidget *home;
    InfoWidget *infol;
    InfoWidget *infor;
    SettingWidget *setting;

    QLine l1;
    QLine l2;

    int sc_width;
    int sc_height;

//    QThread *work;
    Controller *controller;
};

#endif // MainWindow_H

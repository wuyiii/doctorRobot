#ifndef SIDEWIDGET_H
#define SIDEWIDGET_H

#include <QFrame>
#include <QPushButton>
#include <QButtonGroup>

class MainWindow;
class QVBoxLayout;

class SideWidget : public QFrame
{
    Q_OBJECT

friend class MainWindow;

public:
    SideWidget(QWidget *parent = nullptr);
    ~SideWidget();

public slots:

protected:
    void resizeEvent(QResizeEvent *event);

private:
    QVBoxLayout *layout;
    QButtonGroup *group;
    QPushButton *home;
    QPushButton *infol;
    QPushButton *infor;
    QPushButton *setting;
    QPushButton *depth;  
};

#endif // SIDEWIDGET_H

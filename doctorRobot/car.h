#ifndef CAR_H
#define CAR_H

#include <QObject>
#include <QMutex>
#include <QtSerialPort/QSerialPort>

class Car : public QObject
{
    Q_OBJECT

public:
    explicit Car(QObject *parent = nullptr);
    ~Car();

    bool setup_uart(QString name);

    void start();
    bool ok(){return  status;}
    void send(const char *data);

    //xlevel ylevel
    void move_ahead(char l, char r);
    void move_left_ahead(char l, char r);
    void move_right_ahead(char l, char r);
    void move_back(char l, char r);
    void move_left_back(char l, char r);
    void move_right_back(char l, char r);
    void move_left(char l, char r);
    void move_right(char l, char r);
    void stop();

signals:

public slots:
    void on_serial_readyRead();

private:
    QSerialPort *serial = nullptr;
    QMutex mutex;

#define BUFNUM 100
    char *buf;
    int pos = 0;
    bool status = false;
};

#endif // CAR_H

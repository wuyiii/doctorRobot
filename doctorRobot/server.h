#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QMutex>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

class Server : public QObject
{
    Q_OBJECT

signals:
    void get_data(char *dat);

public:
    Server(QObject *parent = nullptr);
    ~Server();

    void start();
    void stop();
    bool ok(){return  status;}
    void send(char *dat, int len);

signals:
    void written();

public slots:
    void on_newConnection();
    void on_readyRead();
    void on_disconnected();
    void on_error(QAbstractSocket::SocketError socketError);

private:
    char data[10];
    bool status = false;
    QMutex mutex;
    QTcpServer *tcpserver = nullptr;
    QTcpSocket *dstsocket = nullptr;
};
#endif // SERVER_H

#include "server.h"
#include "controller.h"
#include <QtDebug>
#include <QDataStream>
#include <QIODevice>

Server::Server(QObject *parent)
    : QObject(parent)
{
    tcpserver = new QTcpServer(this);

    QObject::connect(tcpserver, &QTcpServer::newConnection, this, &Server::on_newConnection);
    QObject::connect(tcpserver, &QTcpServer::acceptError, this, &Server::on_error);
}

Server::~Server()
{
    tcpserver->close();
    delete tcpserver;
}

void Server::start()
{
    tcpserver->listen(QHostAddress(IP_ADDRESS), IP_TCP_PORT);
    if(tcpserver->isListening()){
        status = true;
        qDebug() << "server is listening";
    }
}

void Server::stop()
{
    if(dstsocket)
        dstsocket->close();
    tcpserver->close();
    dstsocket = nullptr;
    status = false;
}

void Server::send(char *dat, int len)
{
    qint64 ret = -1;

    if(!dstsocket || !dstsocket->isValid())
        return;

    mutex.lock();
    ret = dstsocket->write(dat, len);
    mutex.unlock();

    if(ret < 0){
        qDebug() << "server write error";
        dstsocket->close();
        return;
    }else if(ret == len)
        emit written();

    return;
}

void Server::on_newConnection()
{
    dstsocket = tcpserver->nextPendingConnection();
    if(dstsocket == nullptr)
        return;

    qDebug() << dstsocket->localAddress() << dstsocket->localPort() << "connected";
    QObject::connect(dstsocket, &QTcpSocket::readyRead, this, &Server::on_readyRead);
    QObject::connect(dstsocket, &QTcpSocket::disconnected, this, &Server::on_disconnected);
}

void Server::on_readyRead()
{
    qint64 ret;

    while(1){
        ret = dstsocket->read(data, 10);
        if(ret < 0)
        {
            qDebug() << "server read error";
            break;
        }else if(ret == 0){
            break;
        }else if(ret == 10){
            emit get_data(data);
        }else
            break;
    }
}

void Server::on_disconnected()
{
    qDebug() << dstsocket->localAddress() << "disconnected";
    dstsocket->close();
    dstsocket = nullptr;
}

void Server::on_error(QAbstractSocket::SocketError socketError)
{
    (void)socketError;
    qDebug() << tcpserver->errorString();
}

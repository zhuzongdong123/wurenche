#include "tcpserver.h"

TcpServer::TcpServer(QWidget *parent) :
    QObject(parent)
{

}

TcpServer::~TcpServer()
{
}

void TcpServer::startConnectClient(quint16 port)
{
    m_tcpserver = new QTcpServer(this);
    qDebug() << "监听的端口号:" << port;
    m_tcpserver->listen(QHostAddress::Any,port);
    connect(m_tcpserver,SIGNAL(newConnection()),this,SLOT(newConnect()));
}

void TcpServer::tcpsocketDisconnected()
{
    qDebug()<< "连接断开";
}
void TcpServer::newConnect()
{
    //后期如果需要反馈接收方已收到消息，还需要添加一个vector标记给谁反馈
    QTcpSocket *tcpsocket = m_tcpserver->nextPendingConnection();
    connect(tcpsocket,SIGNAL(readyRead()),this,SLOT(readMessage()));
    connect(tcpsocket,SIGNAL(disconnected()),this,SLOT(tcpsocketDisconnected()));
}
void TcpServer::readMessage()
{
    QTcpSocket *tcpsocket = qobject_cast<QTcpSocket*>(sender());
    QByteArray byte = tcpsocket->readAll();
    qDebug()<< "收到TCP连接,等待解析......" << byte;
}

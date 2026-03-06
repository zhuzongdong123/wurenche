#ifndef MYSERVER_H
#define MYSERVER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = nullptr);
    ~TcpServer();

signals:
    void sig_sendRcvMsg();

private slots:
    void newConnect();
    void readMessage();
    void startConnectClient(quint16 port = 8081);
    void tcpsocketDisconnected();

private:
    QTcpServer *m_tcpserver = nullptr;
};

#endif // WIDGET_H

#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QMutex>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient();
    ~TcpClient();

    void startConnect(const QString& ip, int port, QString serverName);
    void closeConnect();
    bool writeDataToServer(QByteArray byte);
    bool getStatus();

signals:
    void sig_sendReceiveMsg(QByteArray byte);
    void sig_connected();
    void sig_disConnected();

protected slots:
    void slt_onConnect();
    void slt_onDisConnect();
    void slt_onReadMsg();
    void slt_timeout();

private:
    QTcpSocket* m_TcpSocket = nullptr;
    QString 	m_strSocketIp;
    int 		m_nSockPort = 0;
    bool m_isOkConnected = false;
    QTimer m_timer;
    QString m_serverName;
};

#endif // TCPTHREAD_H


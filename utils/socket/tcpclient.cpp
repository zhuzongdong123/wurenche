#include "tcpclient.h"
#include "qtimer.h"
#include <QProcess>
#include <QTextCodec>
#include <QDateTime>
#include "appeventbase.h"

TcpClient::TcpClient()
{
    connect(&m_timer, &QTimer::timeout, this, &TcpClient::slt_timeout);
    m_timer.start(6000);
}

TcpClient::~TcpClient()
{
    closeConnect();
}

void TcpClient::slt_onDisConnect()
{
    //socket一旦断开则自动进入这个槽函数
    m_isOkConnected = false;
    qDebug()<<"socket is disconnect！"<<endl;
    emit sig_disConnected();
    emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("%1断开连接").arg(m_serverName));
}

void TcpClient::startConnect(const QString& ip, int port , QString serverName)
{
    m_serverName = serverName;
    emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("%1连接中").arg(m_serverName));
    if (!m_TcpSocket)
    {
        m_TcpSocket = new QTcpSocket(this);
        connect(m_TcpSocket, SIGNAL(readyRead()), this, SLOT(slt_onReadMsg()));
        connect(m_TcpSocket, SIGNAL(connected()), this, SLOT(slt_onConnect()));
        connect(m_TcpSocket, SIGNAL(disconnected()), this, SLOT(slt_onDisConnect()));
    }
    m_TcpSocket->abort();
    m_TcpSocket->disconnectFromHost();
    m_TcpSocket->connectToHost(ip, port);
    m_strSocketIp = ip;
    m_nSockPort = port;
    m_timer.start();
}

void TcpClient::closeConnect()
{
    if(nullptr != m_TcpSocket)
    {
        m_TcpSocket->abort();
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->deleteLater();
        m_TcpSocket = nullptr;
        m_strSocketIp = "";
        m_nSockPort = 0;
    }
    m_timer.stop();

    if(!m_isOkConnected || nullptr == m_TcpSocket)
        return;
    emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("%1断开连接").arg(m_serverName));
}

bool TcpClient::writeDataToServer(QByteArray byte)
{
    if(!m_isOkConnected || nullptr == m_TcpSocket)
        return false;

    int result = m_TcpSocket->write(byte);
    m_TcpSocket->flush();

    if(result < 0)
    {
        qDebug() << "data is send fail: " << byte;
        return false;
    }
    else
    {
        qDebug() << "data is send success: " << byte << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        return true;
    }
}

bool TcpClient::getStatus()
{
    return m_isOkConnected;
}

void TcpClient::slt_onConnect()
{
    //已连接
    qDebug()<<"socket is connected！"<<endl;
    emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("%1连接成功").arg(m_serverName));
    m_isOkConnected = true;
    emit sig_connected();
}

void TcpClient::slt_onReadMsg()
{
    if(m_TcpSocket->bytesAvailable() <= 0)
    {
        //  判定连接失败
        return;
        qDebug()<<"byte size is 0"<<endl;
    }

    QByteArray rcvData = m_TcpSocket->readAll();
    emit sig_sendReceiveMsg(rcvData);
    //qDebug() << "data is receive success: " << rcvData;
}

void TcpClient::slt_timeout()
{
    if(!m_strSocketIp.isEmpty() && !m_isOkConnected)
    {
        startConnect(m_strSocketIp,m_nSockPort,m_serverName);
    }
}

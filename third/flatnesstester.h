#ifndef FLATNESSTESTER_H
#define FLATNESSTESTER_H

#include <QWidget>
#include "tcpclient.h"
#include <QJsonObject>
#include <QJsonArray>

class FlatnessTester : public QWidget
{
    Q_OBJECT
public:
    explicit FlatnessTester(QWidget *parent = nullptr);
    ~FlatnessTester();
    void startConnect();
    void stopConnect();
    void startRecord(bool value);

signals:
    //void sig_sendGNSSInfo(QJsonObject obj);
    void sig_sendValue(double key,double value);

public slots:
    void slt_setParamsInfo(QJsonObject obj);
    void slt_setReceiveMsg(QString data);
    void slt_tcpConnected();

private slots:
    void slt_saveDataToServer();

private:
    TcpClient m_tcpClient;
    QString m_tcpInfo;
    QString m_rcvData;
    bool m_isRecord = false;
    bool m_isStart = false;
    bool m_isStatus = true;
    QJsonArray m_arrayGNSS;
    QJsonArray m_arrayRIR;
    bool checkAppRunningStatus(const QString &appName);
};

#endif // FLATNESSTESTER_H

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QtSerialPort/QSerialPort>         // 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>      // 提供系统中存在的串口信息

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = nullptr);
    QByteArray getRcvData();

public:
    //获取串口列表
    QList<QSerialPortInfo> getSerialportList();

    //打开串口
    bool openSerialport(QString portName);
    //关闭串口
    void closeSerialport();

    bool getStatus();

public slots:
    void slt_sendData(QString msg);//发送串口数据
    void slt_rcvData();//接收串口数据
    void slt_errorOccurred(QSerialPort::SerialPortError error);

signals:
    void sig_sendRcvData(QString data);

private:
    QSerialPort* m_serialport = nullptr;
    QByteArray m_rcvData = nullptr;
};

#endif // SERIALPORT_H

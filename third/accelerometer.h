#ifndef Accelerometer_H
#define Accelerometer_H

#include <QWidget>
#include "tcpclient.h"
#include <QJsonObject>
#include <QJsonArray>

class Accelerometer : public QWidget
{
    Q_OBJECT
public:
    explicit Accelerometer(QWidget *parent = nullptr);
    ~Accelerometer();
    void startConnect();
    void stopConnect();
    void startRecord(bool value);

signals:
    void sig_sendGNSSInfo(QJsonObject obj);
    void sig_sendValue(QMap<QString,double> mapValue);

public slots:
    void slt_setReceiveMsg(QByteArray data);
    void slt_tcpConnected();

private slots:
    void slt_saveDataToServer();

private:
    TcpClient m_tcpClient;
    QString m_tcpInfo;
    QVector<double> m_iriInfo;
    QByteArray m_rcvData;
    QJsonArray m_array;
    bool m_isStartRecord = false;

private:
    void pariseData();
    int accelerometerSpeed = 20;

    struct AccelerometerData
    {
        unsigned char header[4];//头
        unsigned char data[900];//头
        unsigned char gps[14];
        unsigned char endding[4];//尾
    };
    struct LineData
    {
        unsigned char x[3];
        unsigned char y[3];
        unsigned char z[3];
    };
};

#endif // Accelerometer_H

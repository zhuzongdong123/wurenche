#ifndef WIDGETMAPPAGE_H
#define WIDGETMAPPAGE_H

#include <QWidget>
#include "serialport.h"
#include <QJsonObject>
#include <QJsonArray>
#include "httpclient.h"

namespace Ui {
class WidgetMapPage;
}

class WidgetMapPage : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMapPage(QWidget *parent = nullptr);
    ~WidgetMapPage();

    void loadMapUrl(QString urlPath, QString gpsPort);

    void closeGPSRcv();

    //GPS的脉冲重置
    void resetGPS();

    //开始记录到数据库
    void startRecord(bool value);

signals:
    void sig_sendGPSData(QJsonObject obj);

private slots:
    void slt_rcvData(QString data);
    void slt_saveDataToServer();

private:
    Ui::WidgetMapPage *ui;
    QJsonArray m_arrayGPFPD;
    QJsonArray m_arrayGPPOSDMI;
    QJsonArray m_arrayGPGGA;
    QJsonArray m_arrayGPRMC;
    SerialPort m_serialPort;//串口监听工具
    HttpClient m_httpClient;//后端请求交互类
    QString m_rcvData;
    int m_isReset = -1;
    bool m_isStartRecord = false;
    bool m_status = true;

    //3个GPFPD数据
    enum GPFPD_TYPE
    {
        GPFPDDMI = 0,
        GPFPDDMI1,
        GPFPDDMI2,
        GPFPDDMI3,
        GPFPDDMI4
    };

    //GPPOSDMI数据
    enum GPPOSDMI_TYPE
    {
        GPPOSDMI = 0,
        GPPOSDMI1,
        GPPOSDMI2,
        GPPOSDMI3,
        GPPOSDMI4
    };

private:
    bool handleGpsData();

    //处理4种不同协议的数据
    void handleGPFPD(QString data);
    void handleGPPOSDMI(QString data);
    void handleGPGGA(QString data);
    void handleGPRMC(QString data);
};

#endif // WIDGETMAPPAGE_H

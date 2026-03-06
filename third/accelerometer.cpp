#include "accelerometer.h"
#include "accelerometer.h"
#include "appconfigbase.h"
#include <QStyleOption>
#include <QPainter>
#include <QDateTime>
#include "appdatabasebase.h"
#include "appcommonbase.h"
#include "mychart.h"
#include "appeventbase.h"

Accelerometer::Accelerometer(QWidget *parent) : QWidget(parent)
{
    connect(&m_tcpClient, &TcpClient::sig_sendReceiveMsg, this, &Accelerometer::slt_setReceiveMsg, Qt::DirectConnection);
    connect(&m_tcpClient, &TcpClient::sig_connected, this, &Accelerometer::slt_tcpConnected);
    connect(&m_tcpClient,&TcpClient::sig_disConnected,this,[=](){
        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::accelerometer,false);
    });
    QString temp = AppConfigBase::getInstance()->readConfigSettings("Setting","AccelerometerSpeed","20");
    accelerometerSpeed = temp.toInt();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Accelerometer::slt_saveDataToServer);
    timer->start(4000);
}

Accelerometer::~Accelerometer()
{
    m_tcpClient.writeDataToServer("<<<<STOP>>>>");
}

void Accelerometer::startConnect()
{
    if(m_tcpClient.getStatus())
        return;

    //获取平整度仪的ip和端口号
    QString ip = AppConfigBase::getInstance()->readConfigSettings("Accelerometer","hostName","192.168.1.20");
    QString port = AppConfigBase::getInstance()->readConfigSettings("Accelerometer","port","5000");

//    ip = "127.0.0.1";
//    port = "7000";

    m_tcpClient.writeDataToServer("<<<<STOP>>>>");
    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::accelerometer,false);
    m_tcpClient.startConnect(ip,port.toInt(),"加速度计");
}

void Accelerometer::stopConnect()
{
    m_tcpClient.closeConnect();
}

void Accelerometer::startRecord(bool value)
{
    m_isStartRecord = value;
}

void Accelerometer::slt_setReceiveMsg(QByteArray data)
{
    m_rcvData += data;
    //完整的一包数据是922个字节
    pariseData();
}

void Accelerometer::slt_tcpConnected()
{
    m_tcpClient.writeDataToServer("<<<<START>>>>");
    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::accelerometer,true);
}

void Accelerometer::pariseData()
{
    //完整的一包数据
    while(m_rcvData.size() >= 922)
    {
        QByteArray currentData = m_rcvData.mid(0,922);
        m_rcvData = m_rcvData.right(currentData.size()-922);

        AccelerometerData *struct_data = (AccelerometerData*)currentData.data();
        if(nullptr == struct_data)
            return;

        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
        static int num = 0;
        LineData *ld = nullptr;
        for(int count = 0; count < 100; count++)
        {
            ld = (LineData*)(struct_data->data + count * 9);
            if(nullptr != ld)
            {
                double xResult = int((ld->x[0])*0x1000000 + (ld->x[1])*0x10000 + (ld->x[2])*0x100) / (0x40000000 * 1.0);
                double yResult = int((ld->y[0])*0x1000000 + (ld->y[1])*0x10000 + (ld->y[2])*0x100) / (0x40000000 * 1.0);
                double zResult = int((ld->z[0])*0x1000000 + (ld->z[1])*0x10000 + (ld->z[2])*0x100) / (0x40000000 * 1.0);

                num++;
//                if(zResult < 0.8)
//                    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "当前数据(send): " << zResult << num;

                //向外抛当前的数据
                QMap<QString,double> mapValue;
                mapValue.insert("加速度计-X",xResult);
                mapValue.insert("加速度计-Y",yResult);
                mapValue.insert("加速度计-Z",zResult);

                QJsonObject obj;
                obj.insert("xValue",xResult);
                obj.insert("yValue",yResult);
                obj.insert("zValue",zResult);
                obj.insert("pulse_value",gpsObj.value("pulse_value").toInt());
                obj.insert("createTime",time);
                obj.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
                if(m_isStartRecord && AppCommonBase::getInstance()->bIsGPSReset)
                {
                    m_array.push_back(obj);
                }


                if(count%accelerometerSpeed != 0)
                {
                    continue;
                }
                AppDatabaseBase::getInstance()->g_chart->addValues(xResult,yResult,zResult);
            }
        }
    }
}

void Accelerometer::slt_saveDataToServer()
{
    if(m_array.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_array);
        QJsonArray array;
        m_array.swap(array);
        AppEventBase::getInstance()->sig_requestServer(SAVE_ACCELEROMETER,obj);
    }
}

#include "widgetmappage.h"
#include "ui_widgetmappage.h"
#include <QUuid>
#include "appcommonbase.h"
#include "appconfigbase.h"
#include "appeventbase.h"

WidgetMapPage::WidgetMapPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMapPage)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WidgetMapPage::slt_saveDataToServer);
    timer->start(3000);
}

WidgetMapPage::~WidgetMapPage()
{
    delete ui;
}

void WidgetMapPage::loadMapUrl(QString urlPath, QString gpsPort)
{
    if(m_serialPort.getStatus())
        return;

    //加载web页面
    ui->webView->load(QUrl(urlPath));

    //开启串口监听
    bool openReuslt = m_serialPort.openSerialport(gpsPort);
    if(openReuslt)
    {
        resetGPS();//重置脉冲
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("GPS连接成功"));
        ui->stackedWidget->setCurrentIndex(0);
        disconnect(&m_serialPort, &SerialPort::sig_sendRcvData, this, &WidgetMapPage::slt_rcvData);
        connect(&m_serialPort, &SerialPort::sig_sendRcvData, this, &WidgetMapPage::slt_rcvData,Qt::DirectConnection);
        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::GPS,true);
    }
    else
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("GPS连接失败,串口号:%1").arg(gpsPort));
        //打开失败，界面上出现提示
        ui->msgTip->setText(QString("%1打开失败").arg(gpsPort));
        ui->stackedWidget->setCurrentIndex(1);
        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::GPS,false);
    }

//    //测试代码
//    QTimer *timer = new QTimer();
//    connect(timer,&QTimer::timeout,this,[=](){
//        QString temp = "12$GPFPDDMI,2315,2049,1,2,3,37.15623,117.52635,12,1.1,2.1,3.1,4.1,9.1,10.1,11.1,4C,88886666*4C\r\n\r\n$GTIMU,GPSWeek,GPSTime,GyroX,GyroY,GyroZ,AccX,AccY,AccZ,Tpr*cs\r\n$GPGGA,hhmmss.ss,Latitude,N,Longitude,E,FS,NoSV,HDOP,msl,m,Altref,m,DiffA ge,DiffStation*cs\r\n$GPRMC,hhmmss,status,latitude,N,longitude,E,spd,cog,ddmmyy,mv,mvE,mode*cs\r\n";
//        slt_rcvData(temp);
//    });
    //timer->start(300);
}

void WidgetMapPage::closeGPSRcv()
{
    disconnect(&m_serialPort, &SerialPort::sig_sendRcvData, this, &WidgetMapPage::slt_rcvData);
    m_serialPort.closeSerialport();
}

void WidgetMapPage::resetGPS()
{
    qDebug() << "开始重置gps start" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    m_isReset = 0;
    m_serialPort.slt_sendData("$cmd,set,gpfpddmi,null*ff");
    m_rcvData.clear();
    qDebug() << "开始重置gps end" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void WidgetMapPage::startRecord(bool value)
{
    m_isReset = 0;
    m_isStartRecord = value;
}

void WidgetMapPage::slt_rcvData(QString data)
{
    SerialPort* serilPort = dynamic_cast<SerialPort*>(sender());
    if(nullptr != serilPort)
    {
        m_rcvData += serilPort->getRcvData();
        //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") << "收到数据" << data;
        while (0 != m_rcvData.size()) {
            if(!handleGpsData())
                break;
        }
        //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") << "处理完数据";
    }
}

void WidgetMapPage::slt_saveDataToServer()
{
    if(m_arrayGPFPD.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayGPFPD);
        AppEventBase::getInstance()->sig_requestServer(SAVE_GPS_INFO_GPFPD,obj);

        QJsonArray array;
        m_arrayGPFPD.swap(array);
    }

    if(m_arrayGPPOSDMI.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayGPPOSDMI);
        AppEventBase::getInstance()->sig_requestServer(SAVE_GPS_INFO_GPPOSTMI,obj);

        QJsonArray array;
        m_arrayGPPOSDMI.swap(array);
    }

    if(m_arrayGPGGA.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayGPGGA);
        AppEventBase::getInstance()->sig_requestServer(SAVE_GPS_INFO_GPGGA,obj);

        QJsonArray array;
        m_arrayGPGGA.swap(array);
    }

    if(m_arrayGPRMC.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayGPRMC);
        AppEventBase::getInstance()->sig_requestServer(SAVE_GPS_INFO_GPRMC,obj);

        QJsonArray array;
        m_arrayGPRMC.swap(array);
    }
}

bool WidgetMapPage::handleGpsData()
{
    int startIndex = -1;
    int endIndex = -1;

    bool isHandle = false;

    if(!m_rcvData.startsWith("$") && -1 != m_rcvData.indexOf("$"))
    {
        m_rcvData = m_rcvData.mid(m_rcvData.indexOf("$"));
    }


    startIndex = m_rcvData.indexOf("$cmd");
    endIndex = m_rcvData.indexOf("\r\n",startIndex);
    if(-1 != startIndex && -1!= endIndex && endIndex > startIndex)
    {
        QString data = m_rcvData.mid(startIndex,endIndex-startIndex+2);
        if(m_isStartRecord && 0 == m_isReset && data.contains("ok"))
        {
            emit AppEventBase::getInstance()->sig_mapResetSuccessed();
            m_rcvData = m_rcvData.mid(endIndex+2);
            AppCommonBase::getInstance()->bIsGPSReset = true;//脉冲重置成功
            m_isReset = 1;
            qDebug() << "脉冲重置成功" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        }
        else
        {
            m_rcvData = m_rcvData.remove(startIndex, endIndex-startIndex+2);
        }
    }

    startIndex = m_rcvData.indexOf("$");
    endIndex = m_rcvData.indexOf("\r\n",startIndex);
    if(-1 != startIndex && -1!= endIndex && endIndex > startIndex)
    {
        QString data = m_rcvData.mid(startIndex,endIndex-startIndex+2);
        m_rcvData = m_rcvData.remove(startIndex, endIndex-startIndex+2);

        if(data.startsWith("$GPFPD"))
        {
            handleGPFPD(data);
        }
        else if(data.startsWith("$GPPOSDMI"))
        {
            handleGPPOSDMI(data);
        }
        else if(data.startsWith("$GPGGA"))
        {
            handleGPGGA(data);
        }
        else if(data.startsWith("$GPRMC"))
        {
            handleGPRMC(data);
        }

        isHandle = true;
    }

    return isHandle;
}

void WidgetMapPage::handleGPFPD(QString data)
{
    QStringList list = data.split(",");
    if(list.size() < 18)//包不完整
    {
        return;
    }

    double lon = list[7].toDouble();
    double lat = list[6].toDouble();
    double altitude = list[8].toDouble();
    QString uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");

    //脉冲总数
    QString temp = list[17];
    int pulsesTotalNum = temp.split("*")[0].toInt();

    //类型
    GPFPD_TYPE type = GPFPD_TYPE::GPFPDDMI;
    if(list[0].contains("GPFPDDMI1"))
    {
        type = GPFPD_TYPE::GPFPDDMI1;
    }
    else if(list[0].contains("GPFPDDMI2"))
    {
        type = GPFPD_TYPE::GPFPDDMI2;
    }
    else if(list[0].contains("GPFPDDMI3"))
    {
        type = GPFPD_TYPE::GPFPDDMI3;
    }
    else if(list[0].contains("GPFPDDMI4"))
    {
        type = GPFPD_TYPE::GPFPDDMI4;
    }

    //先暂存起来，统一保存到数据库
    QJsonObject gpsObj;
    gpsObj.insert("type",int(type));//类型
    gpsObj.insert("id",uuid);
    gpsObj.insert("GPSWeek",list[1].toDouble());//周
    gpsObj.insert("GPSTime",list[2].toDouble());//秒
    gpsObj.insert("heading",list[3].toDouble());//偏航角
    gpsObj.insert("pitch",list[4].toDouble());//俯仰角
    gpsObj.insert("roll",list[5].toDouble());//横滚角
    gpsObj.insert("lon",lon);
    gpsObj.insert("lat",lat);
    gpsObj.insert("altitude",altitude);
    gpsObj.insert("ve",list[9].toDouble());//东向速度，单位（米/秒）
    gpsObj.insert("vn",list[10].toDouble());//北向速度，单位（米/秒）
    gpsObj.insert("vu",list[11].toDouble());//天向速度，单位（米/秒）
    gpsObj.insert("speed",list[12].toDouble());//速度，单位（米/秒）
    gpsObj.insert("baseLine",list[13].toDouble());//基线长度
    gpsObj.insert("NSV1",list[14].toDouble());//天线 1 卫星数
    gpsObj.insert("NSV2",list[15].toDouble());//天线 2 卫星数
    gpsObj.insert("Status",list[16]);//状态位
    gpsObj.insert("pulse_value",pulsesTotalNum);//脉冲值
    if(2 == temp.split("*").size())
        gpsObj.insert("Cs","*" + temp.replace("\r\n","").split("*")[1]);//检验位
    gpsObj.insert("createTime",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    gpsObj.insert("GPS_time",AppCommonBase::getInstance()->getLastGPSPoint().value("GPS_time").toString());//当前gps时间

    if(m_isStartRecord && AppCommonBase::getInstance()->bIsGPSReset)
    {
        m_arrayGPFPD.push_back(gpsObj);
    }

    //将解析后的结果向外抛送
    emit sig_sendGPSData(gpsObj);
}

void WidgetMapPage::handleGPPOSDMI(QString data)
{
   // qDebug() << data;
    QStringList list = data.split(",");
    if(list.size() < 24)//包不完整
    {
        return;
    }

    //脉冲总数
    QString temp = list[23];
    int pulsesTotalNum = temp.split("*")[0].toInt();

    //测试代码
    static int GPPOSDMI0_count = 0;
    static int GPPOSDMI1_count = 0;
    static int GPPOSDMI2_count = 0;

    GPPOSDMI_TYPE type = GPPOSDMI_TYPE::GPPOSDMI;
    if(list[0].contains("GPPOSDMI1"))
    {
        type = GPPOSDMI_TYPE::GPPOSDMI1;
    }
    else if(list[0].contains("GPPOSDMI2"))
    {
        type = GPPOSDMI_TYPE::GPPOSDMI2;
    }
    else if(list[0].contains("GPPOSDMI3"))
    {
        type = GPPOSDMI_TYPE::GPPOSDMI3;
    }
    else if(list[0].contains("GPPOSDMI4"))
    {
        type = GPPOSDMI_TYPE::GPPOSDMI4;
    }

    //先暂存起来，统一保存到数据库
    QJsonObject gpsObj;
    gpsObj.insert("id",QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
    gpsObj.insert("type",type);//类型
    gpsObj.insert("GPSWeek",list[1].toDouble());//周
    gpsObj.insert("GPSTime",list[2].toDouble());//秒
    gpsObj.insert("heading",list[3].toDouble());//偏航角
    gpsObj.insert("pitch",list[4].toDouble());//俯仰角
    gpsObj.insert("roll",list[5].toDouble());//横滚角
    gpsObj.insert("lat",list[6].toDouble());
    gpsObj.insert("lon",list[7].toDouble());
    gpsObj.insert("altitude",list[8].toDouble());
    gpsObj.insert("GyroX",list[9].toDouble());
    gpsObj.insert("GyroY",list[10].toDouble());
    gpsObj.insert("GyroZ",list[11].toDouble());
    gpsObj.insert("AccX",list[12].toDouble());
    gpsObj.insert("AccY",list[13].toDouble());
    gpsObj.insert("AccZ",list[14].toDouble());
    gpsObj.insert("ve",list[15].toDouble());//东向速度，单位（米/秒）
    gpsObj.insert("vn",list[16].toDouble());//北向速度，单位（米/秒）
    gpsObj.insert("vu",list[17].toDouble());//天向速度，单位（米/秒）
    gpsObj.insert("speed",list[18].toDouble());//速度，单位（米/秒）
    gpsObj.insert("baseLine",list[19].toDouble());//基线长度
    gpsObj.insert("NSV1",list[20].toDouble());//天线 1 卫星数
    gpsObj.insert("NSV2",list[21].toDouble());//天线 2 卫星数
    gpsObj.insert("Status",list[22]);//状态位
    gpsObj.insert("pulse_value",pulsesTotalNum);
    if(2 == temp.split("*").size())
        gpsObj.insert("Cs","*" + temp.replace("\r\n","").split("*")[1]);//检验位
    gpsObj.insert("createTime",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    gpsObj.insert("distance",0.00025*pulsesTotalNum/1000);//当前里程，单位k米

    //获取gps的时间并计算
    QDateTime oldTime = QDateTime::fromString("1980-01-06","yyyy-MM-dd");
    oldTime = oldTime.addSecs(8*3600);
    oldTime = oldTime.addDays(list[1].toDouble()*7);
    QDateTime newTime = oldTime.addSecs(list[2].toDouble());
    gpsObj.insert("GPS_time",newTime.toString("yyyy-MM-dd hh:mm:ss"));//秒

    //界面上显示当前的结果，web页面定位当前点
    if(type == GPPOSDMI_TYPE::GPPOSDMI1)
    {
        AppCommonBase::getInstance()->setLastGPSPoint(gpsObj);
    }

    //地图渲染来自于GPPOSDMI2
    if(type == GPPOSDMI_TYPE::GPPOSDMI1)
    {
        QString jsCode = QString("resetLonLat('%1','%2')").
                arg(QString::number(list[7].toDouble(),'f',7)).
                arg(QString::number(list[6].toDouble(),'f',7));
        ui->webView->page()->runJavaScript(jsCode, [](const QVariant &v) { /*qDebug() << v.toString();*/ });
    }

    if(m_isStartRecord && AppCommonBase::getInstance()->bIsGPSReset)
    {
        if(GPPOSDMI_TYPE::GPPOSDMI1 == type)
        {
            GPPOSDMI0_count++;
           // qDebug() << "GPPOSDMI0_count" << GPPOSDMI0_count;

            //检验状态
            if("4B" != list[22])
            {
                if(m_status)
                {
                    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::GPS,false);
                }
                m_status = false;
            }
            else
            {
                if(!m_status)
                {
                    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::GPS,true);
                }
                m_status = true;
            }
        }
        if(GPPOSDMI_TYPE::GPPOSDMI1 == type)
        {
            AppCommonBase::getInstance()->m_landscapeCameraList.push_back(pulsesTotalNum);
        }
        if(GPPOSDMI_TYPE::GPPOSDMI2 == type)
        {
            AppCommonBase::getInstance()->m_rodeCameraList.push_back(pulsesTotalNum);
        }
        m_arrayGPPOSDMI.push_back(gpsObj);
    }
}

void WidgetMapPage::handleGPGGA(QString data)
{
    QStringList list = data.split(",");
    if(list.size() < 10)//包不完整
    {
        return;
    }

    //先暂存起来，统一保存到数据库
    QJsonObject gpsObj;
    gpsObj.insert("id",QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
    gpsObj.insert("utc",list[1]);
    gpsObj.insert("Latitude",list[2]);
    gpsObj.insert("N",list[3]);
    gpsObj.insert("Longitude",list[4]);
    gpsObj.insert("E",list[5]);
    gpsObj.insert("FS",list[6]);
    gpsObj.insert("NoSV",list[7]);
    gpsObj.insert("HDOP",list[8]);
    gpsObj.insert("msl",list[9]);
    gpsObj.insert("msl_unit",list[10]);
    gpsObj.insert("Altref",list[11]);
    gpsObj.insert("Altref_unit",list[12]);
    gpsObj.insert("DiffAge",list[13]);
    if(!list[14].isEmpty())
        gpsObj.insert("DiffStation",list[14].split("*")[0]);
    QJsonObject gpsObjTemp = AppCommonBase::getInstance()->getLastGPSPoint();
    int pulse_value = gpsObjTemp.value("pulse_value").toInt();
    gpsObj.insert("pulse_value",QString::number(pulse_value));
    gpsObj.insert("createTime",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    gpsObj.insert("GPS_time",AppCommonBase::getInstance()->getLastGPSPoint().value("GPS_time").toString());//当前gps时间
    if(m_isStartRecord && AppCommonBase::getInstance()->bIsGPSReset)
    {
       m_arrayGPGGA.push_back(gpsObj);
    }
}

void WidgetMapPage::handleGPRMC(QString data)
{
    QStringList list = data.split(",");
    if(list.size() < 13)//包不完整
    {
        return;
    }

    //先暂存起来，统一保存到数据库
    QJsonObject gpsObj;
    gpsObj.insert("id",QUuid::createUuid().toString().remove("{").remove("}").remove("-"));
    gpsObj.insert("utc",list[1]);
    gpsObj.insert("status",list[2]);
    gpsObj.insert("latitude",list[3]);
    gpsObj.insert("N",list[4]);
    gpsObj.insert("longitude",list[5]);
    gpsObj.insert("E",list[6]);
    gpsObj.insert("Spd",list[7]);
    gpsObj.insert("cog",list[8]);
    gpsObj.insert("ddmmyy",list[9]);
    gpsObj.insert("mv",list[10]);
    gpsObj.insert("mvE",list[11]);
    if(!list[12].isEmpty())
        gpsObj.insert("mode",list[12].split("*")[0]);
    QJsonObject gpsObjTemp = AppCommonBase::getInstance()->getLastGPSPoint();
    int pulse_value = gpsObjTemp.value("pulse_value").toInt();
    gpsObj.insert("pulse_value",QString::number(pulse_value));
    gpsObj.insert("createTime",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    gpsObj.insert("GPS_time",AppCommonBase::getInstance()->getLastGPSPoint().value("GPS_time").toString());//当前gps时间

    if(m_isStartRecord && AppCommonBase::getInstance()->bIsGPSReset)
    {
       m_arrayGPRMC.push_back(gpsObj);
    }
}

#include "flatnesstester.h"
#include "appconfigbase.h"
#include <QStyleOption>
#include <QPainter>
#include <QProcess>
#include "appeventbase.h"
#include "appcommonbase.h"
#include <QApplication>

FlatnessTester::FlatnessTester(QWidget *parent) : QWidget(parent)
{
    connect(&m_tcpClient, &TcpClient::sig_sendReceiveMsg, this, &FlatnessTester::slt_setReceiveMsg, Qt::DirectConnection);
    connect(&m_tcpClient, &TcpClient::sig_connected, this, &FlatnessTester::slt_tcpConnected);
    //connect(&m_tcpClient, &TcpClient::sig_disConnected, this, &FlatnessTester::startConnect);
    connect(&m_tcpClient,&TcpClient::sig_disConnected,this,[=](){
        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::flatnessTester,false);
        checkAppRunningStatus("MRS.exe");
    });

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &FlatnessTester::slt_saveDataToServer);
    timer->start(3000);

    //平整度仪的三方程序没有启动的时候，调用
    checkAppRunningStatus("MRS.exe");
}

FlatnessTester::~FlatnessTester()
{
    m_tcpClient.writeDataToServer("#FFAASTOP#FFBB");
    //emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "#FFAASTOP#FFBB");
}

void FlatnessTester::startConnect()
{
    if(m_tcpClient.getStatus())
        return;

    //平整度仪的三方程序没有启动的时候，调用
    checkAppRunningStatus("MRS.exe");

    //获取平整度仪的ip和端口号
    QString ip = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","hostName","127.0.0.1");
    QString port = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","port","8080");
    m_tcpClient.writeDataToServer("#FFAASTOP#FFBB");
    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::flatnessTester,false);
    m_tcpClient.startConnect(ip,port.toInt(),"平整度仪");
}

void FlatnessTester::stopConnect()
{
    QString timeout = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","timeout","6000");
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);//定时器，单次触发
    connect(timer,&QTimer::timeout,this,[=](){
        m_isRecord = false;
        m_tcpClient.writeDataToServer("#FFAASTOP#FFBB");
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "#FFAASTOP#FFBB");
        m_tcpClient.closeConnect();
    });
    timer->start(timeout.toInt());
}

void FlatnessTester::startRecord(bool value)
{
    m_isRecord = value;

    if(m_isRecord)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "#FFAASTART#FFBB");
        m_tcpClient.writeDataToServer("#FFAASTART#FFBB");
    }
}

void FlatnessTester::slt_setParamsInfo(QJsonObject obj)
{
    QString range = obj.value("iriRange").toString();
    QString divisionRatio1 = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","divisionRatio1","2");
    QString divisionRatio2 = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","divisionRatio2","2");
    //车载控制软件开始采集前发布该消息，表示发布激光路面平整度仪的项目信息。
    QString projectName = obj.value("roadName").toString() + "-" + obj.value("lane").toString();
    projectName.replace(" ","");
    double StartStation = QString::number(obj.value("pileNum_km").toString().toInt() + obj.value("pileNum_m").toString().toDouble()/1000,'f',3).toDouble();
    QString Direction = "+";
    if("下行" == obj.value("upAndDown").toString())
    {
        Direction = "-";
    }
    QString path = obj.value("savePath").toString();
    m_tcpInfo = "#FFAA" + QString("ProjectName:%1,StartStation:%2,Direction:%3,IRIInterval:%4,Path:%5,%6,%7").
            arg(projectName).arg(StartStation).arg(Direction).arg(range).arg(path).arg(divisionRatio1).arg(divisionRatio2) + "#FFBB";
    qDebug() << "连接成功发送的信息：" << m_tcpInfo;
}

void FlatnessTester::slt_setReceiveMsg(QString data)
{
    m_rcvData += data;
    while (0 != m_rcvData.size()) {

        int startIndex = m_rcvData.indexOf("#FFAA");
        int endIndex = m_rcvData.indexOf("#FFBB");

        if(startIndex < startIndex)
        {
            m_rcvData = m_rcvData.mid(startIndex);
            emit AppEventBase::getInstance()->sig_sendServerMsg(
                        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("数据包解析异常"));
            continue;
        }

        if(-1 != startIndex && -1!= endIndex)
        {
            QString dataValue = m_rcvData.mid(startIndex,endIndex-startIndex+5);
            m_rcvData = m_rcvData.mid(endIndex+5);
            dataValue = dataValue.mid(5,dataValue.length()-10);
            QStringList list = dataValue.split(",");
            if(list.size() > 3 && "DATA_IRI" == list[0])
            {
                double key = list[2].toDouble();
                double iri = list[3].toDouble();

                //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") << "DATA_IRI----------------------" << key << iri;
                emit sig_sendValue(key,iri);

                if(m_isRecord && AppCommonBase::getInstance()->bIsGPSReset)
                {
                    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                    QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
                    QJsonObject obj;
                    obj.insert("start_num",list[1].toDouble());
                    obj.insert("start_num",list[2].toDouble());
                    obj.insert("iri",iri);
                    obj.insert("pulse_value",gpsObj.value("pulse_value").toInt());
                    obj.insert("createTime",time);
                    obj.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
                    m_arrayRIR.push_back(obj);
                }
            }
            else if(list.size() > 4 && "DATA_SSG" == list[0])
            {
                double distance = list[1].toDouble();
                double speed = list[2].toDouble();
                double longitude = list[4].toDouble();
                double lattitude = list[3].toDouble();
                //qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") << "DATA_SSG " << distance << speed << longitude << lattitude;

                if(m_isRecord && AppCommonBase::getInstance()->bIsGPSReset)
                {
                    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                    QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
                    QJsonObject obj;
                    obj.insert("distance",distance);
                    obj.insert("speed",speed);
                    obj.insert("longitude",longitude);
                    obj.insert("lattitude",lattitude);
                    obj.insert("pulse_value",gpsObj.value("pulse_value").toInt());
                    obj.insert("createTime",time);
                    obj.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
                    m_arrayGNSS.push_back(obj);
                }
            }
            else if(list.size() == 2 && "STATUS" == list[0])
            {
                bool status = list[1].toInt();

                if(m_isStatus != status)
                {
                    //判断连接状态
                    if(!status)
                    {
                        qDebug() << "状态异常数据=========" << dataValue;
                        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::flatnessTester,false);
                        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("平整度仪状态异常"));
                    }
                    else
                    {
                        qDebug() << "状态恢复正常=========" << dataValue;
                        emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::flatnessTester,true);
                        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("平整度仪状态正常"));
                    }
                }
                m_isStatus = status;
            }
        }
    }
}

void FlatnessTester::slt_tcpConnected()
{
    QTimer::singleShot(1000, [=]() {
        QString msg = m_tcpInfo;
        m_tcpClient.writeDataToServer(msg.toUtf8());
        //emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + msg);

        if(m_isRecord)
        {
            m_tcpClient.writeDataToServer("#FFAASTART#FFBB");
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "#FFAASTART#FFBB");
        }
    });
    emit AppEventBase::getInstance()->sig_sendDevStatus(DEV_TYPE::flatnessTester,true);
}

void FlatnessTester::slt_saveDataToServer()
{
    if(m_arrayGNSS.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayGNSS);
        QJsonArray array;
        m_arrayGNSS.swap(array);
        AppEventBase::getInstance()->sig_requestServer(SAVE_DATALOCAITON,obj);
    }

    if(m_arrayRIR.size() != 0)
    {
        QJsonObject obj;
        obj.insert("data",m_arrayRIR);
        QJsonArray array;
        m_arrayRIR.swap(array);
        AppEventBase::getInstance()->sig_requestServer(SAVE_DATAIRI,obj);
    }
}

bool FlatnessTester::checkAppRunningStatus(const QString &appName)
{
#ifdef Q_OS_WIN
    QProcess* process = new QProcess;
    process->start("tasklist" ,QStringList());
    process->waitForFinished();
    QString outputStr = QString::fromLocal8Bit(process->readAllStandardOutput());
    if(outputStr.contains(appName)){
        return true;
    }
    else{
        QString exePath = QApplication::applicationDirPath() + "./MRS/MRS.exe";;
        if(!AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","exePath","").isEmpty())
        {
            exePath = AppConfigBase::getInstance()->readConfigSettings("FlatnessTester","exePath","");
        }
        QProcess* process = new QProcess(this);
        bool stat = process->startDetached(QString("\"%1\"").arg(exePath));
        if(stat){
            qDebug() << "start app success!!!";
        }else{
            qDebug() << "start app fail!!!";
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("平整度仪启动失败"));
        }
    }
#endif
    return true;
}

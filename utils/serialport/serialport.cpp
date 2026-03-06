#include "serialport.h"
#include "appeventbase.h"

SerialPort::SerialPort(QObject *parent) : QObject(parent)
{
    m_serialport = new QSerialPort(this);
    connect(m_serialport,SIGNAL(readyRead()),this,SLOT(slt_rcvData()),Qt::UniqueConnection);
    connect(m_serialport,SIGNAL(errorOccurred()),this,SLOT(slt_errorOccurred()),Qt::UniqueConnection);
    m_serialport->setReadBufferSize(1024 * 1024); // 1MB 缓冲区
}

QByteArray SerialPort::getRcvData()
{
    if(nullptr == m_rcvData)
        return nullptr;
    else
    {
        QByteArray array = m_rcvData;
        m_rcvData = nullptr;
        return array;
    }
}

QList<QSerialPortInfo> SerialPort::getSerialportList(){
    //获得所有可用端口列表
    QList<QSerialPortInfo> serialPortInfoList = QSerialPortInfo::availablePorts();
    return serialPortInfoList;
}

void SerialPort::closeSerialport()
{
    //判断串口开启状态
    if(m_serialport->isOpen()){
        //若串口已经打开，则关闭它
        m_serialport->clear();
        m_serialport->close();
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("GPS连接关闭成功"));
    }
}

bool SerialPort::getStatus()
{
    return m_serialport->isOpen();
}

bool SerialPort::openSerialport(QString portName){
    closeSerialport();
    //若串口没有打开，则打开选择的串口
    m_serialport->setPortName(portName);
    m_serialport->setBaudRate(QSerialPort::Baud115200);
    m_serialport->setDataBits(QSerialPort::Data8);
    m_serialport->setParity(QSerialPort::NoParity);
    m_serialport->setStopBits(QSerialPort::OneStop);
    m_serialport->setFlowControl(QSerialPort::NoFlowControl);
    return m_serialport->open(QIODevice::ReadWrite);
}

void SerialPort::slt_rcvData(){
    while (m_serialport->bytesAvailable() > 0)
    {
         m_rcvData += m_serialport->readAll();
    }
    emit sig_sendRcvData("");
}

void SerialPort::slt_errorOccurred(QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::NoError)
    {
        qDebug() << "串口异常" << m_serialport->errorString();
    }
}

void SerialPort::slt_sendData(QString msg){
    QByteArray messageSend;
    messageSend.append(msg);
    if(m_serialport->isOpen())
    {
        m_serialport->write(messageSend);
        m_serialport->flush();
    }
}


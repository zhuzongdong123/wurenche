#include "logmanager.h"
#include "appcommonbase.h"
//QString filePath = "";
//QString logLevel = "Debug";
#include "appcommonbase.h"
#include <QMutex>
#include <QDateTime>
#include <QDir>

LogManager::LogManager(QObject *parent) : QThread(parent)
{

}

LogManager::~LogManager()
{
    stopThread();
}

LogManager* LogManager::getInstance()
{
    LogManager* pAppCommonBase = nullptr;
    static LogManager appCommonBase;
    pAppCommonBase = &appCommonBase;
    return pAppCommonBase;
}

void LogManager::stopThread()
{
    m_isRun = false;
    quit();
    wait();
}

void LogManager::startThread()
{
    m_isRun = true;
    start();
}

void LogManager::outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(!m_isRun || m_logPath.isEmpty())
        return;

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
        break;

    case QtInfoMsg:
        text = QString("Info:");
    }

    static QString logLevel = "Debug";
    //log输出等级为QtWarningMsg=>QtDebugMsg=>QtInfoMsg=>QtCriticalMsg=>QtFatalMsg,只输出比它本身高等级的log
    if(logLevel == "Debug" && type == QtWarningMsg)
    {
        return;
    }
    else if(logLevel == "Info" && (type == QtWarningMsg || type == QtDebugMsg))
    {
        return;
    }
    else if(logLevel == "Critical" && (type == QtWarningMsg || type == QtDebugMsg || type == QtInfoMsg))
    {
        return;
    }

    QJsonObject objTemp;
    objTemp.insert("msg",msg);
    objTemp.insert("time",QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ddd"));
    m_mutex.lock();
    m_array.push_back(objTemp);
    m_mutex.unlock();
    m_isRun = true;
}

void LogManager::run()
{
    while (m_isRun) {
        m_mutex.lock();
        QJsonArray array = m_array;
        QJsonArray arrayTemp;
        m_array.swap(arrayTemp);
        m_mutex.unlock();

        static QString filePath;
        QFile file(m_logPath);
        file.open(QIODevice::ReadWrite | QIODevice::Append);
        for(auto obj : array)
        {
            QString message = QString("%1 %2").arg(obj.toObject().value("time").toString()).arg(obj.toObject().value("msg").toString());
            QTextStream text_stream(&file);
            text_stream << message << endl;
            file.flush();
        }
        file.close();

        QThread::msleep(10);
    }
}

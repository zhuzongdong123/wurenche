#include "dbthread.h"

DBThread::DBThread(QObject *parent) : QThread(parent)
{

}

void DBThread::stopThread()
{
    m_isStopThread = true;
}

void DBThread::run()
{
    while (!m_isStopThread) {
        m_mutex.lock();
        QVector<urlData> saveDBStruct = m_saveDBStruct;
        m_mutex.unlock();

        if(saveDBStruct.size() > 0)
        {
            urlData data = saveDBStruct.front();
            m_paramsDao.slt_rcvData(data.url,data.obj);

            m_mutex.lock();
            m_saveDBStruct.pop_front();
            m_mutex.unlock();
        }

        QThread::msleep(10);
    }
}

void DBThread::saveDB(QString url, QJsonObject obj)
{
    urlData data;
    data.url = url;
    data.obj = obj;
    m_mutex.lock();
    m_saveDBStruct.push_back(data);
    m_mutex.unlock();
}

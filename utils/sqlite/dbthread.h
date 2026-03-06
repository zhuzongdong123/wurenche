#ifndef DBTHREAD_H
#define DBTHREAD_H

#include <QThread>
#include "paramsdao.h"
#include <QVector>
#include <QMutex>

class DBThread : public QThread
{
    Q_OBJECT
public:
    explicit DBThread(QObject *parent = nullptr);
    void stopThread();

protected:
    void run();

public slots:
    void saveDB(QString url, QJsonObject obj);

signals:

private:
    ParamsDao m_paramsDao;
    bool m_isStopThread = false;
    struct urlData
    {
        QString url;
        QJsonObject obj;
    };
    QVector<urlData> m_saveDBStruct;
    QMutex m_mutex;
};

#endif // DBTHREAD_H

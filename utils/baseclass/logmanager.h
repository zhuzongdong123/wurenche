#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QThread>
#include <QJsonArray>
#include <QMutex>

class LogManager : public QThread
{
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = nullptr);
    ~LogManager();

    void outputLog(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static LogManager *getInstance();
    void stopThread();
    void startThread();
    QString m_logPath;
signals:

protected:
    void run();
    bool m_isRun = false;
    QJsonArray m_array;
    QMutex m_mutex;
};

#endif // LOGMANAGER_H

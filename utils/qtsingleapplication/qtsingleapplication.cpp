// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause


#include "qtsingleapplication.h"
#include "qtlocalpeer.h"
#include <QWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProcess>
#ifdef Q_OS_WIN
#include "windows.h"
#endif
#include "logmanager.h"
#include "appcommonbase.h"

/*!
    \class QtSingleApplication qtsingleapplication.h
    \brief The QtSingleApplication class provides an API to detect and
    communicate with running instances of an application.

    This class allows you to create applications where only one
    instance should be running at a time. I.e., if the user tries to
    launch another instance, the already running instance will be
    activated instead. Another usecase is a client-server system,
    where the first started instance will assume the role of server,
    and the later instances will act as clients of that server.

    By default, the full path of the executable file is used to
    determine whether two processes are instances of the same
    application. You can also provide an explicit identifier string
    that will be compared instead.

    The application should create the QtSingleApplication object early
    in the startup phase, and call isRunning() to find out if another
    instance of this application is already running. If isRunning()
    returns false, it means that no other instance is running, and
    this instance has assumed the role as the running instance. In
    this case, the application should continue with the initialization
    of the application user interface before entering the event loop
    with exec(), as normal.

    The messageReceived() signal will be emitted when the running
    application receives messages from another instance of the same
    application. When a message is received it might be helpful to the
    user to raise the application so that it becomes visible. To
    facilitate this, QtSingleApplication provides the
    setActivationWindow() function and the activateWindow() slot.

    If isRunning() returns true, another instance is already
    running. It may be alerted to the fact that another instance has
    started by using the sendMessage() function. Also data such as
    startup parameters (e.g. the name of the file the user wanted this
    new instance to open) can be passed to the running instance with
    this function. Then, the application should terminate (or enter
    client mode).

    If isRunning() returns true, but sendMessage() fails, that is an
    indication that the running instance is frozen.

    Here's an example that shows how to convert an existing
    application to use QtSingleApplication. It is very simple and does
    not make use of all QtSingleApplication's functionality (see the
    examples for that).

    \code
    // Original
    int main(int argc, char **argv)
    {
        QApplication app(argc, argv);

        MyMainWidget mmw;
        mmw.show();
        return app.exec();
    }

    // Single instance
    int main(int argc, char **argv)
    {
        QtSingleApplication app(argc, argv);

        if (app.isRunning())
            return !app.sendMessage(someDataString);

        MyMainWidget mmw;
        app.setActivationWindow(&mmw);
        mmw.show();
        return app.exec();
    }
    \endcode

    Once this QtSingleApplication instance is destroyed (normally when
    the process exits or crashes), when the user next attempts to run the
    application this instance will not, of course, be encountered. The
    next instance to call isRunning() or sendMessage() will assume the
    role as the new running instance.

    For console (non-GUI) applications, QtSingleCoreApplication may be
    used instead of this class, to avoid the dependency on the QtGui
    library.

    \sa QtSingleCoreApplication
*/


void QtSingleApplication::sysInit(const QString &appId)
{
    actWin = 0;
    peer = new QtLocalPeer(this, appId);
    connect(peer, SIGNAL(messageReceived(const QString&)), SIGNAL(messageReceived(const QString&)));
}


/*!
    Creates a QtSingleApplication object. The application identifier
    will be QCoreApplication::applicationFilePath(). \a argc, \a
    argv, and \a GUIenabled are passed on to the QAppliation constructor.

    If you are creating a console application (i.e. setting \a
    GUIenabled to false), you may consider using
    QtSingleCoreApplication instead.
*/

QString filePath = "";
QString logLevel = "Debug";
#include "appcommonbase.h"
#include <QMutex>
#include <QDateTime>
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    //static qint64 max = 10485760;//10 * 1024 * 1024;

    if(AppCommonBase::getInstance()->g_logFolder.isEmpty())
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

    QString dir(AppCommonBase::getInstance()->g_logFolder + "/");
    QDir directory;
    bool exist = directory.exists(dir);
    if(exist == false)
    {
        directory.mkdir(dir);
    }

    if(filePath == "")
    {
        filePath = dir + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    }

    //每天生成一个新的log
    QString dayTime = QDateTime::currentDateTime().toString("yyyyMMdd");
    if(!filePath.contains(dayTime)) {
        filePath = dir + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    }

    if(!AppCommonBase::getInstance()->g_logFolder.isEmpty())
    {
        LogManager::getInstance()->m_logPath = filePath;
        LogManager::getInstance()->outputLog(type,context,msg);
        return;
    }

    static QMutex mutex;
    mutex.lock();
    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ddd");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2 %3").arg(text).arg(current_date).arg(msg);

    QFile file(filePath);
    //理论上不存在
    {
        QTextStream text_stream(&file);
        file.open(QIODevice::ReadWrite | QIODevice::Append);
        text_stream << message << endl;
        file.flush();
        file.close();
    }
    mutex.unlock();
}

QtSingleApplication::QtSingleApplication(int &argc, char **argv, bool GUIenabled)
    : QApplication(argc, argv, GUIenabled)
{
    sysInit();

    //添加全局监听功能
    qApp->installEventFilter(this);
    qInstallMessageHandler(outputMessage);
}


/*!
    Creates a QtSingleApplication object with the application
    identifier \a appId. \a argc and \a argv are passed on to the
    QAppliation constructor.
*/

QtSingleApplication::QtSingleApplication(const QString &appId, int &argc, char **argv)
    : QApplication(argc, argv)
{
    sysInit(appId);

    //添加全局监听功能
    qApp->installEventFilter(this);
    qInstallMessageHandler(outputMessage);
}

#if QT_VERSION < 0x050000

/*!
    Creates a QtSingleApplication object. The application identifier
    will be QCoreApplication::applicationFilePath(). \a argc, \a
    argv, and \a type are passed on to the QAppliation constructor.
*/
QtSingleApplication::QtSingleApplication(int &argc, char **argv, Type type)
    : QApplication(argc, argv, type)
{
    sysInit();
}


#  if defined(Q_WS_X11)
/*!
  Special constructor for X11, ref. the documentation of
  QApplication's corresponding constructor. The application identifier
  will be QCoreApplication::applicationFilePath(). \a dpy, \a visual,
  and \a cmap are passed on to the QApplication constructor.
*/
QtSingleApplication::QtSingleApplication(Display* dpy, Qt::HANDLE visual, Qt::HANDLE cmap)
    : QApplication(dpy, visual, cmap)
{
    sysInit();
}

/*!
  Special constructor for X11, ref. the documentation of
  QApplication's corresponding constructor. The application identifier
  will be QCoreApplication::applicationFilePath(). \a dpy, \a argc, \a
  argv, \a visual, and \a cmap are passed on to the QApplication
  constructor.
*/
QtSingleApplication::QtSingleApplication(Display *dpy, int &argc, char **argv, Qt::HANDLE visual, Qt::HANDLE cmap)
    : QApplication(dpy, argc, argv, visual, cmap)
{
    sysInit();
}

/*!
  Special constructor for X11, ref. the documentation of
  QApplication's corresponding constructor. The application identifier
  will be \a appId. \a dpy, \a argc, \a
  argv, \a visual, and \a cmap are passed on to the QApplication
  constructor.
*/
QtSingleApplication::QtSingleApplication(Display* dpy, const QString &appId, int argc, char **argv, Qt::HANDLE visual, Qt::HANDLE cmap)
    : QApplication(dpy, argc, argv, visual, cmap)
{
    sysInit(appId);
}
#  endif // Q_WS_X11
#endif // QT_VERSION < 0x050000


/*!
    Returns true if another instance of this application is running;
    otherwise false.

    This function does not find instances of this application that are
    being run by a different user (on Windows: that are running in
    another session).

    \sa sendMessage()
*/

bool QtSingleApplication::isRunning()
{
    return peer->isClient();
}


/*!
    Tries to send the text \a message to the currently running
    instance. The QtSingleApplication object in the running instance
    will emit the messageReceived() signal when it receives the
    message.

    This function returns true if the message has been sent to, and
    processed by, the current instance. If there is no instance
    currently running, or if the running instance fails to process the
    message within \a timeout milliseconds, this function return false.

    \sa isRunning(), messageReceived()
*/
bool QtSingleApplication::sendMessage(const QString &message, int timeout)
{
    return peer->sendMessage(message, timeout);
}


/*!
    Returns the application identifier. Two processes with the same
    identifier will be regarded as instances of the same application.
*/
QString QtSingleApplication::id() const
{
    return peer->applicationId();
}


/*!
  Sets the activation window of this application to \a aw. The
  activation window is the widget that will be activated by
  activateWindow(). This is typically the application's main window.

  If \a activateOnMessage is true (the default), the window will be
  activated automatically every time a message is received, just prior
  to the messageReceived() signal being emitted.

  \sa activateWindow(), messageReceived()
*/

void QtSingleApplication::setActivationWindow(QWidget* aw, bool activateOnMessage)
{
    actWin = aw;
    if (activateOnMessage)
        connect(peer, SIGNAL(messageReceived(const QString&)), this, SLOT(activateWindow()));
    else
        disconnect(peer, SIGNAL(messageReceived(const QString&)), this, SLOT(activateWindow()));
}


/*!
    Returns the applications activation window if one has been set by
    calling setActivationWindow(), otherwise returns 0.

    \sa setActivationWindow()
*/
QWidget* QtSingleApplication::activationWindow() const
{
    return actWin;
}


/*!
  De-minimizes, raises, and activates this application's activation window.
  This function does nothing if no activation window has been set.

  This is a convenience function to show the user that this
  application instance has been activated when he has tried to start
  another instance.

  This function should typically be called in response to the
  messageReceived() signal. By default, that will happen
  automatically, if an activation window has been set.

  \sa setActivationWindow(), messageReceived(), initialize()
*/
void QtSingleApplication::activateWindow()
{
    if (actWin) {
        actWin->setWindowState(actWin->windowState() & ~Qt::WindowMinimized);
        actWin->raise();
        actWin->activateWindow();
    }
}


/*!
    \fn void QtSingleApplication::messageReceived(const QString& message)

    This signal is emitted when the current instance receives a \a
    message from another instance of this application.

    \sa sendMessage(), setActivationWindow(), activateWindow()
*/


/*!
    \fn void QtSingleApplication::initialize(bool dummy = true)

    \obsolete
*/

#include <QKeyEvent>
bool QtSingleApplication::eventFilter(QObject *watched, QEvent *event)
{
//    if(event->type() == QEvent::MouseButtonPress)
//    {
//        //static QProcess process;
//        if (watched->inherits("QLineEdit") || watched->inherits("QTextEdit") || watched->inherits("QSpinBox")
//            || watched->inherits("QDoubleSpinBox"))
//        {
//#ifdef Q_OS_WIN
//            PVOID OldValue;
//            BOOL bRet = Wow64DisableWow64FsRedirection (&OldValue);
//            QString csProcess = "C:\\Windows\\System32\\osk.exe";
//            QString params = "";
//            ShellExecute(NULL, L"open", (LPCWSTR)csProcess.utf16(), (LPCWSTR)params.utf16(), NULL, SW_SHOWNORMAL);
//            if( bRet ){
//                Wow64RevertWow64FsRedirection(OldValue);
//            }
//#endif
//        }
////        else
////        {
////            QProcess p;
////            QString c = "taskkill /im osk.exe /f";    //exeFileName为要杀死的进程名
////            p.execute(c);
////            p.close();
////        }
//    }

    return QApplication::eventFilter(watched, event);
}

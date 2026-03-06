#include "mainwindow.h"
#include "appconfigbase.h"
#include <QApplication>
#include <QTextCodec>
#include "qtsingleapplication.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>

static QString g_strDumpPath = "";

//#if defined (_WIN32)
//#include <Windows.h>          // Windows.h必须放在DbgHelp.h前，否则编译会报错
//#include <DbgHelp.h>

//LONG crashHandler(EXCEPTION_POINTERS *pException)
//{
//    QString curDataTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
//    QString dumpName = g_strDumpPath + curDataTime + ".dmp";

//    HANDLE dumpFile = CreateFile((LPCWSTR)QString(dumpName).utf16(),GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//    if(dumpFile != INVALID_HANDLE_VALUE)
//    {
//        MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
//        dumpInfo.ExceptionPointers = pException;
//        dumpInfo.ThreadId = GetCurrentThreadId();
//        dumpInfo.ClientPointers = TRUE;

//        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),dumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
//        CloseHandle(dumpFile);
//    }
//    else
//    {
//        qDebug() << "dumpFile not vaild";
//    }

//    return EXCEPTION_EXECUTE_HANDLER;
//}

////防止CRT（C runtime）函数报错可能捕捉不到
//void DisableSetUnhandledExceptionFilter()
//{
//    void* addr = (void*)GetProcAddress(LoadLibrary(L"kernel32.dll"), "SetUnhandledExceptionFilter");
//    if(addr)
//    {
//        unsigned char code[16];
//        int size = 0;

//        code[size++] = 0x33;
//        code[size++] = 0xC0;
//        code[size++] = 0xC2;
//        code[size++] = 0x04;
//        code[size++] = 0x00;

//        DWORD dwOldFlag, dwTempFlag;
//        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
//        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
//        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
//    }
//}

//#endif

int main(int argc, char *argv[])
{
    QtSingleApplication a(argc, argv);
//    // 加载字体文件
//    QFontDatabase::addApplicationFont("C:/Users/Administrator/Desktop/style/SourceHanSansCN-Medium.otf");
//    QFontDatabase::addApplicationFont("C:/Users/Administrator/Desktop/style/SourceHanSansCN-Regular.otf");
    qApp->setStyleSheet("font-family:微软雅黑");

    //读取配置文件
    AppConfigBase::getInstance()->readConfig();
    AppConfigBase::getInstance()->readCameraConfig();
    MainWindow w;
    w.setFixedSize(1920,1080);
    w.showMaximized();
    //使主窗口关闭时，主事件循环退出，所有的子窗口同步关闭
    a.connect( &w,
               SIGNAL(slt_mainWindowQuit()),
               &a,
               SLOT(quit()));
    return a.exec();
}

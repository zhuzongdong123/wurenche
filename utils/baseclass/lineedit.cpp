#include "lineedit.h"
#include <QProcess>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QHBoxLayout>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

LineEdit::LineEdit(QWidget *parent) : QLineEdit(parent)
{
    //禁用右键事件
    setContextMenuPolicy(Qt::NoContextMenu);
}

void LineEdit::showEvent(QShowEvent *event)
{
//    //添加软键盘图片
//    if(nullptr == m_iconBtn)
//    {
//        m_iconBtn = new QPushButton(this);
//        connect(m_iconBtn, &QPushButton::clicked, this, &LineEdit::slt_keyBoardClicked);
//        m_iconBtn->setStyleSheet("background:transparent;border:none");
//        m_iconBtn->setIcon(QIcon(":/image/keyboard.png"));
//        QHBoxLayout* layout = new QHBoxLayout(this);
//        layout->setMargin(0);
//        layout->setContentsMargins(0, 0, 6, 0);
//        layout->setSpacing(0);
//        layout->addStretch(0);
//        layout->addWidget(m_iconBtn);
//        setLayout(layout);
//    }
}

void LineEdit::slt_keyBoardClicked()
{
#ifdef Q_OS_WIN
    PVOID OldValue;
    BOOL bRet = Wow64DisableWow64FsRedirection (&OldValue);
    QString csProcess = "C:\\Windows\\System32\\osk.exe";
    QString params = "";
    ShellExecute(NULL, L"open", (LPCWSTR)csProcess.utf16(), (LPCWSTR)params.utf16(), NULL, SW_SHOWNORMAL);
    if( bRet ){
        Wow64RevertWow64FsRedirection(OldValue);
    }
#endif
}

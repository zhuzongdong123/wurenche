#include "webview.h"
#include <QNetworkProxyFactory>
#include <QStandardPaths>
#include <QDir>
#include <QShortcut>
#include <QMenu>
#include <QAction>
#include <QWebEngineSettings>

WebView::WebView(QWidget *parent) : QWebEngineView(parent)
{
    init();
    //this->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true); // 启用 WebGL 硬件加速
}

void WebView::load(const QUrl &url)
{
    qDebug() << "当前加载路径: " << url.url();
    QWebEngineView::load(url);
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QWebEngineView::contextMenuEvent(event);
}

void WebView::deleteLocalDataFolder()
{
    //删除QtWebEngine缓存和cookies目录
    QString dataLocal = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    qDebug() << dataLocal;
    QDir cacheDir(dataLocal );
    cacheDir.removeRecursively();
}

void WebView::init()
{
    setAttribute(Qt::WA_AcceptTouchEvents,true);
    this->setContextMenuPolicy(Qt::NoContextMenu);
    m_webChannel = new QWebChannel(this);
    m_content = new Document(this);
    m_webChannel->registerObject(QStringLiteral("cppObject"), m_content);
    this->page()->setWebChannel(m_webChannel);
    this->page()->setBackgroundColor(QColor(255,255,255));//来设置背景颜色

    //打印userAgent
    QString user_agent = this->page()->profile()->httpUserAgent();
    qDebug() << "user_agent" << user_agent;
}

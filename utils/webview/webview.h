#ifndef MYWEBENGINEVIEW_H
#define MYWEBENGINEVIEW_H

#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWebChannel>
#include "document.h"
#include <QFrame>

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = nullptr);
    Document* getDocument(){return m_content;}
    QWebChannel *getWebChannel(){return m_webChannel;}
    void load(const QUrl &url);

    virtual void contextMenuEvent(QContextMenuEvent *event);  //右键菜单事件

protected:
    virtual QWebEngineView *createWindow(QWebEnginePage::WebWindowType type)
    {
        QWebEngineView *pWebView = new QWebEngineView(this);
        connect(pWebView, &QWebEngineView::urlChanged, this, [this, pWebView](const QUrl &url)
        {
            pWebView->deleteLater();
            qDebug() << "url: " << url;
            this->load(url);
        });
        return pWebView;
    }

private:
    QWebChannel *m_webChannel;
    Document* m_content;

private:
    void deleteLocalDataFolder();
    void init();
};

#endif // MYWEBENGINEVIEW_H

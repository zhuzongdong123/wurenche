/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QDebug>
#include <QObject>
#include <QJsonObject>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

Q_DECLARE_METATYPE(QJsonObject)

class Document : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJsonObject m_json READ getJson NOTIFY signalToWeb)

public:
    explicit Document(QObject *parent = nullptr) : QObject(parent)
    {
        qRegisterMetaType<QJsonObject>("QJsonObject");
//        connect(AppEvent::getInstance(),&AppEvent::sig_sendLocalMsgToWeb,this,[=](QJsonObject obj){
//            m_jsonVec.push_back(obj);
//        });

//        connect(&m_timer,&QTimer::timeout,[=](){
//                if(0 != m_jsonVec.size())
//                {
//                    QJsonObject obj = m_jsonVec.front();
//                    m_jsonVec.pop_front();
//                    sendJsonToWeb(obj);
//                }
//            });
//        //刷新一次
//        m_timer.start(500);
    }


    inline QJsonObject getJson() const
    {
        return m_json;
    }

    // js端通过注册的对象ID调用
public slots:
    // 必须为槽函数,web端向QT发送消息
    void sendTextToCpp(const QJsonObject &obj)
    {
        qDebug() << "【收到web发送的消息: " << obj;
        emit sig_sendWebMsgToLocal(obj);
    }

    // cpp端直接通过对象调用(QT端向web发送消息)
    void sendJsonToWeb(const QJsonObject &json)
    {
        QMutexLocker locker(&mutex);
        qDebug() << "向web发出的消息:" << json;
        //if (m_json != json)
        {
            m_json = json;
            emit signalToWeb(json);
        }
    }

signals:
    void signalToWeb(const QJsonObject &json);
    void sig_sendWebMsgToLocal(QJsonObject obj);

private:
    QJsonObject m_json;
    QMutex mutex;
    QVector<QJsonObject> m_jsonVec;
    QTimer m_timer;
};

#endif // DOCUMENT_H

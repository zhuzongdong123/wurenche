#ifndef FORMPILENUMUPDATE_H
#define FORMPILENUMUPDATE_H

#include <QWidget>
#include "httpclient.h"

namespace Ui {
class FormPileNumUpdate;
}

class FormPileNumUpdate : public QWidget
{
    Q_OBJECT

public:
    explicit FormPileNumUpdate(QWidget *parent = nullptr);
    ~FormPileNumUpdate();

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::FormPileNumUpdate *ui;
    HttpClient m_httpClient;//后端请求交互类
    QString m_openTime;
    QJsonObject m_gpsObj;
    double m_currentStation = 0.0;

private slots:
    void slt_saveBtnClicked();
};

#endif // FORMPILENUMUPDATE_H

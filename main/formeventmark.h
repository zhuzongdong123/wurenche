#ifndef FORMEVENTMARK_H
#define FORMEVENTMARK_H

#include <QWidget>
#include "httpclient.h"

namespace Ui {
class FormEventMark;
}

class FormEventMark : public QWidget
{
    Q_OBJECT

public:
    explicit FormEventMark(QWidget *parent = nullptr);
    ~FormEventMark();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void slt_saveBtnClicked();
    void slt_otherBtnClicked(bool isChecked);

private:
    Ui::FormEventMark *ui;
    HttpClient m_httpClient;//后端请求交互类
    QString m_openTime;
    QJsonObject m_gpsObj;
    double m_currentStation;
};

#endif // FORMEVENTMARK_H

#include "formeventmark.h"
#include "ui_formeventmark.h"
#include <QDateTime>
#include "appconfigbase.h"
#include "appcommonbase.h"
#include <QRadioButton>
#include <QMessageBox>
#include "appeventbase.h"

FormEventMark::FormEventMark(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormEventMark)
{
    ui->setupUi(this);
    //去掉最大化、最小化按钮，保留关闭按钮
    this->setWindowFlags(Qt::WindowCloseButtonHint);

    //[保存]按钮点击后，调用槽函数
    connect(ui->saveBtn, &QPushButton::clicked, this, &FormEventMark::slt_saveBtnClicked);
    //[取消]按钮点击后，调用槽函数
    connect(ui->cancleBtn, &QPushButton::clicked, this, &FormEventMark::close);
}

FormEventMark::~FormEventMark()
{
    delete ui;
}

void FormEventMark::showEvent(QShowEvent *event)
{
    //获取当前系统时间
    m_openTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    m_gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
    m_currentStation = AppCommonBase::getInstance()->g_currentStation;
    ui->current_station->setText(QString::number(m_currentStation));
    ui->pulse_value->setText(QString::number(m_gpsObj.value("pulse_value").toInt()));
    raise();
    QWidget::show();
}

void FormEventMark::slt_saveBtnClicked()
{
    QString marker;

    //参数信息保存到数据库
    QJsonObject objParams;
    objParams.insert("marker",ui->lineEdit->text());//事件标记
    objParams.insert("type","1");//1：非路面类型事件
    objParams.insert("lon",m_gpsObj.value("lon").toDouble());//经度
    objParams.insert("lat",m_gpsObj.value("lat").toDouble());//纬度
    objParams.insert("locationId",m_gpsObj.value("id").toString());//经纬度数据的唯一标识
    objParams.insert("createTime",m_openTime);//打开对话框的时间
    int pulse_value = m_gpsObj.value("pulse_value").toInt();
    objParams.insert("pulse_value",pulse_value);//脉冲值
    objParams.insert("current_distance",m_gpsObj.value("distance").toDouble());//当前里程
    objParams.insert("current_station",m_currentStation);//当前桩号
    objParams.insert("GPS_time",m_gpsObj.value("GPS_time").toString());//当前gps时间
    AppEventBase::getInstance()->sig_requestServer(SAVE_EVENT_MARK,objParams);

    close();
}

void FormEventMark::slt_otherBtnClicked(bool isChecked)
{
    ui->remarkWidget->setEnabled(isChecked);
    if(!isChecked)
    {
        ui->lineEdit->clear();
    }
}

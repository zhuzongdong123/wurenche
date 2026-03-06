#include "formpilenumupdate.h"
#include "ui_formpilenumupdate.h"
#include <QDateTime>
#include "appconfigbase.h"
#include "appcommonbase.h"
#include "appeventbase.h"
#include <QMessageBox>
#include <QRegExpValidator>

FormPileNumUpdate::FormPileNumUpdate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormPileNumUpdate)
{
    ui->setupUi(this);
    //去掉最大化、最小化按钮，保留关闭按钮
    this->setWindowFlags(Qt::WindowCloseButtonHint);

    //[保存]按钮点击后，调用槽函数
    connect(ui->saveBtn, &QPushButton::clicked, this, &FormPileNumUpdate::slt_saveBtnClicked);
    //[取消]按钮点击后，调用槽函数
    connect(ui->cancleBtn, &QPushButton::clicked, this, &FormPileNumUpdate::close);

    ui->pileNum_km->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->pileNum_m->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
}

FormPileNumUpdate::~FormPileNumUpdate()
{
    delete ui;
}

void FormPileNumUpdate::showEvent(QShowEvent *event)
{
    //获取当前系统时间
    m_openTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    m_gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
    m_currentStation = AppCommonBase::getInstance()->g_currentStation;
    ui->pileNum_km->clear();
    ui->pileNum_m->clear();
    ui->current_station->setText(QString::number(m_currentStation));
    ui->pulse_value->setText(QString::number(m_gpsObj.value("pulse_value").toInt()));

    raise();
    QWidget::show();
}

void FormPileNumUpdate::slt_saveBtnClicked()
{
    if(ui->pileNum_km->text().isEmpty() && ui->pileNum_m->text().isEmpty())
    {
        QMessageBox::information(this,("警告"),("校桩内容不能为空"));
        return;
    }

    //参数信息保存到数据库
    QJsonObject objParams;
    objParams.insert("pileNum",QString::number(ui->pileNum_km->text().toInt() + double(ui->pileNum_m->text().toInt())/1000,'f',3));//桩号
    objParams.insert("lon",m_gpsObj.value("lon").toDouble());//经度
    objParams.insert("lat",m_gpsObj.value("lat").toDouble());//纬度
    objParams.insert("locationId",m_gpsObj.value("id").toString());//经纬度数据的唯一标识
    objParams.insert("createTime",m_openTime);//打开对话框的时间
    int pulse_value = m_gpsObj.value("pulse_value").toInt();
    objParams.insert("pulse_value",pulse_value);//脉冲值
    objParams.insert("current_distance",m_gpsObj.value("distance").toDouble());//当前里程
    objParams.insert("GPS_time",m_gpsObj.value("GPS_time").toString());//当前gps时间
    AppEventBase::getInstance()->sig_requestServer(UPDATE_PILENUM,objParams);

    close();
}

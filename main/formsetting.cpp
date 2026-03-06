#include "formsetting.h"
#include "ui_formsetting.h"
#include "serialport.h"
#include "appconfigbase.h"
#include "appcommonbase.h"
#include <QFileDialog>
#include <QMessageBox>
#include "appeventbase.h"
#include <QListView>

FormSetting::FormSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormSetting)
{
    ui->setupUi(this);
    //去掉最大化、最小化按钮，保留关闭按钮
    this->setWindowFlags(Qt::WindowCloseButtonHint);

    //初始化
    init();

    //创建信号槽
    createConnect();
}

FormSetting::~FormSetting()
{
    delete ui;
}

QString FormSetting::getGpsPort()
{
    return ui->serialPortInfoBox->currentText();
}

QString FormSetting::getSaveFolderPath()
{
    return ui->savePathEdit->text();
}

int FormSetting::getStartMileage()
{
    return ui->pileNumEdit_km->text().toInt()*1000 + ui->pileNumEdit_m->text().toInt();
}

/**
 * @brief init 类的初始化
 */
void FormSetting::init()
{
    //获取上一次的执行人和执行单位，显示到界面上
    QString optPerson = AppConfigBase::getInstance()->readConfigSettings("Setting","preOptPerson","");
    QString optUnit = AppConfigBase::getInstance()->readConfigSettings("Setting","preOptUnit","");
    ui->optPersonEdit->setText(optPerson);
    ui->optUnitEdit->setText(optUnit);

    //获取所有的串口号显示到下拉框中
    SerialPort serialPort;
    QList<QSerialPortInfo> list = serialPort.getSerialportList();
    for(auto serialPortInfo : list)
    {
        ui->serialPortInfoBox->addItem(serialPortInfo.portName());
    }
    //获取上一次使用的RTK的串口号，并默认显示到界面上
    QString preGPSPort = AppConfigBase::getInstance()->readConfigSettings("Setting","preGPSPort","");
    ui->serialPortInfoBox->setCurrentText(preGPSPort);

    ui->pileNumEdit_km->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->pileNumEdit_m->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->iriRange->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    ui->upAndDownBox->setView(new QListView());
    ui->serialPortInfoBox->setView(new QListView());
}

/**
 * @brief createConnect 创建信号槽连接
 */
void FormSetting::createConnect()
{
    //[保存]按钮点击后，调用槽函数
    connect(ui->saveBtn, &QPushButton::clicked, this, &FormSetting::slt_saveBtnClicked);
    //[取消]按钮点击后，调用槽函数
    connect(ui->cancleBtn, &QPushButton::clicked, this, &FormSetting::close);
    //[选择]按钮点击后，调用文件资源管理器
    connect(ui->selectFolderBtn, &QPushButton::clicked, this, &FormSetting::slt_selectFolderBtnClicked);
}

/**
 * @brief checkParams 参数校验(例如：必输项校验等)
 * @return  返回值(return): bool true:成功 false:失败
 */
bool FormSetting::checkParams()
{
    //判断u盘路径等是否存在
    if(ui->roadNameEdit->text().isEmpty())
    {
        QMessageBox::information(this,"警告","路名不能为空");
        return false;
    }

    if(ui->savePathEdit->text().isEmpty())
    {
        QMessageBox::information(this,"警告","存储路径不能为空");
        return false;
    }

    if(!ui->camera0_checkbox->isChecked() && !ui->camera3_checkbox->isChecked() && !ui->camera1_checkbox->isChecked()
            && !ui->flatnessTester_checkbox->isChecked() && !ui->accelerometer_checkbox->isChecked()
            && !ui->gps_checkbox->isChecked())
    {
        QMessageBox::information(this,"警告","请选择预览的设备");
        return false;
    }

    //判断必输项参数是否为空
    return true;
}

/**
 * @brief slt_saveBtnClicked 点击了保存按钮
 */
void FormSetting::slt_saveBtnClicked()
{
    //参数校验，校验失败不处理
    if(!checkParams())
    {
        return;
    }

    //参数信息保存到数据库
    QJsonObject objParams;
    objParams.insert("startMileage",ui->pileNumEdit_km->text().toInt()*1000+ui->pileNumEdit_m->text().toInt());//开始里程
    objParams.insert("roadName",ui->roadNameEdit->text());//路名
    objParams.insert("pileNum",QString("%1km").arg(ui->pileNumEdit_km->text().toInt()+0.001*ui->pileNumEdit_m->text().toInt()));//桩号
    objParams.insert("pileNum_km",ui->pileNumEdit_km->text());//
    objParams.insert("pileNum_m",ui->pileNumEdit_m->text());//
    objParams.insert("iriRange",ui->iriRange->text());//IRI计算区间
    objParams.insert("upAndDown",ui->upAndDownBox->currentText());//上下行
    objParams.insert("technicalLevel",ui->buttonGroup->button(ui->buttonGroup->checkedId())->text());//技术等级
    objParams.insert("lane",ui->laneEdit->text());//车道
    objParams.insert("optPerson",ui->optPersonEdit->text());//执行人
    objParams.insert("optUnit",ui->optUnitEdit->text());//执行单位
    objParams.insert("savePath",ui->savePathEdit->text());//存储路径
    objParams.insert("camera0Flag",ui->camera0_checkbox->isChecked());//是否开启当前设备
    objParams.insert("camera1Flag",ui->camera1_checkbox->isChecked());//是否开启当前设备
    objParams.insert("camera3Flag",ui->camera3_checkbox->isChecked());//是否开启当前设备
    objParams.insert("flatnessTesterFlag",ui->flatnessTester_checkbox->isChecked());//是否开启当前设备
    objParams.insert("accelerometerFlag",ui->accelerometer_checkbox->isChecked());//是否开启当前设备
    objParams.insert("gpsFlag",ui->gps_checkbox->isChecked());//是否开启当前设备
    //AppEventBase::getInstance()->sig_requestServer(SAVE_PARAMS,objParams);
    AppCommonBase::getInstance()->g_logFolder = ui->savePathEdit->text();

    //部分信息保存到配置文件中
    AppConfigBase::getInstance()->updateConfigSetting("Setting","preOptPerson",ui->optPersonEdit->text());
    AppConfigBase::getInstance()->updateConfigSetting("Setting","preOptUnit",ui->optUnitEdit->text());
    AppConfigBase::getInstance()->updateConfigSetting("Setting","preGPSPort",ui->serialPortInfoBox->currentText());
    ui->selectFolderBtn->hide();
    close();

    objParams.insert("route",ui->roadNameEdit->text());
    if(!ui->pileNumEdit_km->text().isEmpty() && !ui->pileNumEdit_m->text().isEmpty())
    {
        objParams.insert("pileNum",QString::number(ui->pileNumEdit_km->text().toInt() + 0.001*ui->pileNumEdit_m->text().toInt()) + "km");
    }
    emit sig_sendParams(objParams);
}

/**
 * @brief slt_selectFolderBtnClicked 点击了选择文件夹按钮
 */
void FormSetting::slt_selectFolderBtnClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(nullptr,("选择"),"");
    ui->savePathEdit->setText(folderPath);
}

void FormSetting::slt_prepareBtnClicked()
{
    if(!isEnabled())
        return;

    this->setEnabled(false);
    //参数信息保存到数据库
    QJsonObject objParams;
    objParams.insert("roadName",ui->roadNameEdit->text());//路名
    objParams.insert("pileNum",QString("%1km").arg(ui->pileNumEdit_km->text().toInt()+0.001*ui->pileNumEdit_m->text().toInt()));//桩号
    objParams.insert("upAndDown",ui->upAndDownBox->currentText());//上下行
    objParams.insert("technicalLevel",ui->buttonGroup->button(ui->buttonGroup->checkedId())->text());//技术等级
    objParams.insert("lane",ui->laneEdit->text());//车道
    objParams.insert("optPerson",ui->optPersonEdit->text());//执行人
    objParams.insert("optUnit",ui->optUnitEdit->text());//执行单位
    objParams.insert("savePath",ui->savePathEdit->text());//存储路径
    objParams.insert("camera0Flag",ui->camera0_checkbox->isChecked());//是否开启当前设备
    objParams.insert("camera1Flag",ui->camera1_checkbox->isChecked());//是否开启当前设备
    objParams.insert("camera3Flag",ui->camera3_checkbox->isChecked());//是否开启当前设备
    objParams.insert("flatnessTesterFlag",ui->flatnessTester_checkbox->isChecked());//是否开启当前设备
    objParams.insert("accelerometerFlag",ui->accelerometer_checkbox->isChecked());//是否开启当前设备
    objParams.insert("gpsFlag",ui->gps_checkbox->isChecked());//是否开启当前设备
    AppEventBase::getInstance()->sig_requestServer(SAVE_PARAMS,objParams);
}

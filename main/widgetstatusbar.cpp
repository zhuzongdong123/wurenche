#include "widgetstatusbar.h"
#include "ui_widgetstatusbar.h"
#include "appeventbase.h"
#include "appcommonbase.h"
#include "appconfigbase.h"
#include <QTimer>
#include <QtMath>
#include <QDir>
#include <QFileInfo>
#include "windows.h"
#include <QStyleOption>
#include <QPainter>
//求磁盘剩余空间
double getDiskFreeSpace(QString driver)
{
    QString strDiver;
    LPCWSTR lpcwstrDriver=(LPCWSTR)driver.utf16();

        ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;

        if( !GetDiskFreeSpaceEx( lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes) )
        {
            qDebug() << "ERROR: Call to GetDiskFreeSpaceEx() failed.";
            return 0;
        }
        return (double)liTotalFreeBytes.QuadPart/1024/1024/1024;
}

WidgetStatusBar::WidgetStatusBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetStatusBar)
{
    ui->setupUi(this);
    getBtnWidget()->setVisible(false);
    connect(AppEventBase::getInstance(), &AppEventBase::sig_sendDevStatus, this, &WidgetStatusBar::slt_setDevStatus);
    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,[=](){

        ui->gpsBtnL->setVisible(ui->gpsBtn->isVisible());
        ui->camera0L->setVisible(ui->camera0->isVisible());
        ui->camera3L->setVisible(ui->camera3->isVisible());
        ui->camera1L->setVisible(ui->camera1->isVisible());
        ui->flatnessTesterL->setVisible(ui->flatnessTester->isVisible());


            QJsonObject gpsObj =  AppCommonBase::getInstance()->getLastGPSPoint();
            int pulse_value = gpsObj.value("pulse_value").toInt();

            //设置当前速度（即1米每秒等于3.6千米每小时）
            int sp = gpsObj.value("speed").toDouble()*3.6;
            ui->speed->setText(QString("%1km/h").arg(sp));

            //倒车的时候，里程和桩号无视
            if(pulse_value > pre_pulse_value)
            {
                //获取里程(mm)
                double distance = pulse_value/4;
                distance = distance/1000;
                ui->mileage->setText(QString::number(distance/1000,'f',3)  + "km");
//                if(distance < 1000)
//                {
//                    ui->mileage->setText(QString::number(distance,'f',0)  + "m");
//                }
//                else
//                {
//                    int kmValue = int(distance)/1000;
//                    ui->mileage->setText(QString::number(kmValue+0.001*(int(distance)%1000))  + "km");
//                }

                //桩号重置
                double startMileage;
                if(m_isAddDistance)
                {
                    startMileage =  m_startMileage + (distance-m_startDistance);
                }
                else
                {
                    startMileage =  m_startMileage - (distance-m_startDistance);
                }

                ui->pileNum->setText(QString::number(startMileage/1000,'f',3)  + "km");
//                if(startMileage < 1000)
//                {
//                    ui->pileNum->setText(QString::number(0.001*startMileage)  + "km");
//                }
//                else
//                {
//                    int kmValue = int(startMileage)/1000;
//                    ui->pileNum->setText(QString::number(kmValue+0.001*(int(startMileage)%1000))  + "km");
//                }

                //当前桩号
                AppCommonBase::getInstance()->g_currentStation = startMileage/1000;

                if(fabs(distance-m_preSendValue) >= 100)
                {
                    AppEventBase::getInstance()->sig_updateMileage(distance);
                    m_preSendValue = distance;
                }
            }
            pre_pulse_value = pulse_value;


            //显示当前行频
            if(!AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","LineRateTimeout","").isEmpty())
            {
                ui->radius->setText(QString("%1").arg(AppCommonBase::getInstance()->m_resultingTriggerLineRate));
            }
            else
            {
                ui->label_9->hide();
                ui->radius->hide();
            }

            //获取剩余磁盘空间
            QString filePath = m_savePath;
            if(!filePath.isEmpty() && filePath.split(":").size() > 0)
            {
                filePath = filePath.split(":")[0] + ":/";
                int freeSpace =getDiskFreeSpace(filePath);
                ui->freeDisk->setText(QString::number(freeSpace) + "G");
            }
            else
            {
                ui->freeDisk->setText("--");
            }
        });
    //刷新一次
    timer->start(100);

    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendCH120CacheCount,[=](int count){
            ui->cacheCount_CH120->setText(QString::number(count));
        });

    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendCL042CacheCount_L,[=](int count){
            m_cacheCount_CL042_L = count;

            QString showText = QString::number(count);
            if(AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera)
            {
                showText = QString::number(m_cacheCount_CL042_L) + "+" + QString::number(m_cacheCount_CL042_R);
            }

            if(showText != ui->cacheCount_CL042->text())
                ui->cacheCount_CL042->setText(showText);
        });
    connect(AppEventBase::getInstance(),&AppEventBase::sig_sendCL042CacheCount_R,[=](int count){
            m_cacheCount_CL042_R = count;
            QString showText = QString::number(count);
            if(AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera)
            {
                showText = QString::number(m_cacheCount_CL042_L) + "+" + QString::number(m_cacheCount_CL042_R);
            }

            if(showText != ui->cacheCount_CL042->text())
                ui->cacheCount_CL042->setText(showText);
        });
}

WidgetStatusBar::~WidgetStatusBar()
{
    delete ui;
}

void WidgetStatusBar::setStartRecord()
{
    int kmValue = int(m_startMileage)/1000;
    ui->pileNum->setText(QString::number(double(m_startMileage)/1000,'f',3)  + "km");
    m_preSendValue = 0;
}

QToolButton *WidgetStatusBar::getCamera0()
{
    return ui->camera0;
}

QToolButton *WidgetStatusBar::getCamera3()
{
    return ui->camera3;
}

QToolButton *WidgetStatusBar::getCamera1()
{
    return ui->camera1;
}

QToolButton *WidgetStatusBar::getGps()
{
    return ui->gpsBtn;
}

QToolButton *WidgetStatusBar::getFlatnessTester()
{
    return ui->flatnessTester;
}

QToolButton *WidgetStatusBar::getAccelerometer()
{
    return ui->accelerometer;
}

QWidget *WidgetStatusBar::getBtnWidget()
{
    return ui->widget;
}

void WidgetStatusBar::setStartMileageAndDistance(int startMileage, double startDistance)
{
    m_startMileage = startMileage;
    m_startDistance = startDistance*1000;
}

void WidgetStatusBar::slt_setGPSData(QJsonObject obj)
{
    //保存路径
    m_savePath = obj.value("savePath").toString();

    //速度
    if(obj.value("speed").toVariant().isValid())
    {
        //ui->speed->setText(QString::number(obj.value("speed").toInt()) + "km/h");
    }

    //桩号
    if(obj.value("startMileage").toVariant().isValid())
    {
        m_startMileage = obj.value("startMileage").toInt();
        //int kmValue = int(m_startMileage)/1000;
        ui->pileNum->setText(QString::number(double(m_startMileage)/1000,'f',3)  + "km");
    }

    //路线
    if(obj.value("route").toVariant().isValid())
    {
        ui->route->setText(obj.value("route").toString());
    }

    if(obj.value("upAndDown").toVariant().isValid())
    {
        if("上行" == obj.value("upAndDown").toString())
        {
            m_isAddDistance = true;
        }
        else
        {
            m_isAddDistance = false;
        }
    }


}

void WidgetStatusBar::slt_setDevStatus(int type, bool status)
{
    if(DEV_TYPE::GPS == type)
    {
        ui->gpsBtn->setProperty("status",status);
        ui->gpsBtn->setStyleSheet(ui->gpsBtn->styleSheet());
    }
    else if(DEV_TYPE::CAMERA0 == type)
    {
        ui->camera0->setProperty("status",status);
        ui->camera0->setStyleSheet(ui->camera0->styleSheet());
    }
    else if(DEV_TYPE::CAMERA3 == type)
    {
        ui->camera3->setProperty("status",status);
        ui->camera3->setStyleSheet(ui->camera3->styleSheet());
    }
    else if(DEV_TYPE::CAMERA1 == type)
    {
        ui->camera1->setProperty("status",status);
        ui->camera1->setStyleSheet(ui->camera1->styleSheet());
    }
    else if(DEV_TYPE::accelerometer == type)
    {
        ui->accelerometer->setProperty("status",status);
        ui->accelerometer->setStyleSheet(ui->accelerometer->styleSheet());
    }
    else if(DEV_TYPE::flatnessTester == type)
    {
        ui->flatnessTester->setProperty("status",status);
        ui->flatnessTester->setStyleSheet(ui->flatnessTester->styleSheet());
    }
}

void WidgetStatusBar::paintEvent(QPaintEvent *event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
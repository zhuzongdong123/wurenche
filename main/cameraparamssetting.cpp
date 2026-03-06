#include "cameraparamssetting.h"
#include "ui_cameraparamssetting.h"
#include <QStyleOption>
#include <QPainter>

CameraParamsSetting::CameraParamsSetting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraParamsSetting)
{
    ui->setupUi(this);

    //曝光时间调节
    connect(ui->exposureTime_slider,&QSlider::sliderMoved,this,[=](int value){
        if(0 == ui->exposureTime_slider->maximum())
            return;
        if(ui->exposureTime_spinBox->value() != value)
            ui->exposureTime_spinBox->setValue(value);
    });
    connect(ui->exposureTime_spinBox,&QSpinBox::textChanged,this,[=](QString value){
        if(0 == ui->exposureTime_spinBox->maximum())
            return;

        ui->exposureTime_slider->setValue(value.toInt());
        emit sig_exposureTimeValueChanged(value.toInt());
    });

    //增益调节
    connect(ui->gain_slider,&QSlider::sliderMoved,this,[=](int value){
        if(0 == ui->gain_slider->maximum())
            return;

        ui->gain_spinBox->setValue(double(value)*ui->gain_spinBox->maximum()/ui->gain_slider->maximum());
    });
    connect(ui->gain_spinBox,&QDoubleSpinBox::textChanged,this,[=](QString value){
        if(0 == ui->gain_spinBox->maximum())
            return;

        ui->gain_slider->setValue(value.toDouble()*ui->gain_slider->maximum()/ui->gain_spinBox->maximum());
        emit sig_gainValueChanged(ui->gain_spinBox->value());
    });

//    //伽马调节
//    connect(ui->gamma_slider,&QSlider::sliderMoved,this,[=](int value){
//        if(0 == ui->gamma_slider->maximum())
//            return;

//        ui->gamma_spinBox->setValue(double(value)*ui->gamma_spinBox->maximum()/ui->gamma_slider->maximum());
//    });
//    connect(ui->gamma_spinBox,&QDoubleSpinBox::textChanged,this,[=](QString value){
//        if(0 == ui->gamma_spinBox->maximum())
//            return;

//        ui->gamma_slider->setValue(value.toDouble()*ui->gamma_slider->maximum()/ui->gamma_spinBox->maximum());
//        emit sig_gammaValueChanged(ui->gamma_spinBox->value());
//    });

    this->setAttribute(Qt::WA_TranslucentBackground, true);
    this->setWindowFlags(Qt::WindowStaysOnTopHint| Qt::FramelessWindowHint|Qt::Dialog);
}

CameraParamsSetting::~CameraParamsSetting()
{
    delete ui;
}

void CameraParamsSetting::setExposureTimeMaxValue(int maxValue)
{
    ui->exposureTime_slider->setMaximum(maxValue);
    ui->exposureTime_spinBox->setMaximum(maxValue);
}

void CameraParamsSetting::slt_setParams(int exposureTime, double gainValue, double gammaValue)
{
    ui->exposureTime_spinBox->setValue(exposureTime);
    ui->gain_spinBox->setValue(gainValue);
    //ui->gamma_spinBox->setValue(gammaValue);
}

void CameraParamsSetting::slt_exposureTimeChanged(int cur, int min, int max)
{
    if(max < min || min < 15)
        return;

    ui->exposureTime_slider->setMinimum(min);
    ui->exposureTime_slider->setMaximum(max);
    ui->exposureTime_slider->setValue(cur);

    ui->exposureTime_spinBox->setMinimum(min);
    ui->exposureTime_spinBox->setMaximum(max);
    ui->exposureTime_spinBox->setValue(cur);
}

void CameraParamsSetting::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.fillRect(rect(),  QColor(0, 0, 0, 30)); // 半透明黑色
}

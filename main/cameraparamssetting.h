#ifndef CAMERAPARAMSSETTING_H
#define CAMERAPARAMSSETTING_H

#include <QWidget>
#include <QProxyStyle>

namespace Ui {
class CameraParamsSetting;
}

class CameraParamsSetting : public QWidget
{
    Q_OBJECT

public:
    explicit CameraParamsSetting(QWidget *parent = nullptr);
    ~CameraParamsSetting();
    void setExposureTimeMaxValue(int maxValue);

public slots:
    void slt_setParams(int exposureTime,double gainValue,double gammaValue);
    void slt_exposureTimeChanged(int cur, int min, int max);

signals:
    void sig_exposureTimeValueChanged(int value);
    void sig_gainValueChanged(double value);
    void sig_gammaValueChanged(double value);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::CameraParamsSetting *ui;
};

#endif // CAMERAPARAMSSETTING_H

#ifndef WIDGETSTATUSBAR_H
#define WIDGETSTATUSBAR_H

#include <QWidget>
#include <QJsonObject>
#include <QToolButton>
namespace Ui {
class WidgetStatusBar;
}

class WidgetStatusBar : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetStatusBar(QWidget *parent = nullptr);
    ~WidgetStatusBar();
    void setStartRecord();

    QToolButton* getCamera0();
    QToolButton* getCamera1();
    QToolButton *getCamera3();
    QToolButton* getGps();
    QToolButton* getFlatnessTester();
    QToolButton* getAccelerometer();
    QWidget* getBtnWidget();

    //设置起始里程和距离
    void setStartMileageAndDistance(int startMileage,double startDistance);

public slots:
    void slt_setGPSData(QJsonObject obj);
    void slt_setDevStatus(int type, bool status);

protected:
    void paintEvent(QPaintEvent *event);

private:
    Ui::WidgetStatusBar *ui;
    int m_startMileage = 0;//单位是米
    int m_startDistance = 0;//单位是米
    int m_preSendValue = 0;
    bool m_isAddDistance = true;
    int m_cacheCount_CL042_L = 0;
    int m_cacheCount_CL042_R = 0;
    QString m_savePath;
    int pre_pulse_value = 0;
};

#endif // WIDGETSTATUSBAR_H

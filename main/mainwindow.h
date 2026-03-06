/*
 * @file mainwindow.h
 * @brief 主界面：包含菜单栏和工作区
 * @author 朱宗冬
 * @date 2024-04-06
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "formsetting.h"
#include "formpilenumupdate.h"
#include "formeventmark.h"
#include "paramsdao.h"
#include "flatnesstester.h"
#include "accelerometer.h"
#include <QThread>
#include "dbthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent*event);
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void sig_mainWindowQuit();

private slots:
    /**
     * @brief slt_prepareBtnClicked 准备按钮点击事件
     */
    void slt_prepareBtnClicked();

    /**
     * @brief slt_startSaveBtnClicked 开始存储按钮点击事件
     */
    void slt_startSaveBtnClicked();

    /**
     * @brief slt_stopSaveBtnClicked 停止存储按钮点击事件
     */
    void slt_stopSaveBtnClicked();

    /**
     * @brief slt_paramsSaveSuccessed 参数保存成功
     */
    void slt_paramsSaveSuccessed(QJsonObject paramsObj);

    /**
     * @brief slt_openCamera0 打开道路相机
     */
    void slt_openCamera0(bool isChecked);
    void slt_Camera0Clicked();
    void slt_cameraDoubleClicked();

    /**
     * @brief slt_openCamera1 打开景观相机
     */
    void slt_openCamera1(bool isChecked);
    void slt_Camera1Clicked();

    /**
     * @brief slt_openFlatnessTester 打开激光平整度仪
     */
    void slt_openFlatnessTester(bool isChecked);

    /**
     * @brief slt_openFlatnessTester 打开加速度计
     */
    void slt_openAccelerometer(bool isChecked);

    /**
     * @brief slt_openGPS 打开GPS
     */
    void slt_openGPS(bool isChecked);

    /**
     * @brief slt_updateMileage 更新里程(每100米触发一次)
     */
    void slt_updateMileage(double value);

    void slt_roadEventClicked();//路面类型的事件触发
    void slt_notroadEventClicked();//非路面类型的事件触发
    void slt_pileNumUpdated(QString url, QJsonObject obj);
    void slt_Camera3Clicked();
private:
    Ui::MainWindow *ui;
    FormSetting m_settingDialog;//配置弹窗
    FormPileNumUpdate m_formPileNumUpdate;//桩号修正
    FormEventMark m_formEventMark;//时间标注
    //ParamsDao* m_dao = nullptr;
    //QThread* m_thread = nullptr;
    DBThread* dbthread = nullptr;
    FlatnessTester m_flatnessTester;//平整度仪的控制类
    Accelerometer m_accelerometer;//加速度计的控制类
    bool m_isStartSave = false;

private:
    /**
     * @brief createConnect 创建信号槽连接
     */
    void createConnect();

    /**
     * @brief init 类的初始化
     */
    void init();

    //修改文件的名称
    void changeFilesCompany();

    void outputRecord();

    void updateLayout();
};
#endif // MAINWINDOW_H

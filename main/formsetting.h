/**
 * @file formsetting.h
 * @brief 程序的设置弹窗。保存基础信息
 * @author 朱宗冬
 * @date 2024-04-06
 */
#ifndef FORMSETTING_H
#define FORMSETTING_H

#include <QWidget>
#include "httpclient.h"
#include <QJsonObject>

namespace Ui {
class FormSetting;
}

class FormSetting : public QWidget
{
    Q_OBJECT

public:
    explicit FormSetting(QWidget *parent = nullptr);
    ~FormSetting();

    /**
     * @brief getGpsPort 获取GPS监听的端口号
     * @return  返回值(return): QString 端口号
     */
    QString getGpsPort();

    /**
     * @brief getSaveFolderPath 获取存储文件夹路径
     * @return  返回值(return): QString 文件夹路径
     */
    QString getSaveFolderPath();

    /**
     * @brief getStartMileage 获取初始里程
     * @return  返回值(return): int 初始里程，单位米
     */
    int getStartMileage();

signals:
    void sig_sendParams(QJsonObject obj);

private:
    Ui::FormSetting *ui;
    HttpClient m_httpClient;//后端请求交互类

private:
    /**
     * @brief init 类的初始化
     */
    void init();

    /**
     * @brief createConnect 创建信号槽连接
     */
    void createConnect();

    /**
     * @brief checkParams 参数校验(例如：必输项校验等)
     * @return  返回值(return): bool true:成功 false:失败
     */
    bool checkParams();

public slots:
    /**
     * @brief slt_saveBtnClicked 点击了保存按钮
     */
    void slt_saveBtnClicked();

    /**
     * @brief slt_selectFolderBtnClicked 点击了选择文件夹按钮
     */
    void slt_selectFolderBtnClicked();

    /**
     * @brief slt_prepareBtnClicked 点击了准备菜单按钮
     */
    void slt_prepareBtnClicked();
};

#endif // FORMSETTING_H

/*
 * @file paramsdao.h
 * @brief 参数相关的数据库操作
 * @author 朱宗冬
 * @date 2024-04-04
 */
#ifndef PARAMSDAO_H
#define PARAMSDAO_H
#include "mysqlite.h"
#include <QJsonObject>
#include <QObject>

class ParamsDao : public  QObject
{
public:
    ParamsDao();

    /**
     * @brief createAllTable 创建所有的数据库表格
     */
    void createAllTable();

    /**
    * @brief saveParams 添加参数
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool saveParams(QJsonObject& obj);

    /**
    * @brief saveGPSInfo 保存gps信息
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool saveGPSInfo_GPFPD(QJsonObject& obj);

    /**
    * @brief saveEventMarker 保存事件标注信息
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool saveEventMarker(QJsonObject& obj);

    /**
    * @brief savePileNum 校对桩号
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool savePileNum(QJsonObject& obj);

    /**
    * @brief saveAccelerometer 保存加速度的值
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool saveAccelerometer(QJsonObject& obj);

    /**
    * @brief saveCameraInfo 保存相机信息
    * @param 入参(in): QJsonObject obj 参数
    * @return  返回值(return): bool 执行结果 true:成功 false:失败
    */
    bool saveCameraInfo(QJsonObject& obj);

    bool saveGPSInfo_GPPOSDMI(QJsonObject &obj);
    bool saveGPSInfo_GPGGA(QJsonObject &obj);
    bool saveGPSInfo_GPRMC(QJsonObject &obj);
    bool searchGPSInfo_GPPOSDMI(QJsonObject &obj);
    bool saveDataIRI(QJsonObject &obj);
    bool saveDataLocation(QJsonObject &obj);
    bool saveLog(QJsonObject &obj);
public slots:
    void slt_rcvData(QString url, QJsonObject obj);

private:
    MySqlite m_sqlite;
};

#endif // PARAMSDAO_H

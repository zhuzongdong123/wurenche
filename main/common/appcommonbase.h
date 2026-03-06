/**
 * @file appcommonbase.h
 * @brief 公共数据维护单例类基类
 * 定义公共变量、函数、结构体、枚举
 * @author 朱宗冬
 * @date 2023-09-21
 */
#ifndef APPCOMMONBASE_H
#define APPCOMMONBASE_H

#include <QObject>
#include <QDebug>
#include <QJsonObject>

/*数据请求过程中url定义*/
//保存设置弹窗的参数信息
#define SAVE_PARAMS "/controller/params/save"
//更新桩号
#define UPDATE_PILENUM "/controller/pileNum/update"
//事件标注
#define SAVE_EVENT_MARK "/controller/eventMarker/save"
//查询gppostdmi1中type=1的数据个数
#define SLECT_GPPOSTMI "/controller/GPS/GPGGA/select"
//保存GPS信息
#define SAVE_GPS_INFO_GPFPD "/controller/GPS/GPFPD/save"
#define SAVE_GPS_INFO_GPPOSTMI "/controller/GPS/GPPOSDMI/save"
#define SAVE_GPS_INFO_GPGGA "/controller/GPS/GPGGA/save"
#define SAVE_GPS_INFO_GPRMC "/controller/GPS/GPRMC/save"
//保存加速度
#define SAVE_ACCELEROMETER "/controller/Accelerometer/save"
//保存平整度仪
#define SAVE_DATALOCAITON "/controller/dataLocation/save"
#define SAVE_DATAIRI "/controller/dataIRI/save"
//保存日志
#define SAVE_LOG "/controller/log/save"
//保存相机信息
#define SAVE_CAMERA_INFO "/controller/cameraImage/save"

class AppCommonBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief getInstance 获取单例类的实例
     * @return AppCommonBase* 返回类的实例指针
     */
    static AppCommonBase* getInstance();
    void setLastGPSPoint(QJsonObject obj);
    QJsonObject getLastGPSPoint();
    bool g_rodeCameraStart = false;//道路(线阵)相机
    bool g_landscapeCameraStart = false;//景观(面阵)相机
    bool bIsGPSReset = false;//gps是否重置成功
    QString g_roadType = "a";//_a表示沥青路、_c表示水泥路、_g表示砂石路
    double g_currentStation = 0.0;//当前桩号

    QList<int> m_rodeCameraList;
    QList<int> m_landscapeCameraList;

    //log日志输出路径
    QString g_logFolder;

    //触发行频
    int m_triggerLineRate = 0;
    //理论推荐触发行频
    int m_resultingTriggerLineRate = 0;
    bool m_isDoubleLinearArrayCamera = false;

private:
    QString m_sUserId;//当前指控端的唯一标识
    QJsonObject m_lastGPSPoint;
};



#endif // APPCOMMONBASE_H

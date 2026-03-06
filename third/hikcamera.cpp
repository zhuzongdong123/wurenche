#include "hikcamera.h"
#include <QDateTime>
#include <QDir>
#include <QJsonObject>
#include "appcommonbase.h"
#include "appconfigbase.h"
#include "appeventbase.h"
#include "widgetmappage.h"
#include <QThread>
#include <QApplication>
#include <QtConcurrent>
#include <QtMath>

void __stdcall __imageCallBackEx(unsigned char *pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    HikCamera *hik = (HikCamera*)pUser;
    if (pFrameInfo == NULL || pData == NULL || pUser == NULL)
    {
        qDebug() << "错误000000000000000000000000000" << pFrameInfo << pData << pUser;
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机图片数据有误，不处理"));
        return;
    }
    hik->onImageCallback(pData, pFrameInfo);
}
void __stdcall __eventCallBackEx(MV_EVENT_OUT_INFO * pEventInfo, void* pUser)
{
    HikCamera *hik = (HikCamera*)pUser;
    if (pEventInfo == NULL || pUser == NULL)
    {
        return;
    }
    hik->outPutEventText(pEventInfo->EventName, pEventInfo->pEventData,pEventInfo->nEventDataSize);
}

void __stdcall __allEventCallBack(MV_EVENT_OUT_INFO * pEventInfo, void* pUser)
{
    HikCamera *hik = (HikCamera*)pUser;
    if (pEventInfo == NULL || pUser == NULL)
    {
        return;
    }
    hik->outPutAllEventText(pEventInfo);
}

HikCamera::HikCamera(QObject *parent)
    : QObject{parent}
{
    this->fpsTimer = new QTimer(this);
    this->fpsTimer->setInterval(1000);
    this->fpsTimer->setSingleShot(false);
    connect(this->fpsTimer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()));
    this->refreshCamera();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &HikCamera::slt_saveDataToServer);
    timer->start(3000);

    QTimer *timerGetCache = new QTimer(this);
    connect(timerGetCache, &QTimer::timeout,this,[=](){
        //获取录像的缓存数量
        int count = 0;
        for(int i = 0; i < m_threadList.size(); i++)
        {
            count += m_threadList[i]->getCacheCount();
        }

        if(m_cameraType == CAMERA_TYPE::CH120)
        {
            emit AppEventBase::getInstance()->sig_sendCH120CacheCount(count);
        }
        else if(m_cameraType == CAMERA_TYPE::CL042)
        {
            emit AppEventBase::getInstance()->sig_sendCL042CacheCount_L(count);
        }
        else if(m_cameraType == CAMERA_TYPE::CL042_2)
        {
            emit AppEventBase::getInstance()->sig_sendCL042CacheCount_R(count);
        }
    });
    timerGetCache->start(1000);


    QTimer *timerGetRate = new QTimer(this);
    connect(timerGetRate, &QTimer::timeout,this,[=](){
        if(NULL == this->m_camHandle || m_isCloseRecord)
            return;

        //线阵相机获取触发行频和理论推荐触发行频
        if(CAMERA_TYPE::CL042 == m_cameraType/* || CAMERA_TYPE::CL042_2 == m_cameraType*/)
        {
            int nRet = MV_OK;
            MVCC_INTVALUE value;
            nRet = MV_CC_GetIntValue(this->m_camHandle, "TriggerLineRate", &value);
            if (MV_OK != nRet)
            {
                qDebug() << "Get TriggerLineRate fail!" << nRet;
            }
            else
            {
                AppCommonBase::getInstance()->m_triggerLineRate = value.nCurValue;
            }

            nRet = MV_CC_GetIntValue(this->m_camHandle, "ResultingTriggerLineRate", &value);
            if (MV_OK != nRet)
            {
                qDebug() << "Get ResultingTriggerLineRate fail!" << nRet;
            }
            else
            {
                AppCommonBase::getInstance()->m_resultingTriggerLineRate = value.nCurValue;
            }
        }
    });
    //需要监控触发行频
    if(!AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","LineRateTimeout","").isEmpty())
    {
        int LineRateTimeout = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","LineRateTimeout","1000").toInt();
        timerGetRate->start(LineRateTimeout);
    }

    //清空相机缓存
    connect(AppEventBase::getInstance(), &AppEventBase::sig_mapResetSuccessed, this, &HikCamera::clearCameraCache);
    qDebug() << "当前主线程" << QThread::currentThread();
}

void HikCamera::setCacheCount(int count)
{
    //存储录像的缓存线程数量
    if(m_threadList.size() > 0)
        return;

    //int threadCount = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","cacheThreadCount","5").toInt();
    for(int i = 0;  i < count; i++)
    {
        m_threadList.append(new ImageSaveThread(this));
    }
}

HikCamera::~HikCamera()
{
    if(this->m_isGrabbing)
        this->stopGrab();
    this->freeCamera();
    //停止存储线程
    for(int i = 0; i < m_threadList.size(); i++)
    {
        m_threadList[i]->stopThread();
    }
}
QList<QString> HikCamera::camera()
{
    return this->cameraList;
}

void HikCamera::setSaveFolderPath(QString folderPath)
{
    QString cameraName = getDevName(m_cameraType);
    if(!folderPath.isEmpty())
    {
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机开始存储 %1").arg(m_cameraLogName));
    }

    m_saveFolderPath = folderPath;

    //文件夹不存在则创建(可创建多级目录)
    QString path = folderPath + "/" + cameraName;
    QDir dir(path);
    if(!dir.exists() && !folderPath.isEmpty())
    {
        dir.mkpath(path);
    }
}

bool HikCamera::isOpen()
{
    return m_isOpen;
}
void HikCamera::onTimerTimeout()
{
    if(NULL == this->m_camHandle || m_isCloseRecord)
        return;

    int nRet = MV_OK;

    MVCC_FLOATVALUE value;
    nRet = MV_CC_GetFloatValue(this->m_camHandle, "ExposureTime", &value);
    if (MV_OK != nRet)
    {
        qDebug() << "Get Exposure Time fail!" << nRet;
        return;
    }

    if(0 == m_frameCount)
        return;

    this->m_exTimeMin = (int)value.fMin;
    if(this->m_frameCount > 0)
        this->m_exTimeMax = 1000000 / this->m_frameCount;
    this->m_exTimeMax = qMin(this->m_exTimeMax, (int)value.fMax);
    if(fabs(this->m_exTimeCur-value.fCurValue) > 1e-3)
    {
        this->m_exTimeCur = value.fCurValue;
        if(m_exTimeMax > 100000)
            m_exTimeMax = 100000;
        emit this->exposureTimeChanged(this->m_exTimeCur, this->m_exTimeMin, this->m_exTimeMax);
    }
    //qDebug("Exposure Time:%f min:%d max:%d", value.fCurValue, this->m_exTimeMin, this->m_exTimeMax);

    this->m_fps = this->m_frameCount;
//    qDebug("%s fps at HikCamera is %d", this->cameraList[this->m_selectedCameraIndex].toLocal8Bit().data(),
//            this->m_frameCount);
    //qDebug() << "帧率" <<  m_frameCount;
    this->m_frameCount = 0;
}
void HikCamera::freeCamera()
{
    if(m_isOpen)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机关闭成功 %1").arg(m_cameraLogName));
    }

    int nRet;
    if(this->m_camHandle)
    {
        nRet = MV_CC_CloseDevice(this->m_camHandle);
        if (MV_OK != nRet)
        {
            //qFatal("Close Device fail! nRet [0x%x]", nRet);
            qDebug() << "句柄关闭失败";
            return;
        }
        nRet = MV_CC_DestroyHandle(this->m_camHandle);
        if (MV_OK != nRet)
        {
            //qFatal("Destroy Handle fail! nRet [0x%x]", nRet);
            qDebug() << "句柄释放失败";
            return;
        }
    }
    this->m_camHandle = NULL;
    m_isOpen = false;
    m_isGrabbing = false;
    m_isRecording = false;
}

//播放某个相机(已经打开的话，无法打开，程序崩溃，需要添加try...catch)
void HikCamera::setCamera(int index)
{
    try {
        //防止重复打开
        if(m_isOpen)
            return;

        m_cameraType = CAMERA_TYPE(index);
        if(m_cameraType == CAMERA_TYPE::CH120)
        {
            m_cameraLogName = "景观相机";
        }
        if(m_cameraType == CAMERA_TYPE::CL042)
        {
            m_cameraLogName = "破损相机(左)";
        }
        if(m_cameraType == CAMERA_TYPE::CL042_2)
        {
            m_cameraLogName = "破损相机(右)";
        }

        QString cameraName;
        bool isSearch = false;
        MV_CC_DEVICE_INFO* pDeviceInfo = getDevInfo(CAMERA_TYPE(index),isSearch);

        if(NULL == pDeviceInfo || !isSearch)
            emit AppEventBase::getInstance()->sig_sendDevStatus(m_cameraType,false);

        int nRet;
        if(NULL == pDeviceInfo || m_isOpen || !isSearch)
            return;

        this->freeCamera();

        //从搜索成功的相机列表中打开句柄
        // ch:选择设备并创建句柄 | Select device and create handle
        nRet = MV_CC_CreateHandle(&this->m_camHandle, pDeviceInfo);
        for(int i = 0; i < m_threadList.size(); i++)
        {
            void *camHandleSaveImg = NULL;
            MV_CC_CreateHandle(&camHandleSaveImg, pDeviceInfo);
            m_threadList[i]->setCamHandle(camHandleSaveImg);
            m_threadList[i]->setCameraName(m_cameraLogName);
        }

        if (MV_OK != nRet)
        {
            //qFatal("Create Handle fail! nRet [0x%x]", nRet);
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机创建句柄失败"));
            return;
        }
        // ch:打开设备 | Open device
        nRet = MV_CC_OpenDevice(this->m_camHandle);
        if (MV_OK != nRet)
        {
            //qFatal("Open Device fail! nRet [0x%x]", nRet);
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机打开失败 %1").arg(m_cameraLogName));
            emit AppEventBase::getInstance()->sig_sendDevStatus(m_cameraType,false);
            return;
        }
        emit AppEventBase::getInstance()->sig_sendDevStatus(m_cameraType,true);

        // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(this->m_camHandle);
            if (nPacketSize <= 0)
                qWarning("Get Packet Size fail nRet [0x%x]!", nPacketSize);
            else
            {
                nRet = MV_CC_SetIntValue(this->m_camHandle, "GevSCPSPacketSize", nPacketSize);
                if(nRet != MV_OK)
                    qWarning("Set Packet Size fail nRet [0x%x]!", nRet);
            }
        }

        //线阵相机的时候，参数设置
        if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
        {
            //6:帧触发开始 9:行开始
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerSelector",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","TriggerSelector","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发选择器'参数修改失败");

            //0:关闭 1:打开
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerMode",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","TriggerMode","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发模式'参数修改失败");

            //7:软触发 0:线路0 3:线路3 8:变频器 22:动作1 25:多路
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerSource",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","TriggerSource","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发源'参数修改失败");

            //0:上升沿 1:下降沿 2:高电平 3:低电平 4:上升或下降沿
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerActivation",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","TriggerActivation","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发极性'参数修改失败");

            //0:编码器0
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "EncoderSelector",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","EncoderSelector","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '编码器'参数修改失败");

            //0:线路0 3:线路3 128:N/A
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "EncoderSourceA",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","EncoderSourceA","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '编码器源A'参数修改失败");

            //0:线路0 3:线路3 128:N/A
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "EncoderSourceB",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","EncoderSourceB","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '编码器源B'参数修改失败");

            //0:任意方向 1:仅正方向 2:仅反方向
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "EncoderOutputMode",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","EncoderOutputMode","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '编码器输出模式'参数修改失败");

            //0:忽略方向 1:遵循方向 2:反方向
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "EncoderCounterMode",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","EncoderCounterMode","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '编码器计数器模式'参数修改失败");

            //0:线路0 3:线路3 128:N/A 7:编码器模块输出
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "InputSource",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","InputSource","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '输入源'参数修改失败");

            //Min: 1 Max: 128
            if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "PreDivider",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","PreDivider","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '预分频器'参数修改失败");

            //Min: 1 Max: 32
            if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "Multiplier",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","Multiplier","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '乘法器'参数修改失败");

            //Min: 1 Max: 32
            if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "PostDivider",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","PostDivider","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '后分频器'参数修改失败");

            //0:线路0 1:线路1 3:线路3 4:线路4
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "LineSelector",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","LineSelector","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '线路选择器'参数修改失败");

            //0:输入 8:频闪输出
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "LineMode",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","LineMode","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '线路模式'参数修改失败");

            //0:Line Mode 1:Frame Mode
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "StrobeSourceSelector",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","StrobeSourceSelector","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " 'Strobe Source Selector'参数修改失败");

            //0:关 1开
            if(MV_OK != MV_CC_SetBoolValue(this->m_camHandle, "StrobeEnable",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","StrobeEnable","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '输出使能'参数修改失败");

            //TDIMode默认设置为2
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TDIMode",AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","TDIMode","1").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " 'TDIMode'参数修改失败");

            //Min: 0 Max: 50000
            if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "StrobeLineDuration",
                                           AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","StrobeLineDuration","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '输出线路持续时间'参数修改失败");

            //设置缓存节点个数
            if(!AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","ImageNodeNum","").isEmpty() && MV_OK != MV_CC_SetImageNodeNum(this->m_camHandle,
                                              AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","ImageNodeNum","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '缓存节点'参数修改失败");

            //水平镜像
            if(MV_OK != MV_CC_SetBoolValue(this->m_camHandle, "ReverseX",AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","ReverseX","1").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '水平镜像'参数修改失败");

            //线阵相机高度
            if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "Height",AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","Height","2000").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '高度'参数修改失败");

            //左相机
            if(CAMERA_TYPE::CL042 == m_cameraType)
            {
                //线阵相机宽度
                if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "Width",AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L_width","2048").toInt()))
                    AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " 左相机'宽度'参数修改失败");

//                //线阵相机横向偏移
//                if(!AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L_OffsetX","").isEmpty() &&
//                        MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "OffsetX",AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L_OffsetX","").toInt()))
//                    AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + " 左相机'偏移'参数修改失败");

            }
            //右相机
            else if(CAMERA_TYPE::CL042_2 == m_cameraType)
            {
                //线阵相机宽度
                if(MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "Width",AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_width","2048").toInt()))
                    AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " 右相机'宽度'参数修改失败");

//                //线阵相机横向偏移
//                if(!AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_OffsetX","").isEmpty() &&
//                        MV_OK != MV_CC_SetIntValueEx(this->m_camHandle, "OffsetX",AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R_OffsetX","").toInt()))
//                    AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + " 右相机'偏移'参数修改失败");
            }

            //注册监听所有的事件回调
            if(!AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","RegisterAllEvent","").isEmpty() &&
                    AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","RegisterAllEvent","").toInt())
            {
                qDebug() << "注册全部事件回调--------------";
                MV_CC_RegisterAllEventCallBack(m_camHandle, __allEventCallBack, this);
            }

            //是否检验帧号
            m_isCheckFrameNum = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","CheckFrameNum","0").toInt();
            m_imageQuality = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","imageQuality","60").toInt();

            qDebug() << getDevName(m_cameraType) << "setPixelFormat:" << HIKCAM_FMT_CL042;
            nRet = MV_CC_SetEnumValue(this->m_camHandle, "PixelFormat", HIKCAM_FMT_CL042);
            if (MV_OK != nRet)
                qWarning("Set Pixel Format fail! nRet [0x%x]", nRet);
        }
        //面阵相机
        else if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            //0:关闭 1:打开
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerMode",
                                           AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","TriggerMode","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发模式'参数修改失败");

            //7:软触发 0:线路0 3:线路3 8:变频器 22:动作1 25:多路
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerSource",
                                           AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","TriggerSource","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发源'参数修改失败");

            //0:上升沿 1:下降沿 2:高电平 3:低电平 4:上升或下降沿
            if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerActivation",
                                           AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","TriggerActivation","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发极性'参数修改失败");

            //触发使能打开
            if(MV_OK != MV_CC_SetBoolValue(this->m_camHandle, "TriggerCacheEnable",1))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发使能'参数修改失败");

            //水平镜像
            if(MV_OK != MV_CC_SetBoolValue(this->m_camHandle, "ReverseX",AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","ReverseX","1").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '水平镜像'参数修改失败");

            //垂直镜像
            if(MV_OK != MV_CC_SetBoolValue(this->m_camHandle, "ReverseY",AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","ReverseY","1").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '垂直镜像'参数修改失败");

            //设置缓存节点个数
            m_imageQuality = AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","imageQuality","99").toInt();
            if(!AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","ImageNodeNum","").isEmpty() && MV_OK != MV_CC_SetImageNodeNum(this->m_camHandle,
                                              AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","ImageNodeNum","").toInt()))
                AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '缓存节点'参数修改失败");
            qDebug() << getDevName(m_cameraType) << "setPixelFormat:" << HIKCAM_FMT_CH120;
            nRet = MV_CC_SetEnumValue(this->m_camHandle, "PixelFormat", HIKCAM_FMT_CH120);
            if (MV_OK != nRet)
                qWarning("Set Pixel Format fail! nRet [0x%x]", nRet);
        }

        m_cameraName = getDevName(m_cameraType);

        //登录成功，曝光自动关闭
        setExposureAuto(false);

        //强制开启伽马使能 0:关 1开
        MV_CC_SetBoolValue(this->m_camHandle, "SuperBayerEnable",1);
       // MV_CC_SetBoolValue(this->m_camHandle, "GammaEnable",1);

        //设置默认值
        //面阵相机曝光、增益和gamma默认值为300us、6、0.6
        if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            MV_CC_SetFloatValue(this->m_camHandle, "ExposureTime", 300);
            setgainValue(6);
          //  setgammaValue(0.6);
        }
        //2.线阵相机设置曝光、增益和gamma默认值为30us、12、0.7
        else if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
        {
            MV_CC_SetFloatValue(this->m_camHandle, "ExposureTime", 30);
            if(AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera)
            {
                MV_CC_SetBoolValue(this->m_camHandle, "DigitalShiftEnable",1);
                setgainValue(2);
            }
            else
            {
                setgainValue(23.98);
            }
          //  setgammaValue(0.7);
        }

        //向外界抛送当前的相机参数
        MVCC_FLOATVALUE exposureTime;
        MVCC_FLOATVALUE gainValue;
        MVCC_FLOATVALUE gammaValue;
        MV_CC_GetFloatValue(this->m_camHandle, "ExposureTime",&exposureTime);
        if(AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera)
            MV_CC_GetFloatValue(this->m_camHandle, "DigitalShift",&gainValue);
        else
            MV_CC_GetFloatValue(this->m_camHandle, "Gain",&gainValue);
        MV_CC_GetFloatValue(this->m_camHandle, "Gamma",&gammaValue);
        emit sendParams(exposureTime.fCurValue,gainValue.fCurValue,gammaValue.fCurValue);

        //注册回调
        this->startGrab();

        qInfo("set camera %d ok!", m_cameraType);
        m_isOpen = true;
        m_maxGPSFrameNum = AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","maxGPSFrameNum","50000").toInt();
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机打开成功 %1").arg(m_cameraLogName));
        QApplication::processEvents();
    } catch (...) {
        qCritical("camera open fail!", this->m_cameraType);
    }
}
void HikCamera::refreshCamera()
{
    int nRet;

    //搜索设备，获取列表
    this->cameraList.clear();
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &this->stDeviceList);
    if (MV_OK != nRet)
    {
        qWarning("Enum Devices fail! nRet [0x%x]", nRet);
        qDebug() << "设备搜索失败";
        return;
    }
    if (this->stDeviceList.nDeviceNum == 0)
    {
        qWarning("Find No HikCamera Devices");
        qDebug() << "未找到相机设备";
        return;
    }

    qInfo("Found %d cameras", this->stDeviceList.nDeviceNum);

    for (unsigned int i = 0; i < this->stDeviceList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = this->stDeviceList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
            break;

        QString cameraName = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
        qDebug() << "相机名称: " << cameraName;
        if(this->stDeviceList.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE)
            this->cameraList.append(QString((char*)this->stDeviceList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.chModelName));
    }
}

void HikCamera::onImageCallback(unsigned char *pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{
    QDateTime startTime = QDateTime::currentDateTime();

    m_imageCount++;
    qDebug() << "---收到图片，开始处理----" << getDevName(m_cameraType) << m_imageCount  <<
                QString("Width[%1], Height[%2], nFrameNum[%3], nFrameLen[%4]").arg(pFrameInfo->nWidth).arg(pFrameInfo->nHeight).arg(pFrameInfo->nFrameNum).arg(pFrameInfo->nFrameLen)
                << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");;

    //前后两张图片的帧号
    if(-100 != m_preFrameNum && (m_preFrameNum+1) != int(pFrameInfo->nFrameNum)
            && int(pFrameInfo->nFrameNum) > m_preFrameNum)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("%1 前后帧号不相连").
                                                       arg(m_cameraLogName));
        qDebug() << "前后帧号不相连" << m_preFrameNum << int(pFrameInfo->nFrameNum);
    }
    m_preFrameNum = pFrameInfo->nFrameNum;

    if(!m_saveFolderPath.isEmpty() && AppCommonBase::getInstance()->bIsGPSReset)
    {
        MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
        memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));
        stSaveFileParam.enImageType = MV_Image_Jpeg; // ch:需要保存的图像类型 | en:Image format to save
        stSaveFileParam.nWidth = pFrameInfo->nWidth;         // ch:相机对应的宽 | en:Width
        stSaveFileParam.nHeight = pFrameInfo->nHeight;          // ch:相机对应的高 | en:Height
        stSaveFileParam.nDataLen = pFrameInfo->nFrameLen;
        stSaveFileParam.pData = pData;
        stSaveFileParam.iMethodValue = 0;
        stSaveFileParam.nQuality = m_imageQuality;//99编码质量最高
        QString cameraName = getDevName(m_cameraType);

        QJsonObject gpsObj = AppCommonBase::getInstance()->getLastGPSPoint();
        int pulsesTotalNum = 0;
        QString type = AppCommonBase::getInstance()->g_roadType;
        bool isNan = false;
        if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            stSaveFileParam.enPixelType = pFrameInfo->enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type PixelType_Gvsp_BayerRG8
            bool isFind = false;
            QTime _time = QTime::currentTime().addMSecs(5000);
            //延时500毫秒
            while(m_isRcvLine0 && QTime::currentTime() < _time)
            {
                //面阵相机的索引列表
                if(AppCommonBase::getInstance()->m_landscapeCameraList.size() > 0)
                {
                    isFind = true;
                    pulsesTotalNum = AppCommonBase::getInstance()->m_landscapeCameraList[0];
                    AppCommonBase::getInstance()->m_landscapeCameraList.pop_front();

                    if(-1 != m_prePulsesTotalNum)
                    {
                        int temp = qAbs(m_prePulsesTotalNum-pulsesTotalNum);
                        if(temp > m_maxGPSFrameNum)
                        {
                            AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("GPS帧号前后差距较大"));
                            qDebug() << "异常数据，GPS帧号前后差距较大" << "GPS上一帧：" << m_prePulsesTotalNum << "GPS当前帧：" << pulsesTotalNum << "相机编号：" << int(pFrameInfo->nFrameNum);
                        }
                    }
                    m_prePulsesTotalNum = pulsesTotalNum;
//                    int temp = qAbs(pFrameInfo->nFrameNum*20000 - pulsesTotalNum);
//                    if(temp > 10000)
//                    {
//                        qDebug() << "异常数据?" << temp << pFrameInfo->nFrameNum << pulsesTotalNum << AppCommonBase::getInstance()->m_landscapeCameraList.size();
//                        isNan = true;
//                    }
//                    else
//                    {
//                        AppCommonBase::getInstance()->m_landscapeCameraList.pop_front();
//                    }
                    break;
                }
                QApplication::processEvents(QEventLoop::AllEvents,100);
            }

            if(!isFind)
            {
                emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "面阵相机图片未存储");
            }

            type = "";//面阵相机不需要添加道路类型
            m_CH120_count++;
        }
        //线阵相机
        else if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
        {
            stSaveFileParam.enPixelType = HIKCAM_FMT_CL042;  // ch:相机对应的像素格式 | en:Camera pixel type
            m_CL042_count++;
            pulsesTotalNum = m_CL042_count;
        }

        //图片路径
        QDateTime time = QDateTime::currentDateTime();
        QString filePath;

        //面阵相机，命名规则
        if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            if(isNan)
                filePath = "/nan" +  QString("_%1").arg(m_linearArrayCamera_suffix) + QString::number(pFrameInfo->nFrameNum) + ".jpg";
            else
                filePath = "/" + QString::number(pulsesTotalNum) +  QString("_%1").arg(m_linearArrayCamera_suffix) + QString::number(pFrameInfo->nFrameNum) + ".jpg";
        }
        else
        {
            filePath = "/" + type +  "_" + QString::number(pulsesTotalNum) +  QString("_%1").arg(m_linearArrayCamera_suffix) + QString::number(pFrameInfo->nFrameNum) + ".jpg";
        }

        //每10000个创建一个文件夹
        int count = (m_imageCount-1)/10000 + 1;
        QString savePath = m_saveFolderPath + "/" + cameraName + QString("/%1").arg(count);
        QDir dir(savePath);
        if(!dir.exists())
        {
            //不存在则创建
            dir.mkdir(savePath);
        }
        std::string path = QString(savePath  + filePath).toStdString();

        //获取到以后才储存图片
        if(0 != pulsesTotalNum)
        {
            if(m_threadList.size() > 0)
            {
                sprintf_s(stSaveFileParam.pImagePath, 256, path.c_str(), stSaveFileParam.nWidth, stSaveFileParam.nHeight, pFrameInfo->nFrameNum);
                m_threadList[m_listNum]->addFile(&stSaveFileParam);
                m_listNum++;
                if(m_listNum == m_threadList.size())
                    m_listNum = 0;
            }

            //参数信息保存到数据库
            QJsonObject objParams;
            objParams.insert("path",filePath);//图片的相对路径
            int type = int(m_cameraType);
//            if(m_cameraType == CAMERA_TYPE::CL042_2)
//                type = 0;
            objParams.insert("type",type);//相机类型
            objParams.insert("lon",gpsObj.value("lon").toDouble());//经度
            objParams.insert("lat",gpsObj.value("lat").toDouble());//纬度
            objParams.insert("locationId",gpsObj.value("id").toString());//经纬度数据的唯一标识
            objParams.insert("createTime",time.toString("yyyy-MM-dd hh:mm:ss"));//创建时间
            objParams.insert("save_path",m_saveFolderPath + "/" + filePath);//图片的绝对路径
            objParams.insert("pulse_value",pulsesTotalNum);//脉冲值
            objParams.insert("GPS_time",gpsObj.value("GPS_time").toString());//当前gps时间
            objParams.insert("heading",gpsObj.value("heading").toDouble());
            objParams.insert("pave_type",AppCommonBase::getInstance()->g_roadType);
            objParams.insert("rcv_count",m_CH120_count);//面阵相机存储开始收到的第几张图片
            m_mutex.lock();
            m_array.push_back(objParams);
            m_mutex.unlock();
        }
    }

    emit sendImageSize(pFrameInfo->nWidth, pFrameInfo->nHeight);
    emit this->dataReady(this->m_fps);
    this->m_frameCount++;

    //计算图片处理时长
    QDateTime endTime = QDateTime::currentDateTime();
    int timeL = startTime.msecsTo(endTime);
    if(timeL > 80)
    {
        qDebug() << getDevName(m_cameraType) << "本条数据处理时间过长,大于80" << timeL;
    }
    else if(timeL > 70)
    {
        qDebug() << getDevName(m_cameraType) << "本条数据处理时间过长,大于70" << timeL;
    }
    else if(timeL > 60)
    {
        qDebug() << getDevName(m_cameraType) << "本条数据处理时间过长,大于60" << timeL;
    }
    qDebug() << "---收到图片，结束处理----" << getDevName(m_cameraType) << m_imageCount  <<
                QString("Width[%1], Height[%2], nFrameNum[%3], nFrameLen[%4]").arg(pFrameInfo->nWidth).arg(pFrameInfo->nHeight).arg(pFrameInfo->nFrameNum).arg(pFrameInfo->nFrameLen)
                << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");;
}

void HikCamera::startGrab()
{
    int nRet;

    qDebug() << getDevName(m_cameraType) << "开始采集 start" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    if(this->m_camHandle == NULL)
        return;
    if(this->m_isGrabbing)
        return;
//    nRet = MV_CC_SetEnumValue(this->m_camHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_CONTINUOUS);
//    if (MV_OK != nRet)
//    {
//        qWarning("Set ExposureAuto fail! nRet [0x%x]", nRet);
//        return;
//    }

    // ch:注册抓图回调 | en:Register image callback
    nRet = MV_CC_RegisterImageCallBackEx(this->m_camHandle, __imageCallBackEx, this);
    if (MV_OK != nRet)
    {
        qWarning("Register Image CallBack fail! nRet [0x%x]", nRet);
        return;
    }

    // ch:开始取流 | en:Start grab image
    nRet = MV_CC_StartGrabbing(this->m_camHandle);
    if (MV_OK != nRet)
    {
        qWarning("Start Grabbing fail! nRet [0x%x]", nRet);
        return;
    }
    this->fpsTimer->start();
    this->m_isGrabbing = true;

    //创建线程存储池
    if(m_cameraType == CAMERA_TYPE::CL042 || CAMERA_TYPE::CL042_2 == m_cameraType)
    {
        int count = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","cacheCount","30").toInt();
        int size = AppConfigBase::getInstance()->readCameraSettings("linearArrayCameraParams","size","8192000").toInt();
        for(int i = 0; i < m_threadList.size(); i++)
        {
            m_threadList[i]->newFileParam(count,size);
        }
    }
    else
    {
        int count = AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","cacheCount","20").toInt();
        int size = AppConfigBase::getInstance()->readCameraSettings("areaArrayCameraParams","size","12288000").toInt();
        for(int i = 0; i < m_threadList.size(); i++)
        {
            m_threadList[i]->newFileParam(count,size);
        }
    }

    //显示图像
    nRet = MV_CC_Display(this->m_camHandle, m_mainWndID);
    if (MV_OK != nRet)
    {
        qDebug() << "显示图像失败" << nRet;
    }
    else
    {
        qDebug() << "显示图像成功" << nRet;
    }

    qDebug() << getDevName(m_cameraType) << "开始采集 end" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void HikCamera::regeistCallBack()
{
    if(m_isOpen && m_camHandle != NULL)
    {
        MV_CC_RegisterImageCallBackEx(this->m_camHandle, __imageCallBackEx, this);
    }
}

void HikCamera::stopGrab()
{
    int nRet;

    if(this->m_camHandle == NULL)
        return;
    if(!this->m_isGrabbing)
        return;

    this->m_isGrabbing = false;
    this->m_isRecording = false;
    this->fpsTimer->stop();
    nRet = MV_CC_StopGrabbing(this->m_camHandle);
    if (MV_OK != nRet)
    {
        qWarning("Stop Grabbing fail! nRet [0x%x]", nRet);
        return;
    }
}
void HikCamera::startRecord()
{
    if(!m_isOpen)
    {
        qDebug() << "return";
        return;
    }

    //面阵相机。点击开始存储以后，打开触发模式
    if(CAMERA_TYPE::CH120 == m_cameraType)
    {
        //0:关闭 1:打开
        if(MV_OK != MV_CC_SetEnumValue(this->m_camHandle, "TriggerMode",1))
            AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '触发模式'参数修改失败");
    }

    //监听事件
    if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
    {
        regeistEvent("StreamTransferOverflow");//相机缓存内图像被覆盖
        regeistEvent("AcquisitionTrigger");//帧触发开始
        regeistEvent("AcquisitionTriggerEnd");//帧触发结束
    }
    //监听事件
    else if(CAMERA_TYPE::CH120 == m_cameraType)
    {
        regeistEvent("FrameEnd");
        regeistEvent("FrameStart");
        regeistEvent("ExposureEnd");
        regeistEvent("Line0RisingEdge");
        regeistEvent("FrameStartOverTrigger");
        regeistEvent("OverRun",false);
        regeistEvent("StreamTransferOverflow");
    }
    for(int i = 0; i < m_threadList.size(); i++)
    {
        m_threadList[i]->start();
    }

    if(this->m_isRecording)
        return;
    if(!this->m_isGrabbing)
        this->startGrab();
    this->m_isRecording = true;

    if(!m_saveFolderPath.isEmpty())
    {
        setSaveFolderPath(m_saveFolderPath);
    }
}
void HikCamera::stopRecord()
{
    if(!m_isOpen)
    {
        qDebug() << "相机未开启成功";
        return;
    }

    if(!this->m_isRecording)
    {
        qDebug() << "相机未保存";
        return;
    }
    this->m_isRecording = false;

    if(!m_saveFolderPath.isEmpty())
    {
        QString cameraName = getDevName(m_cameraType);
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + QString("相机停止存储 %1").arg(m_cameraLogName));
        if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            QString count = QString::number(m_lineoCount_CH120);
            m_lastMsg = m_cameraLogName + ",上升沿 " + count + ",帧开始 " + QString::number(m_frameStartCount)
                    + ",帧结束 " + QString::number(m_frameEndCount);
            AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + ",上升沿 " + count + ",帧开始 " + QString::number(m_frameStartCount)
                                                            + ",帧结束 " + QString::number(m_frameEndCount));
        }
        else if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
        {
            QString count = QString::number(m_lineoCountStart_CL042);
            QString count1 = QString::number(m_lineoCountEnd_CL042);
            AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + ",触发开始 " + count + ",触发结束 " + count1);
        }
    }
}

void HikCamera::setExposureTime(int time)
{
    if(m_cameraType == CAMERA_TYPE::CH120 && (time < 15 || 0 == m_exTimeMax))
        return;

    int nRet;

    this->m_exTimeCur = time;
    nRet = MV_CC_SetFloatValue(this->m_camHandle, "ExposureTime", this->m_exTimeCur);
    if (MV_OK != nRet)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '曝光时间'参数修改失败" + QString::number(time,'f',2));
        qWarning("Set ExposureTime fail! nRet [0x%x]", nRet);
        qDebug() << m_exTimeMin << m_exTimeMax;
        return;
    }
    if(m_exTimeMax > 100000)
        m_exTimeMax = 100000;
    emit this->exposureTimeChanged(this->m_exTimeCur, this->m_exTimeMin, this->m_exTimeMax);
}
void HikCamera::setExposureAuto(bool isAuto)
{
    int nRet;

    if(isAuto)
        nRet = MV_CC_SetEnumValue(this->m_camHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_CONTINUOUS);
    else
        nRet = MV_CC_SetEnumValue(this->m_camHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);

    if (MV_OK != nRet)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '自动曝光'关闭失败");
        qWarning("Set ExposureAuto fail! nRet [0x%x]", nRet);
        return;
    }
}

void HikCamera::setgainValue(float value)
{
    if(getDevName(m_cameraType).isEmpty())
        return;

    int nRet;
    QString gainName = "Gain";
    if(AppCommonBase::getInstance()->m_isDoubleLinearArrayCamera && (m_cameraType == CAMERA_TYPE::CL042 || m_cameraType == CAMERA_TYPE::CL042_2))
        nRet = MV_CC_SetFloatValue(this->m_camHandle, "DigitalShift", value);
    else
        nRet = MV_CC_SetFloatValue(this->m_camHandle, "Gain", value);

    if (MV_OK != nRet)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '增益'参数修改失败" + QString::number(value,'f',2));
        qWarning("Set Gain fail! nRet [0x%x]", nRet);
        return;
    }
}

void HikCamera::setgammaValue(float value)
{
    return;

    int nRet;
    nRet = MV_CC_SetFloatValue(this->m_camHandle, "Gamma", value);
    if (MV_OK != nRet)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + " '伽马'参数修改失败" + QString::number(value,'f',2));
        qWarning("Set Gamma fail! nRet [0x%x]", nRet);
        return;
    }
}

void HikCamera::clearCameraCache()
{
    if(nullptr != m_camHandle && (m_cameraType == CAMERA_TYPE::CL042  || CAMERA_TYPE::CL042_2 == m_cameraType))
    {
        //清除缓存
        qDebug() << "开始清空图像缓存";
        int nRet = MV_CC_ClearImageBuffer(m_camHandle);
        if (MV_OK == nRet)
        {
            qDebug() << getDevName(m_cameraType) << "图像缓存清空成功";
        }
        else
        {
            qDebug() << getDevName(m_cameraType) << "图像缓存清空失败";
        }
    }
}

void HikCamera::slt_saveDataToServer()
{
    m_mutex.lock();
    if(0 == m_array.size())
    {
        m_mutex.unlock();
        return;
    }

    QJsonArray arrayTemp = m_array;
    QJsonArray array;
    m_array.swap(array);
    m_mutex.unlock();

    //保存结果到数据库中
    QJsonObject obj;
    obj.insert("data",arrayTemp);
    AppEventBase::getInstance()->sig_requestServer(SAVE_CAMERA_INFO,obj);
}

MV_CC_DEVICE_INFO *HikCamera::getDevInfo(CAMERA_TYPE type, bool& isSearch)
{
    isSearch = false;
    MV_CC_DEVICE_INFO* pDeviceInfo = NULL;
    for (unsigned int i = 0; i < this->stDeviceList.nDeviceNum; i++)
    {
        pDeviceInfo = this->stDeviceList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
            continue;

        QString cameraName = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
        QString seriaNumber = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
        cameraName = QString("%1(%2)").arg(cameraName).arg(seriaNumber);
        if(type == CAMERA_TYPE::CH120 &&
                cameraName == AppConfigBase::getInstance()->readCameraSettings("CameraName","areaArrayCamera",""))
        {
            m_cameraName = cameraName;
            isSearch = true;
            break;
        }
        if(type == CAMERA_TYPE::CL042 &&
                cameraName == AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_L",""))
        {
            m_linearArrayCamera_suffix = "L";
            m_cameraName = cameraName;
            isSearch = true;
            break;
        }
        if(type == CAMERA_TYPE::CL042_2 &&
                cameraName == AppConfigBase::getInstance()->readCameraSettings("CameraName","linearArrayCamera_R",""))
        {
            m_linearArrayCamera_suffix = "R";
            m_cameraName = cameraName;
            isSearch = true;
            break;
        }
    }
    return pDeviceInfo;
}

QString HikCamera::getDevName(CAMERA_TYPE type)
{
//    QString cameraName;
//    for (unsigned int i = 0; i < this->stDeviceList.nDeviceNum; i++)
//    {
//        MV_CC_DEVICE_INFO* pDeviceInfo = this->stDeviceList.pDeviceInfo[i];
//        if (NULL == pDeviceInfo)
//            continue;

//        cameraName = reinterpret_cast<char *>(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
//        if(type == CAMERA_TYPE::CH120 && cameraName.contains("CH120"))
//        {
//            break;
//        }
//        if((type == CAMERA_TYPE::CL042 || type == CAMERA_TYPE::CL042_2) && cameraName.contains("CL042"))
//        {
//            break;
//        }
//    }
    return m_cameraName;
}

void HikCamera::regeistEvent(QString eventName, bool open)
{
    int nRet = MV_OK;
    // Set Event of ExposureEnd On
    nRet = MV_CC_SetEnumValueByString(m_camHandle,"EventSelector",eventName.toStdString().c_str());
    if (MV_OK != nRet)
    {
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + QString(" '%1'参数选择失败").arg(eventName));
    }

    if(!open)
        return;

    nRet = MV_CC_SetEnumValueByString(m_camHandle,"EventNotification","On");
    if (MV_OK != nRet)
    {
        printf("Set Event Notification fail! nRet [0x%x]\n", nRet);
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + QString(" '%1'EventNotification开启失败").arg(eventName));
    }

    // Register event callback
    nRet = MV_CC_RegisterEventCallBackEx(m_camHandle, eventName.toStdString().c_str(), __eventCallBackEx, this);
    if (MV_OK != nRet)
    {
        printf("Register Event CallBack fail! nRet [0x%x]\n", nRet);
        AppEventBase::getInstance()->sig_sendServerMsg(m_cameraLogName + QString(" '%1'事件监听失败").arg(eventName));
    }
}

void HikCamera::outPutEventText(QString eventName, void *data, int dataLength)
{
    if(!m_saveFolderPath.isEmpty() && AppCommonBase::getInstance()->bIsGPSReset)
    {
        //面阵相机
        if(CAMERA_TYPE::CH120 == m_cameraType)
        {
            if(eventName.contains("Line0Rising"))
            {
                m_isRcvLine0 = true;
                m_lineoCount_CH120++;
                qDebug() << "监听到事件触发" << m_cameraName  << eventName << m_lineoCount_CH120 << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_lineoCount_CH120)));
            }
            else if(eventName.contains("FrameStart"))
            {
                m_frameStartCount++;
                qDebug() << "监听到事件触发" << m_cameraName  << eventName << m_frameStartCount << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
               // AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_frameStartCount)));
            }
            else if(eventName.contains("FrameEnd") && 0 != m_frameStartCount)
            {
                m_frameEndCount++;
                qDebug() << "监听到事件触发" << m_cameraName  << eventName << m_frameEndCount << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_frameEndCount)));
            }
            else if(eventName.contains("ExposureEnd"))
            {
                m_exposureEndCount++;
                qDebug() << "监听到事件触发" << m_cameraName  << eventName << m_exposureEndCount << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_frameEndCount)));
            }
            else
            {
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" 监听到'%1'事件").arg(eventName));
                qDebug() << "监听到其它事件触发" << m_cameraName  << eventName << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            }
        }
        else if(CAMERA_TYPE::CL042 == m_cameraType  || CAMERA_TYPE::CL042_2 == m_cameraType)
        {
            //线阵相机
            if(eventName.contains("AcquisitionTrigger"))
            {
                m_lineoCountStart_CL042++;
                qDebug() << "监听到触发开始" << m_cameraName  << eventName << m_lineoCountStart_CL042 << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_lineoCountStart_CL042)));
            }
            else if(eventName.contains("AcquisitionTriggerEnd"))
            {
                m_lineoCountEnd_CL042++;
                qDebug() << "监听到触发结束" << m_cameraName  << eventName << m_lineoCountEnd_CL042 << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" '%1'事件,总次数:%2").arg(eventName).arg(QString::number(m_lineoCountEnd_CL042)));
            }
            else
            {
                //AppEventBase::getInstance()->sig_sendServerMsg(getDevName(m_cameraType) + QString(" 监听到'%1'事件").arg(eventName));
                qDebug() << "监听到其它事件触发" << m_cameraName  << eventName << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            }
        }
    }
}

void HikCamera::outPutAllEventText(MV_EVENT_OUT_INFO *pEventInfo)
{
    if(CAMERA_TYPE::CH120 == m_cameraType)
    {
        qDebug() << "监听到事件触发(面阵)：" << m_cameraName << QString("Event名称:%1,Event号:%2").arg(pEventInfo->EventName).arg(QString::number(int(pEventInfo->nEventID)));
    }

    else if(CAMERA_TYPE::CL042 == m_cameraType || CAMERA_TYPE::CL042_2 == m_cameraType)
    {
        qDebug() << "监听到事件触发(线阵)：" << m_cameraName  << QString("Event名称:%1,Event号:%2").arg(pEventInfo->EventName).arg(QString::number(int(pEventInfo->nEventID)));
    }
}

void HikCamera::setWinId(HWND winId)
{
    m_mainWndID = winId;
}

void HikCamera::closeSaveImage()
{
    if(NULL != this->m_camHandle)
        MV_CC_RegisterImageCallBackEx(this->m_camHandle, NULL, this);

    m_isCloseRecord = true;
    qDebug() << m_cameraName << "关闭图片回调函数，不再接收相机图片";
}

#ifndef HIKCAMERA_H
#define HIKCAMERA_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QtMath>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <MvCameraControl.h>
#include <QJsonArray>
#include "httpclient.h"
#include "imagesavethread.h"
#include <QTimer>
#include <QMutex>

#define HIKCAM_BUFLEN 5 * 1048576
#define HIKCAM_FMT_CL042  PixelType_Gvsp_Mono8
#define HIKCAM_FMT_CH120  PixelType_Gvsp_BayerRG8

//相机类型
enum CAMERA_TYPE
{
    CL042,//线阵相机（道路相机）
    CH120,//面阵相机(景观相机)
    CL042_2,//第二个线阵相机（道路相机）
};

class HikCamera : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QString> camera READ camera NOTIFY cameraChanged)
public:
    explicit HikCamera(QObject *parent = nullptr);
    void setCacheCount(int count);
    ~HikCamera();
    QList<QString> camera();
    void freeCamera();
    void setSaveFolderPath(QString folderPath);
    bool isOpen();
    void outPutEventText(QString eventName, void* data, int dataLength);
    void outPutAllEventText(MV_EVENT_OUT_INFO * pEventInfo);
    // unsigned char imgBuffer[HIKCAM_BUFLEN];
    void setWinId(HWND winId);
    void closeSaveImage();
    QString getCameraName(){return m_cameraName;}
    QString getLastMsg(){return m_lastMsg;}

public slots:
    void onTimerTimeout();
    void onImageCallback(unsigned char *pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);
    void setCamera(int);
    void refreshCamera();
    void startGrab();
    void regeistCallBack();
    void stopGrab();
    void startRecord();
    void stopRecord();
    void setExposureTime(int time);
    void setExposureAuto(bool isAuto);
    void setgainValue(float value);
    void setgammaValue(float value);
    void clearCameraCache();
signals:
    void cameraChanged();
    void rawDataGot(unsigned char *data, int width, int height, bool isMono8Data = false);
    void sendNullData();
    void dataReady(int fps);
    void imageReady(const QImage& image,int width, int height);
    void exposureTimeChanged(int cur, int min, int max);
    void sendParams(int exposureTime,double gainValue,double gammaValue);
    void sendImageSize(int width, int height);

private slots:
    void slt_saveDataToServer();

private:
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    QList<QString> cameraList;
    //int m_selectedCameraIndex = -1;
    CAMERA_TYPE m_cameraType;
    QTimer *fpsTimer = NULL;
    void *m_camHandle = NULL;
    int m_frameCount = 0;
    int m_fps = 0;
    bool m_isGrabbing = false;
    bool m_isRecording = false;
    int m_exTimeCur = 0;
    int m_exTimeMin = 0;
    int m_exTimeMax = 0;
    QString m_saveFolderPath = "";
    bool m_isReady = false;
    QMutex m_mutex;
    QJsonArray m_array;
    HttpClient m_httpClient;//后端请求交互类
    bool m_isOpen = false;
    bool m_isRcvLine0 = false;
    bool m_isCheckFrameNum = false;
    HWND m_mainWndID = nullptr;

    int m_lineoCount_CH120 = 0;//面阵相机触发个数
    int m_lineoCountStart_CL042 = 0;
    int m_lineoCountEnd_CL042 = 0;
    int m_preLandscapeCameraIndex = -1;
    int m_preRodeCameraIndex = -1;

    int m_frameStartCount = 0;
    int m_frameEndCount = 0;
    int m_exposureEndCount = 0;
    int m_imageQuality = 99;

    int m_CH120_count = 0;
    int m_CL042_count = 0;
    int m_prePulsesTotalNum = -1;
    int m_maxGPSFrameNum = 30000;

    int m_listNum = 0;

    //上一个帧号
    int m_preFrameNum = -100;

    //线程存储线程
    QList<ImageSaveThread*> m_threadList;
    int m_imageCount = 0;
    QString m_cameraName;
    QString m_cameraLogName;
    QString m_linearArrayCamera_suffix = "F";
    bool m_isCloseRecord = false;
    QString m_lastMsg;
private:
     MV_CC_DEVICE_INFO* getDevInfo(CAMERA_TYPE type, bool& isSearch);
     QString getDevName(CAMERA_TYPE type);
     void regeistEvent(QString eventName, bool open = true);
};

#endif // HIKCAMERA_H

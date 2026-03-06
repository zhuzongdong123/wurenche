#include "imagesavethread.h"
#include <QDebug>
#include "appconfigbase.h"
#include <QDateTime>
#include "appeventbase.h"

void deepCopy(const unsigned char* src, unsigned char** dest, size_t length)
{
    if(NULL == dest)
        return;

    memcpy(*dest,src,length*sizeof (unsigned char));
}


ImageSaveThread::ImageSaveThread(QObject *parent) : QThread(parent)
{
}

ImageSaveThread::~ImageSaveThread()
{
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

    m_isStop = true;
    quit();
    wait();

    for(int i = 0; i < m_readVec.size(); i++)
    {
        MV_SAVE_IMG_TO_FILE_PARAM* stSaveFileParam = m_readVec[i];
        free(stSaveFileParam->pData);
        delete stSaveFileParam;
        stSaveFileParam = nullptr;
    }

    for(int i = 0; i < m_saveVec.size(); i++)
    {
        MV_SAVE_IMG_TO_FILE_PARAM* stSaveFileParam = m_saveVec[i];
        free(stSaveFileParam->pData);
        delete stSaveFileParam;
        stSaveFileParam = nullptr;
    }
}

void ImageSaveThread::stopThread()
{
    m_isStop = true;
}

void ImageSaveThread::newFileParam(int count, int size)
{
    m_mutex.lock();
    if(0 != m_readVec.size())
    {
        m_mutex.unlock();
        return;
    }

    for(int i = 0; i < count; i++)
    {
        MV_SAVE_IMG_TO_FILE_PARAM* stSaveFileParam = new MV_SAVE_IMG_TO_FILE_PARAM;
        stSaveFileParam->pData = (unsigned char*)malloc(size);
        stSaveFileParam->nDataLen = size;
        m_readVec.push_back(stSaveFileParam);
    }

    m_size = size;

    qDebug() << "开辟缓存----------" << count << size;
    m_mutex.unlock();
}

void ImageSaveThread::addFile(MV_SAVE_IMG_TO_FILE_PARAM *param)
{
    if(!isRunning())
        return;

    qDebug() << "插入图片到缓存中 start" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;;
    m_mutex.lock();
    if(m_readVec.size() > 0)
    {
        MV_SAVE_IMG_TO_FILE_PARAM* newParam = m_readVec[0];
        qDebug() << "插入图片到缓存中 test1" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
//        //*newParam = *m_readVec[0];
//        //深拷贝
//        unsigned char * pData = newParam->pData;
//        *newParam = *param;
//        memcpy(pData, param->pData, param->nDataLen);
//        newParam->pData = pData;
        newParam->enPixelType = param->enPixelType;
        qDebug() << "插入图片到缓存中 test2" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
        deepCopy(param->pData,&newParam->pData,param->nDataLen);
        qDebug() << "插入图片到缓存中 test3" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
        //memcmp(newParam->pData,param->pData,param->nDataLen);
        newParam->nDataLen = param->nDataLen;
        newParam->nWidth = param->nWidth;
        newParam->nHeight = param->nHeight;
        newParam->enImageType = param->enImageType;
        newParam->nQuality = param->nQuality;
        qDebug() << "插入图片到缓存中 test4" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
        memset(newParam->pImagePath,0,256);
        for(int i =0; i < 256; i++)
        {
            newParam->pImagePath[i] = param->pImagePath[i];
        }
        newParam->iMethodValue = param->iMethodValue;
        qDebug() << "插入图片到缓存中 test5" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
        m_readVec.pop_front();//移除第一个元素
        m_saveVec.push_back(newParam);
        qDebug() << "插入图片到缓存中 test6" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
    }
    else
    {
        qDebug() << "异常信息---缓存空间不足，当前图片不处理" << m_cameraName;;
        AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + QString("%1 缓存空间不足导致丢图").arg(m_cameraName));
    }
    m_mutex.unlock();
    qDebug() << "插入图片到缓存中 end" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << m_cameraName;
}

void ImageSaveThread::setCamHandle(void *camHandle)
{
    m_camHandle = camHandle;
}

void ImageSaveThread::setCameraName(QString name)
{
    m_cameraName = name;
}

int ImageSaveThread::getCacheCount()
{
    m_mutex.lock();
    int count = m_saveVec.size();
    m_mutex.unlock();
    return  count;
}

void ImageSaveThread::run()
{
    while (true) {
        MV_SAVE_IMG_TO_FILE_PARAM* newParam = nullptr;
        m_mutex.lock();
        if(0 != m_saveVec.size())
        {
            newParam = m_saveVec[0];
            m_readVec.push_back(newParam);
            m_saveVec.pop_front();
            m_mutex.unlock();
        }
        else
        {
            m_mutex.unlock();
            QThread::msleep(40);
            //没有需要保存的图片且需要停止的时候，关闭线程
            if(m_isStop)
            {
                break;
            }
        }

        if(nullptr != newParam)
        {
            int ret = MV_OK;
            ret = MV_CC_SaveImageToFile(this->m_camHandle, newParam);
            if(MV_OK != ret)
            {
                qDebug() << "异常信息---图片存储失败" << ret << m_cameraName;;
                AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ") + QString("%1 图片存储失败 %2").arg(m_cameraName).arg(ret));
            }
        }
        QThread::msleep(20);
    }
}

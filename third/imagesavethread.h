#ifndef IMAGESAVETHREAD_H
#define IMAGESAVETHREAD_H

#include <QThread>
#include <QVector>
#include <MvCameraControl.h>
#include <QMutex>

class ImageSaveThread : public QThread
{
    Q_OBJECT
public:
    explicit ImageSaveThread(QObject *parent = nullptr);
    ~ImageSaveThread();

    void stopThread();

    //提前开辟图片存储的缓存控件
    void newFileParam(int count, int size);

    //将准备存储的图形插入
    void addFile(MV_SAVE_IMG_TO_FILE_PARAM* param);
    void addFileToThread(MV_SAVE_IMG_TO_FILE_PARAM* param);

    void setCamHandle(void *camHandle);
    void setCameraName(QString name);

    //获取图像缓存数量
    int getCacheCount();

protected:
    void run();

private:
    void *m_camHandle = NULL;//相机句柄
    QVector<MV_SAVE_IMG_TO_FILE_PARAM*> m_readVec;//读容器
    QVector<MV_SAVE_IMG_TO_FILE_PARAM*> m_saveVec;//写容器
    QMutex m_mutex;
    bool m_isStop = false;
    int m_size = 0;
    QString m_cameraName;
};

#endif // IMAGESAVETHREAD_H
